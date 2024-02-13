/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>
#include "tusb.h"
#include "device/usbd_pvt.h"

#include "XInputDescriptors.h"

#define XINPUT_OUT_SIZE 32
#define GAMEPAD_JOYSTICK_MID 0

// USB endpoint state vars
extern uint8_t endpoint_in;
extern uint8_t endpoint_out;
extern uint8_t xinput_out_buffer[XINPUT_OUT_SIZE];
extern const usbd_class_driver_t xinput_driver;

void receive_xinput_report(void);
bool send_xinput_report(void *report, uint8_t report_size);
void send_report(void *report, uint16_t report_size);

#pragma once
