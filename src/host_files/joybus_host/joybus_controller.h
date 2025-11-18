//
// Created by loc15 on 14/11/25.
//

#ifndef MAIN_JOYBUS_CONTROLLER_H
#define MAIN_JOYBUS_CONTROLLER_H

#include "hardware/pio.h"

typedef struct {
    uint8_t pin;
    PIO pio;
    uint sm;
    pio_sm_config *c;
    uint offset;
    uint8_t stateBytes;
    uint8_t *controllerState;
    double min_axis_X;
    double max_axis_X;
    double min_axis_Y;
    double max_axis_Y;
} joybus_controller_t;

void joybus_controller_init(joybus_controller_t *joybus_config_data);

void joybus_send_data(uint8_t *request, uint8_t dataLength, uint8_t responseLength);

double get_scaled_analog_axis(double axisPos, int axis);

#endif //MAIN_JOYBUS_CONTROLLER_H
