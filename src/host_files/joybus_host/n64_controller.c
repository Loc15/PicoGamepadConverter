//
// Created by loc15 on 14/11/25.
//
#include "pico/stdlib.h"
#include "controller.pio.h"

#include "n64_controller.h"
#include "joybus_controller.h"

joybus_controller_t joybus_n64;

void n64_controller_init(uint8_t pio, uint8_t joybus_pin, uint8_t *response) {
    joybus_n64.pio = (pio ? pio1 : pio0);
    joybus_n64.pin = joybus_pin;
    joybus_n64.controllerState = response;
    joybus_n64.min_axis_X = -0.5;
    joybus_n64.max_axis_X = 0.5;
    joybus_n64.min_axis_Y = -0.5;
    joybus_n64.max_axis_Y = 0.5;
    joybus_n64.offset = pio_add_program(joybus_n64.pio, &controller_program);
    joybus_n64.sm = pio_claim_unused_sm(joybus_n64.pio, true);
    pio_sm_config tmpConfig = controller_program_get_default_config(joybus_n64.offset);
    joybus_n64.c = &tmpConfig;
    controller_program_init(joybus_n64.pio, joybus_n64.sm, joybus_n64.offset, joybus_n64.pin, joybus_n64.c);
    joybus_controller_init(&joybus_n64);
    unsigned char data[1] = {0x00};
    //uint8_t response[3];
    joybus_send_data(data, 1, 1);
    sleep_us(200);
}

void n64_controller_update_state() {
    uint8_t data[1] = {0x01};
    joybus_send_data(data, 1, 4);
    busy_wait_us(500);
}

double n64_convert_axis(int8_t axisPos, uint16_t joystick_mid, int axis) {
    double unscaledAxisPos = axisPos / (double) N64_JOYSTICK_MAX;
    double scaledAxisPos = get_scaled_analog_axis(unscaledAxisPos, axis);
    return scaledAxisPos * joystick_mid + joystick_mid;
}