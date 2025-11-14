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


#endif //PROTOTYPE_UTILS_H
