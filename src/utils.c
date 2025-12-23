//
// Created by loc15 on 12/01/24.
//


#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/sio.h"
#include "hardware/watchdog.h"
#include "pico_flash.h"

#include "utils.h"
#include "config.h"
#include "convert_data.h"
#ifdef USE_WS2812_LED
    #include "pico/status_led.h"
#endif
#if PICO_W
    #include "pico/cyw43_arch.h"
#endif

void init_led(){
#if PICO_W
    if(cyw43_arch_init()){
        printf("Wi-Fi init failed");
        return;
    }
#elif LED_PIN
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
#endif

#ifdef USE_WS2812_LED
    status_led_init();
    colored_status_led_set_on_with_color(0);
#endif
}

void led_on(){
#if PICO_W
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
#elif LED_PIN
    gpio_put(LED_PIN, 1);
#endif
}

#ifdef USE_WS2812_LED
    void set_color_led(uint8_t device){
        uint32_t color = 0;
        switch ((MODE)device) {
            case XINPUT:
                color = PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(1,15,1);
                break;
            case SWITCH:
                color = PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(22,10,5);
                break;
            case DINPUT:
                color = PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(15,4,1);
                break;
            case KBD_PS2:
                color = PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(22,10,5);
                break;
            case PSX:
                color = PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(56,10,5);
                break;
            case BLUETOOTH:
                color = PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(1,1,15);
                break;
            case WII:
                color = PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(1,1,1);
                break;
            case GC:
                color = PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(10,1,15);
                break;
            default:
                color = PICO_COLORED_STATUS_LED_COLOR_FROM_RGB(15,1,1);
                break;
        }
        //https://github.com/raspberrypi/pico-sdk/issues/2630
        colored_status_led_set_state(false);
        sleep_us(100);
        colored_status_led_set_on_with_color(color);
    }
#endif

void led_off(){
#if PICO_W
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
#elif LED_PIN
    gpio_put(LED_PIN, 0);
#endif
}

#ifdef USE_WS2812_LED
    void disable_led_color_control(){
        status_led_deinit();
    }
#endif

// Picoboard has a button attached to the flash CS pin, which the bootrom
// checks, and jumps straight to the USB bootcode if the button is pressed
// (pulling flash CS low). We can check this pin in by jumping to some code in
// SRAM (so that the XIP interface is not required), floating the flash CS
// pin, and observing whether it is pulled low.
//
// This doesn't work if others are trying to access flash at the same time,
// e.g. XIP streamer, or the other core.

bool __no_inline_not_in_flash_func(get_bootsel_button)() {
    const uint CS_PIN_INDEX = 1;

    // Must disable interrupts, as interrupt handlers may be in flash, and we
    // are about to temporarily disable flash access!
    uint32_t flags = save_and_disable_interrupts();

    // Set chip select to Hi-Z
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    // Note we can't call into any sleep functions in flash right now
    for (volatile int i = 0; i < 1000; ++i);

    // The HI GPIO registers in SIO can observe and control the 6 QSPI pins.
    // Note the button pulls the pin *low* when pressed.
#if PICO_RP2040
#define CS_BIT (1u << 1)
#else
#define CS_BIT SIO_GPIO_HI_IN_QSPI_CSN_BITS
#endif
    bool button_state = !(sio_hw->gpio_hi_in & CS_BIT);

    // Need to restore the state of chip select, else we are going to have a
    // bad time when we return to code in flash!
    hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl,
                    GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
                    IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

    restore_interrupts(flags);

    return button_state;
}

uint8_t what_pio_use(uint8_t host){
    switch ((MODE)host) {
        case KBD_PS2:
        case PSX:
        case N64:
        case BLUETOOTH:
        case WII:
            return 0;
        default:
            return 1;
    }
}

void save_wii_addr(void *wii_addr){
    //Copy the saved old data
    uint8_t buffer[256];
    for (int i = 0; i < 36; ++i){
        buffer[i] = read_flash(i);
    }
    // Set saved wii_addr
    buffer[27] = 1;

    //Copy wii_addr
    memcpy(&buffer[28], (uint8_t *)wii_addr, 6);

    //Write to flash
    write_flash(buffer, 1);
}

void swap_modes_in_flash(void *alternative_mode) {
    //Copy the saved old data
    uint8_t buffer[256];
    for (int i = 0; i < TOTAL_SIZE_IN_FLASH; ++i){
        buffer[i] = read_flash(i);
    }
    //Swap modes
    buffer[DATA_FEATURES_OFFSET + SWITCH_MODE] = buffer[DEVICE_MODE_OFFSET];
    buffer[DEVICE_MODE_OFFSET] = *(uint8_t *)alternative_mode;

    //Write to flash
    write_flash(buffer, 1);

    /*RESET*/
    watchdog_reboot(0, SRAM_END, 500);
}
