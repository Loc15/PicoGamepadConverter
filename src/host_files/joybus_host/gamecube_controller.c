#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"

#include "gamecube_controller.h"
#include "joybus_controller.h"
#include "controller.pio.h"

joybus_controller_t joybus_gc;

/**
 * Comments cite gamecube controller reverse engineering writeup from Jeff Longo
 * https://jefflongo.dev/posts/gc-controller-reverse-engineering-part-1/
 */

/* controller.pio init uses ~1 us-class host cells (N64-oriented). GameCube host is ~5 us/bit (Jeff Longo).
 * The `frequency` parameter in controller_program_init is NOT the bit rate: a bit is
 * (T1 + T2) = 16 PIO cycles, and cyclesPerBit = (T1+T2)/4 = 4, so bit_time = 4 / frequency.
 * For 5 us/bit, frequency must be 800 kHz. (N64's 1 MHz gives 4 us/bit, also within GC spec.) */
static void gamecube_host_set_pio_bitrate(void) {
    const unsigned t1 = 4u, t2 = 12u; /* must match controller.pio T1 / T2 */
    const int cycles_per_bit = (int)(t1 + t2) / 4;
    const float gc_host_param_hz = 800000.f; /* -> 5 us/bit */
    float div = (float)clock_get_hz(clk_sys) / ((float)cycles_per_bit * gc_host_param_hz);
    pio_sm_set_enabled(joybus_gc.pio, joybus_gc.sm, false);
    pio_sm_set_clkdiv(joybus_gc.pio, joybus_gc.sm, div);
    pio_sm_set_enabled(joybus_gc.pio, joybus_gc.sm, true);
}

void gamecube_controller_init(uint8_t pio, uint8_t joybus_pin, uint8_t *response) {
    joybus_gc.pio = (pio ? pio1 : pio0);
    joybus_gc.pin = joybus_pin;
    joybus_gc.controllerState = response;
    joybus_gc.min_axis_X = -0.5;
    joybus_gc.max_axis_X = 0.5;
    joybus_gc.min_axis_Y = -0.5;
    joybus_gc.max_axis_Y = 0.5;
    joybus_gc.offset = pio_add_program(joybus_gc.pio, &controller_program);
    joybus_gc.sm = pio_claim_unused_sm(joybus_gc.pio, true);
    pio_sm_config cfg = controller_program_get_default_config(joybus_gc.offset);
    controller_program_init(joybus_gc.pio, joybus_gc.sm, joybus_gc.offset, joybus_gc.pin, &cfg);
    gamecube_host_set_pio_bitrate();
    joybus_controller_init(&joybus_gc);

    /* Per Jeff Longo: ID (0x00) first, then origin (0x41). Origin returns 10 bytes — not 3. */
    uint8_t id_rx[3];
    uint8_t origin_rx[10];
    uint8_t id_cmd[1] = {0x00};
    joybus_gc.controllerState = id_rx;
    updatePioOutputSize(8); /* 1 data byte = 8 bits -> PULL_THRESH = 13 */
    joybus_send_data(id_cmd, 1, 3);
    sleep_us(50);
    uint8_t origin_cmd[1] = {0x41};
    joybus_gc.controllerState = origin_rx;
    updatePioOutputSize(8);
    joybus_send_data(origin_cmd, 1, 10);
    joybus_gc.controllerState = response;
    sleep_us(200);
}

void gamecube_controller_update_state() {
    // Poll state with rumble off (third byte = 0x00).
    // 3 data bytes = 24 bits -> PULL_THRESH = 29 so the full command is shifted out.
    // Without this, the default threshold of 13 (set for N64's 1-byte commands) causes
    // OSRE to trigger after only 8 data bits and the rest of the poll is silently dropped.
    uint8_t poll_data[3] = {0x40, 0x03, 0x00};
    updatePioOutputSize(24);
    joybus_send_data(poll_data, 3, 8);
    busy_wait_us(500);
}
