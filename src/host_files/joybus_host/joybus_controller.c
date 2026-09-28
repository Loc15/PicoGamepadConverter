//
// Created by loc15 on 14/11/25.
//

#include <stdio.h>
#include "pico/stdlib.h"

#include "joybus_controller.h"

#define JOYBUS_PHASE_TIMEOUT_US 600

joybus_controller_t *internal_config;

void joybus_controller_init(joybus_controller_t *joybus_config_data) {
    internal_config = joybus_config_data;
}

void emptyRxFifo() {
    while (pio_sm_get_rx_fifo_level(internal_config->pio, internal_config->sm)) {
        pio_sm_get(internal_config->pio, internal_config->sm);
    }
}

void updatePioOutputSize(uint8_t autoPullLength) {
    // Pull mask = 0x3E000000
    // Push mask = 0x01F00000
    pio_sm_set_enabled(internal_config->pio, internal_config->sm, false);
    internal_config->pio->sm[internal_config->sm].shiftctrl = (internal_config->pio->sm[internal_config->sm].shiftctrl & 0xA00FFFFF) |
                              (0x8 << 20) |
                              (((autoPullLength + 5) & 0x1F) << 25);
    // Clear FIFOs and SM internal state, then rewind PC to the program's wrap target.
    // SM_RESTART alone does not reset PC, so a state machine stuck in the receive
    // `wait 0 pin 0` loop (e.g. after a previously truncated command) would otherwise
    // remain stuck even after we change the pull threshold.
    pio_sm_clear_fifos(internal_config->pio, internal_config->sm);
    internal_config->pio->ctrl |= 1 << (4 + internal_config->sm);
    pio_sm_exec(internal_config->pio, internal_config->sm,
                pio_encode_jmp(internal_config->offset));
    pio_sm_set_enabled(internal_config->pio, internal_config->sm, true);
}

void joybus_send_data(uint8_t *request, uint8_t dataLength, uint8_t responseLength) {
    uint8_t *response = (internal_config->controllerState);
    uint32_t dataWithResponseLength = ((responseLength - 1) & 0x1F) << 27;
    for (int i = 0; i < dataLength; i++) {
        dataWithResponseLength |= *(request + i) << (19 - i * 8);
    }
    // TX can wedge if the PIO state machine stops consuming words (e.g. line stuck / bad timing).
    // Avoid permanently blocking the polling core: bound the wait and recover by restarting the SM.
    absolute_time_t tx_timeout_us = make_timeout_time_us(JOYBUS_PHASE_TIMEOUT_US);
    while (pio_sm_is_tx_fifo_full(internal_config->pio, internal_config->sm) && !time_reached(tx_timeout_us)) {
        tight_loop_contents();
    }
    if (pio_sm_is_tx_fifo_full(internal_config->pio, internal_config->sm)) {
        // SM is usually stuck in receive (wait 0 pin) with no controller, while this core
        // kept enqueueing commands until the TX FIFO filled. Restarting the SM does not
        // drain TX on RP2040 — without clearing FIFOs every poll would bail here forever
        // and the data line would show no host activity.
        pio_sm_set_enabled(internal_config->pio, internal_config->sm, false);
        pio_sm_clear_fifos(internal_config->pio, internal_config->sm);
        internal_config->pio->ctrl |= 1u << (4 + internal_config->sm);
        emptyRxFifo();
        pio_sm_set_enabled(internal_config->pio, internal_config->sm, true);
        for (int i = 0; i < responseLength; i++) {
            response[i] = 0;
        }
        // Fall through: send this poll after recovery so Joybus keeps trying.
    }
    pio_sm_put(internal_config->pio, internal_config->sm, dataWithResponseLength);

    int16_t remainingBytes = responseLength;
    while (remainingBytes > 0) {
        absolute_time_t timeout_us = make_timeout_time_us(JOYBUS_PHASE_TIMEOUT_US);
        bool timedOut = false;
        while (pio_sm_is_rx_fifo_empty(internal_config->pio, internal_config->sm) && !timedOut) {
            timedOut = time_reached(timeout_us);
        }
        // If the controller doesn't respond, don't block or crash by reading an empty FIFO.
        // Instead, emit 0 for that byte and continue so the caller can keep running and
        // diagnostics can show "no response" rather than freezing.
        if (timedOut) {
            response[responseLength - remainingBytes] = 0;
            remainingBytes--;
            continue;
        }

        uint32_t data = pio_sm_get(internal_config->pio, internal_config->sm);
        response[responseLength - remainingBytes] = (uint8_t)(data & 0xFF);
        remainingBytes--;
    }
}

double get_scaled_analog_axis(double axisPos, int axis) {
    double *maxAxis;
    double *minAxis;
    if(axis){
        maxAxis = &(internal_config->max_axis_Y);
        minAxis = &(internal_config->min_axis_Y);
    }
    else{
        maxAxis = &(internal_config->max_axis_X);
        minAxis = &(internal_config->min_axis_X);
    }
    *maxAxis = axisPos > *maxAxis ? axisPos : *maxAxis;
    *minAxis = axisPos < *minAxis ? axisPos : *minAxis;
    return axisPos > 0 ? axisPos / *maxAxis : -axisPos / *minAxis;
}
