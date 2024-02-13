/* Copyright (C) 1883 Thomas Edison - All Rights Reserved
 * You may use, distribute and modify this code under the
 * terms of the GPLv2 license, which unfortunately won't be
 * written for another century.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <stdio.h>
#include "ps2kbd.pio.h"

#include "ps2kbd.h"

#include "hardware/clocks.h"
#include "hardware/pio.h"

static PIO kbd_pio;         // pio0 or pio1
static uint kbd_sm;         // pio state machine index
static uint base_gpio;      // data signal gpio #
static uint8_t interrupt_pio;


// clang-format off

#define BS 0x8
#define TAB 0x9
#define LF 0xA
#define ESC 0x1B

// Lower-Case ASCII codes by keyboard-code index, 16 elements per row
/*static const uint8_t lower[] = {
    0,  0,   0,   0,   0,   0,   0,   0,  0,  0,   0,   0,   0,   TAB, '`', 0,
    0,  0,   0,   0,   0,   'q', '1', 0,  0,  0,   'z', 's', 'a', 'w', '2', 0,
    0,  'c', 'x', 'd', 'e', '4', '3', 0,  0,  ' ', 'v', 'f', 't', 'r', '5', 0,
    0,  'n', 'b', 'h', 'g', 'y', '6', 0,  0,  0,   'm', 'j', 'u', '7', '8', 0,
    0,  ',', 'k', 'i', 'o', '0', '9', 0,  0,  '.', '/', 'l', ';', 'p', '-', 0,
    0,  0,   '\'',0,   '[', '=', 0,   0,  0,  0,   LF,  ']', 0,   '\\',0,   0,
    0,  0,   0,   0,   0,   0,   BS,  0,  0,  0,   0,   0,   0,   0,   0,   0,
    0,  0,   0,   0,   0,   0,   ESC, 0,  0,  0,   0,   0,   0,   0,   0,   0};
*/

#define KEYBOARD_MASK_UP    0
#define KEYBOARD_MASK_DOWN  1
#define KEYBOARD_MASK_LEFT  2
#define KEYBOARD_MASK_RIGHT 3
#define KEYBOARD_MASK_START 4
#define KEYBOARD_MASK_BACK  5
#define KEYBOARD_MASK_LEFT_THUMB     6
#define KEYBOARD_MASK_RIGHT_THUMB    7
#define KEYBOARD_MASK_LEFT_SHOULDER  8
#define KEYBOARD_MASK_RIGHT_SHOULDER 9
#define KEYBOARD_MASK_GUIDE 10
#define KEYBOARD_MASK_A     12
#define KEYBOARD_MASK_B     13
#define KEYBOARD_MASK_X     14
#define KEYBOARD_MASK_Y     15
#define KEYBOARD_MASK_LX_L  16
#define KEYBOARD_MASK_LX_R  17
#define KEYBOARD_MASK_LX_U  18
#define KEYBOARD_MASK_LX_D  19
#define KEYBOARD_MASK_RX_L  20
#define KEYBOARD_MASK_RX_R  21
#define KEYBOARD_MASK_RX_U  22
#define KEYBOARD_MASK_RX_D  23
#define KEYBOARD_MASK_LEFT_TRIGGER   24
#define KEYBOARD_MASK_RIGHT_TRIGGER  25



// Lower-Case ASCII codes by keyboard-code index, 16 elements per row
static const uint8_t lower[] = {
    31,  31,   31,   31,   31,   31,   31,   31,  31,  31,   31,   31,   31,   31, 31, 31,
    31,  31,   31,   31,   31,  31, 31, 31,  31,  31, 31, KEYBOARD_MASK_DOWN, KEYBOARD_MASK_LEFT, KEYBOARD_MASK_UP, 31, 31,
    31,  31, 31, KEYBOARD_MASK_RIGHT, 31, 31, 31, 31,  31,  31, 31, 31, KEYBOARD_MASK_LEFT_TRIGGER, 31, 31, 31,
    31,  31, 31, KEYBOARD_MASK_X, 31, KEYBOARD_MASK_LEFT_SHOULDER, 31, 31,  31,  31, 31, KEYBOARD_MASK_A, KEYBOARD_MASK_Y, 31, 31, 31,
    31,  31, KEYBOARD_MASK_B, KEYBOARD_MASK_RIGHT_SHOULDER, KEYBOARD_MASK_RIGHT_TRIGGER, 31, 31, 31,  31, 31, 31, 31, 31, KEYBOARD_MASK_GUIDE, 31, 31,
    31,  31,  31, 31, 31, 31, 31,   31,  31,  31,   KEYBOARD_MASK_START, 31, 31, 31 ,31,   31,
    31,  31,   31,   31,   31,   31,   KEYBOARD_MASK_BACK,  31,  31,  31,   31,   KEYBOARD_MASK_LX_L,   31,   31,   31,   31,
    31,  31,   KEYBOARD_MASK_LX_D,   31,   KEYBOARD_MASK_LX_R,   KEYBOARD_MASK_LX_U,   31, 31,  31,  31,   31,   31,   31,   31,   31,   31};

