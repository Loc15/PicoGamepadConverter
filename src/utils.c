//
// Created by loc15 on 12/01/24.
//


#include "pico/stdlib.h"

#include "utils.h"
#include "config.h"
#if PICO_W
#include "pico/cyw43_arch.h"
#else
#define LED LED_PIN
#endif

void init_led(){
#if PICO_W
    if(cyw43_arch_init()){
        printf("Wi-Fi init failed");
        return;
    }
#else
    gpio_init(LED);
    gpio_set_dir(LED, GPIO_OUT);
#endif
}

void led_on(){
#if PICO_W
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
#else
    gpio_put(LED, 1);
#endif
}

void led_off(){
#if PICO_W
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
#else
    gpio_put(LED, 0);
#endif
}
