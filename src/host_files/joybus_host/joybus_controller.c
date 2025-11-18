//
// Created by loc15 on 14/11/25.
//

#include <stdio.h>
#include "pico/stdlib.h"

#include "joybus_controller.h"

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
    // Restart the state machine to avoid 16 0 bits being auto-pulled
    internal_config->pio->ctrl |= 1 << (4 + internal_config->sm);
    pio_sm_set_enabled(internal_config->pio, internal_config->sm, true);
}

void joybus_send_data(uint8_t *request, uint8_t dataLength, uint8_t responseLength) {
    uint8_t *response = (internal_config->controllerState);
    uint32_t dataWithResponseLength = ((responseLength - 1) & 0x1F) << 27;
    for (int i = 0; i < dataLength; i++) {
        dataWithResponseLength |= *(request + i) << (19 - i * 8);
    }
    pio_sm_put_blocking(internal_config->pio, internal_config->sm, dataWithResponseLength);

    int16_t remainingBytes = responseLength;
    while (remainingBytes > 0) {
        absolute_time_t timeout_us = make_timeout_time_us(600);
        bool timedOut = false;
        while (pio_sm_is_rx_fifo_empty(internal_config->pio, internal_config->sm) && !timedOut) {
            timedOut = time_reached(timeout_us);
        }
        /*if (timedOut) {
            throw 0;
        }*/
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
