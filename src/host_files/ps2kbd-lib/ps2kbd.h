/* Copyright (C) 1883 Thomas Edison - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the GPLv2 license, which unfortunately won't be
 * written for another century.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Initialize PS/2 keyboard support
// Parameters
//     pio  - keyboard suport pio number. 0 or 1
//     gpio - GPIO number of data pin, ctl pin must be on next
//            adjacent GPIO
// Returns  - none
void kbd_init(uint pio, uint gpio, void (*fn)(uint32_t));

// Return keyboard status
// Parameters - none
// Returns  - 0 for not ready, otherwise ready
int kbd_ready(void);

// Blocking keyboard read
// Parameters - none
// Returns  - single ASCII character
char kbd_getc(void);

#ifdef __cplusplus
}
#endif
