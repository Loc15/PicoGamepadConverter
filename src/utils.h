//
// Created by loc15 on 12/01/24.
//

#ifndef PROTOTYPE_UTILS_H
#define PROTOTYPE_UTILS_H

//Initialization led
void init_led();
//Turn on the led
void led_on();
//Turn off the led
void led_off();
//Set color from mode
void set_color_led(uint8_t device);
//Disable control over color led and deletes pio occupation
void disable_led_color_control();
//Boostel button
bool get_bootsel_button();
//Pio to use
uint8_t what_pio_use(uint8_t host);
//Save wii addr
void save_wii_addr(void *wii_addr);
//Swap modes in flash
void swap_modes_in_flash(void *alternative_mode);

#endif //PROTOTYPE_UTILS_H