// Upper-Case ASCII codes by keyboard-code index
static const uint8_t upper[] = {
    0,  0,   0,   0,   0,   0,   0,   0,  0,  0,   0,   0,   0,   TAB, '~', 0,
    0,  0,   0,   0,   0,   'Q', '!', 0,  0,  0,   'Z', 'S', 'A', 'W', '@', 0,
    0,  'C', 'X', 'D', 'E', '$', '#', 0,  0,  ' ', 'V', 'F', 'T', 'R', '%', 0,
    0,  'N', 'B', 'H', 'G', 'Y', '^', 0,  0,  0,   'M', 'J', 'U', '&', '*', 0,
    0,  '<', 'K', 'I', 'O', ')', '(', 0,  0,  '>', '?', 'L', ':', 'P', '_', 0,
    0,  0,   '"', 0,   '{', '+', 0,   0,  0,  0,   LF,  '}', 0,   '|', 0,   0,
    0,  0,   0,   0,   0,   0,   BS,  0,  0,  0,   0,   0,   0,   0,   0,   0,
    0,  0,   0,   0,   0,   0,   ESC, 0,  0,  0,   0,   0,   0,   0,   0,   0};
// clang-format on


static uint8_t release; // Flag indicates the release of a key
static uint8_t shift;   // Shift indication
static uint8_t ascii;   // Translated to ASCII

void (*callback)( uint32_t ) = {0}; //callback from the main

void __not_in_flash_func(kbd_irq)(){

    static uint32_t keyboard_t;            //structure for keyboard->static

    uint8_t code = *((io_rw_8*)&kbd_pio->rxf[kbd_sm] + 3);
    switch (code) {
    case 0xF0:               // key-release code 0xF0 detected
        release = 1;         // set release
        break;               // go back to start
    case 0x12:               // Left-side SHIFT key detected
    case 0x59:               // Right-side SHIFT key detected
        if (release) {       // L or R Shift detected, test release
            shift = 0;       // Key released preceded  this Shift, so clear shift
            release = 0;     // Clear key-release flag
        } else
            shift = 1; // No previous Shift detected before now, so set Shift_Key flag now
        break;
    case 0xE0:               //extended keys are recognizable by the E0h
        //16 bits data
        break;
    default:
        // no case applies
        ascii = (shift ? upper : lower)[code]; // Get ASCII value by case
        if(ascii == 31){        //fix
            break;
        }
        else if (!release){                             // If no key-release detected yet
            keyboard_t |= (1 << ascii);        //Just take 31 bits
        }
        else{                                           // If a key is release
            keyboard_t &= ~((1 << ascii));           //Just take 31 bits
            release = 0;
        }                             
        (*callback)(keyboard_t);                              //Call the callback function
        break;
    }
    //ascii = 0;
    //(*callback)();                              //Call the callback function
    pio_interrupt_clear(kbd_pio, 0);            //Clear interrupt
}

void kbd_init(uint pio, uint gpio, void (*fn)(uint32_t)) {
    kbd_pio = pio ? pio1 : pio0;
    interrupt_pio = pio ? PIO1_IRQ_0 : PIO0_IRQ_0;
    base_gpio = gpio; // base_gpio is data signal, base_gpio+1 is clock signal
    // init KBD pins to input
    gpio_init(base_gpio);
    gpio_init(base_gpio + 1);
    // with pull up
    gpio_pull_up(base_gpio);
    gpio_pull_up(base_gpio + 1);
    // get a state machine
    kbd_sm = pio_claim_unused_sm(kbd_pio, true);
    // reserve program space in SM memory
    uint offset = pio_add_program(kbd_pio, &ps2kbd_program);
    // Set pin directions base
    pio_sm_set_consecutive_pindirs(kbd_pio, kbd_sm, base_gpio, 2, false);
    // program the start and wrap SM registers
    pio_sm_config c = ps2kbd_program_get_default_config(offset);
    // Set the base input pin. pin index 0 is DAT, index 1 is CLK
    sm_config_set_in_pins(&c, base_gpio);
    // Shift 8 bits to the right, autopush enabled
    sm_config_set_in_shift(&c, true, true, 8);
    // Deeper FIFO as we're not doing any TX
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
    // We don't expect clock faster than 16.7KHz and want no less
    // than 8 SM cycles per keyboard clock.
    float div = (float)clock_get_hz(clk_sys) / (8 * 16700);
    sm_config_set_clkdiv(&c, div);
    //Enable irq interrupts for up and down actions.
    pio_set_irq0_source_enabled(kbd_pio, pis_interrupt0, true);
    //irq_add_shared_handler(PIO1_IRQ_0, kbd_irq, 0);
    irq_set_exclusive_handler(interrupt_pio, kbd_irq);
    irq_set_enabled(interrupt_pio, true);
    // Ready to go
    pio_sm_init(kbd_pio, kbd_sm, offset, &c);
    pio_sm_set_enabled(kbd_pio, kbd_sm, true);

    //callback function
    callback = fn;
    
}
