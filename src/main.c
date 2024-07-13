#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"

#include "pio_usb.h"
#include "tusb.h"

//Device
#include "xinput_driver.h"
#include "hid_driver.h"

#if PICO_W

#include "blue_hid.h"
#include "bluetooth_descriptors.h"
#include "wiimote.h"
#include "wiimote_btstack.h"

#endif

#include "controller_simulator.h"

//Host
#include "xinput_host.h"
#include "ps2kbd.h"
#include "psx.h"

#if PICO_W

#include "blue_host.h"

#endif

//Utils
#include "convert_data.h"
#include "gamepad_parser.h"
#include "rndis.h"
#include "pico_flash.h"
#include "utils.h"
#include "config.h"


//--------------------------------------------------------------------+
// GLOBAL AND STATIC VARIABLES
//--------------------------------------------------------------------+

void read_keyboard(uint32_t keyboard_t);

void read_psx_controller(uint32_t *psx_data);

static void sendReportData(void *original_data);

uint8_t endpoint_in = 0;
uint8_t endpoint_out = 0;
uint8_t xinput_out_buffer[XINPUT_OUT_SIZE] = {};

/*MODES*/
MODE HOST = XINPUT;
MODE DEVICE = XINPUT;

/*HID*/
HID_TYPE HID_HOST = GENERIC;


static SwitchReport switchReport =
        {
                .buttons = 0,
                .hat = SWITCH_HAT_NOTHING,
                .lx = SWITCH_JOYSTICK_MID,
                .ly = SWITCH_JOYSTICK_MID,
                .rx = SWITCH_JOYSTICK_MID,
                .ry = SWITCH_JOYSTICK_MID,
                .vendor = 0,
        };

static HIDReport hidReport = {
        .square_btn = 0, .cross_btn = 0, .circle_btn = 0, .triangle_btn = 0,
        .l1_btn = 0, .r1_btn = 0, .l2_btn = 0, .r2_btn = 0,
        .select_btn = 0, .start_btn = 0, .l3_btn = 0, .r3_btn = 0, .ps_btn = 0, .tp_btn = 0,
        .direction = 0x08,
        .l_x_axis = HID_JOYSTICK_MID,
        .l_y_axis = HID_JOYSTICK_MID,
        .r_x_axis = HID_JOYSTICK_MID,
        .r_y_axis = HID_JOYSTICK_MID,
        .right_axis = 0x00, .left_axis = 0x00, .up_axis = 0x00, .down_axis = 0x00,
        .triangle_axis = 0x00, .circle_axis = 0x00, .cross_axis = 0x00, .square_axis = 0x00,
        .l1_axis = 0x00, .r1_axis = 0x00, .l2_axis = 0x00, .r2_axis = 0x00
};


static XInputReport xinputReport = {
        .report_id = 0,
        .report_size = XINPUT_ENDPOINT_SIZE,
        .buttons1 = 0,
        .buttons2 = 0,
        .lt = 0,
        .rt = 0,
        .lx = GAMEPAD_JOYSTICK_MID,
        .ly = GAMEPAD_JOYSTICK_MID,
        .rx = GAMEPAD_JOYSTICK_MID,
        .ry = GAMEPAD_JOYSTICK_MID,
        ._reserved = {},
};

static PSXInputState psxReport = {
        .buttons1 = 0xff,
        .buttons2 = 0xff,
        .lx = 0x80,
        .ly = 0x80,
        .rx = 0x80,
        .ry = 0x80,
        .l2 = 0x00,
        .r2 = 0x00
};

#if PICO_W
static WiimoteReport wiimote_report = {
        .wiimote = {0},
        .nunchuk = {0},
        .classic = {0},
        .switch_mode = 0,
        .mode = NO_EXTENSION,
        .fake_motion = 0,
        .center_accel = 0
};

static BluetoothReport bluetooth_report = {
        .dpad = 0xF,
        .lx = 0x7F,
        .ly = 0x7F,
        .rx = 0x7F,
        .ry = 0x7F,
        .rt = 0x00,
        .lt = 0x00,
        .buttons1 = 0x00,
        .buttons2 = 0x00,
        .battery = 0x26
};
#endif

/*------------- MAIN -------------*/

// core1: handle host events
void core1_main() {
    //PS1/PS2 DEVICE MODE NEED REBOOT THE CORE
    if(DEVICE == PSX){
        psx_device_main();
    }

    switch (HOST) {
        case KBD_PS2:
            kbd_init(1, DAT_GPIO, read_keyboard);
            break;
        case PSX:
            psx_init(1, CMD, DAT, read_psx_controller);
            break;
        case BLUETOOTH:
            //It can't have input and output bluetooth simultaneously
            if ((DEVICE == BLUETOOTH) || (DEVICE == WII)) {
                return;
            }
#if PICO_W
            btstack_host(sendReportData);
#endif
            break;
        default:
            switch (DEVICE) {
                case WII:
                case BLUETOOTH:
                    //Wifi chip use pio, that cause problems
                    //init host stack for native usb (roothub port0)
                    tuh_init(0);
                    while (true) {
                        tuh_task();
                    }
                    break;
                default: {
                        // Use tuh_configure() to pass pio configuration to the host stack
                        // Note: tuh_configure() must be called before
                        pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
                        pio_cfg.pin_dp = PIO_USB_PIN;
                        tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);

                        // To run USB SOF interrupt in core1, init host stack for pio_usb (roothub
                        // port1) on core1
                        tuh_init(1);

                        while (true) {
                            tuh_task(); // tinyusb host task
                        }
                    }
                    break;
            }
            break;
    }
}

// core0: handle device events
int main(void) {
    // default 125MHz is not appropriate. Sysclock should be multiple of 12MHz.
    set_sys_clock_khz(240000, true);

    stdio_init_all();

    /*LED*/
    init_led();

    /*READ MODES FROM FLASH*/
    uint8_t read_value = read_flash(0);
    HOST = read_value > 6 ? 0 : read_value;
    read_value = read_flash(1);
    DEVICE = read_value > 6 ? 0 : read_value;

    printf("MODE HOST -> %d | MODE DEVICE -> %d\n", HOST, DEVICE);

    /*READ FEATURES FROM FLASH*/
    uint8_t features_data[18];
    for (int i = 0; i < 18; ++i) {
        features_data[i] = read_flash(9 + i);
    }
    set_features_from_flash(features_data);

    gpio_init(WEB_PIN);
    gpio_set_dir(WEB_PIN, GPIO_IN);

    sleep_ms(3000);

    /*WEB INTERFACE*/
    if (gpio_get(WEB_PIN)) {
        DEVICE = WEB;
        rndis();
    }

    sleep_ms(10);

    /*Check if PS1/PS2 DEVICE MODE is enabled*/
    if(DEVICE == PSX){
        psx_device_init(0, &psxReport, core1_main);
    }

    multicore_reset_core1();
    // all USB task run in core1
    multicore_launch_core1(core1_main);

    switch (DEVICE) {
        case BLUETOOTH:
#if PICO_W
            btstack_hid(&bluetooth_report);
#endif
            break;
        case WII:
#if PICO_W
            // Set led functions
            wiimote_emulator_set_led(led_on, led_off);
            // Wiimote emulator
            wiimote_emulator(&wiimote_report);
#endif
            break;
        case PSX:
            //This core is for device modes but PS1/PS2 MODE need reboot the core1
            switch(HOST){
                case KBD_PS2:
                    kbd_init(1, ALT_DAT_GPIO, read_keyboard);
                    while(1){};
                    break;
                case PSX:
                    psx_init(1, ALT_CMD, ALT_DAT, read_psx_controller);
                    while(1){};
                    break;
                case BLUETOOTH:
#if PICO_W
                    btstack_host(sendReportData);
#endif
                    break;
                default:
                    //init host stack for native usb (roothub port0)
                    tuh_init(0);
                    while(true){
                        tuh_task();
                    }
                    break;
            }
            break;
        default:
            // init device stack on native usb (roothub port0)
            tud_init(0);
            while (true) {
                tud_task(); // tinyusb device task
            }
            break;
    }


    return 0;
}


//--------------------------------------------------------------------+
// USB Driver Callback Setup
//--------------------------------------------------------------------+

const usbd_class_driver_t *usbd_app_driver_get_cb(uint8_t *driver_count) {
    *driver_count = 1;

    switch (DEVICE) {
        case XINPUT:
            return &xinput_driver;
        case SWITCH:
        case DINPUT:
        case WEB:
            return &hid_driver;
        default:
            break;
    }

}

//--------------------------------------------------------------------+
// USB HID Callbacks (Required) 
//--------------------------------------------------------------------+

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t
tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    // TODO: Handle the correct report type, if required
    (void) itf;

    uint8_t report_size = 0;
    SwitchReport switch_report;

    report_size = sizeof(SwitchReport);
    memcpy(buffer, &switch_report, report_size);

    return report_size;
}

/*LO QUE MANDA EL HOST*/
// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
    (void) itf;

    // echo back anything we received from host
    //tud_hid_report(report_id, buffer, bufsize);
}


//--------------------------------------------------------------------+
// Send Report Device
//--------------------------------------------------------------------+

static inline uint32_t board_millis(void) {
    return to_ms_since_boot(get_absolute_time());
}

static void sendReportData(void *original_data) {

    // Poll every 1ms
    const uint32_t interval_ms = 1;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms) return;  // not enough time
    start_ms += interval_ms;

    // PROCESS DATA
    switch (DEVICE) {
        case XINPUT:
            new_report_fun(original_data, HOST, &xinputReport, XINPUT);
            break;
        case BLUETOOTH:
#if PICO_W
            new_report_fun(original_data, HOST, &bluetooth_report, BLUETOOTH);
#endif
            break;
        case SWITCH:
            new_report_fun(original_data, HOST, &switchReport, SWITCH);
            break;
        case DINPUT:
            new_report_fun(original_data, HOST, &hidReport, DINPUT);
            break;
        case PSX:
            new_report_fun(original_data, HOST, &psxReport, PSX);
            break;
        case WII:
#if PICO_W
            new_report_fun(original_data, HOST, &wiimote_report, WII);
#endif
            break;
        default:
            break;
    }

    if (tud_suspended())
        tud_remote_wakeup();


    // SEND REPORT
    switch (DEVICE) {
        case XINPUT:
            send_xinput_report(&xinputReport, sizeof(XInputReport));
            break;
        case SWITCH:
            send_hid_report(0, &switchReport, sizeof(switchReport));
            break;
        case DINPUT:
            send_hid_report(0, &hidReport, sizeof(hidReport));
            break;
        default:
            break;
    }

}


//--------------------------------------------------------------------+
// Init Host
//--------------------------------------------------------------------+

usbh_class_driver_t const *usbh_app_driver_get_cb(uint8_t *driver_count) {

    *driver_count = 1;

    switch (HOST) {
        case XINPUT:
            return &xinput_driver_h;
        default:
            *driver_count = 0;
            return 0;
    }
}

//--------------------------------------------------------------------+
// Host Xinput
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_xinput_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
    xinputh_interface_t *xid_itf = (xinputh_interface_t *) report;
    xinput_gamepad_t *p = &xid_itf->pad;

    if (xid_itf->connected && xid_itf->new_pad_data) {
        sendReportData(p);
    }
    tuh_xinput_receive_report(dev_addr, instance);
}

void tuh_xinput_mount_cb(uint8_t dev_addr, uint8_t instance, const xinputh_interface_t *xinput_itf) {
    HOST = XINPUT;

    printf("XINPUT MOUNTED %02x %d\n", dev_addr, instance);
    // If this is a Xbox 360 Wireless controller we need to wait for a connection packet
    // on the in pipe before setting LEDs etc. So just start getting data until a controller is connected.
    if (xinput_itf->type == XBOX360_WIRELESS && xinput_itf->connected == false) {
        tuh_xinput_receive_report(dev_addr, instance);
        return;
    }
    tuh_xinput_set_led(dev_addr, instance, 0, true);
    tuh_xinput_set_led(dev_addr, instance, 1, true);
    tuh_xinput_set_rumble(dev_addr, instance, 0, 0, true);
    tuh_xinput_receive_report(dev_addr, instance);

    led_on(); //LED ON!

}

void tuh_xinput_umount_cb(uint8_t dev_addr, uint8_t instance) {
    printf("XINPUT UNMOUNTED %02x %d\n", dev_addr, instance);

    led_off();; //LED OFF!
}


//--------------------------------------------------------------------+
// Host HID
//--------------------------------------------------------------------+

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {

    HOST = DINPUT;

    //LED ON
    led_on();

    uint16_t vid, pid;
    tuh_vid_pid_get(dev_addr, &vid, &pid);

    printf("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
    printf("VID = %04x, PID = %04x\r\n", vid, pid);

    /*FALTA ACOMODAR!!!!!!*/
    if (vid == 0x046d && (pid == 0xc219 || pid == 0xc216 || pid == 0xc218)) {
        HID_HOST = LOGITECH;
    } else if (vid == 0x054c && pid == 0x0268) {
        HID_HOST = PS3;
    } else if (vid == 0x2dc8 && pid == 0x3013) {
        HID_HOST = EIGHT_BITDO;
    } else if ((vid == 0x054c && (pid == 0x09cc || pid == 0x05c4)) || (vid == 0x0f0d && pid == 0x005e)
               || (vid == 0x0f0d && pid == 0x00ee) || (vid == 0x1f4f && pid == 0x1002)) {

        HID_HOST = PS4;
    } else {
        uint8_t ret = generate_HID_report_info(desc_report, desc_len);
        if (ret != 0) {
            printf("Error: USB_ProcessHIDReport failed: %d\r\n", ret);
            return;
        }
        HID_HOST = GENERIC;
    }

    // request to receive report
    if (!tuh_hid_receive_report(dev_addr, instance)) {
        printf("Error: cannot request to receive report\r\n");
    }

}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    printf("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
    //LED OFF
    led_off();
    free_HID_report_info();
}


// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {

    if (len != 0) {
        switch (HID_HOST) {
            case LOGITECH:
            case EIGHT_BITDO:
            case PS3: {
                uint8_t report_copy[len];
                memcpy(&report_copy, report, len);
                report_copy[0] = HID_HOST;
                sendReportData(report_copy);
            }
                break;
            case PS4: {
                //awful trick to don't collapse with LOGITECH case
                uint8_t report_copy[10];
                memcpy(&report_copy[2], &report[1], 4);
                memcpy(&report_copy[6], &report[6], 4);
                report_copy[1] = report[5];
                report_copy[0] = HID_HOST;
                sendReportData(report_copy);
            }
                break;
            case GENERIC: {
                uint8_t report_copy[7];
                parse_report(report, len, report_copy);
                sendReportData(report_copy);
            }
                break;
        }
    }

    // continue to request to receive report
    if (!tuh_hid_receive_report(dev_addr, instance)) {
        printf("Error: cannot request to receive report\r\n");
    }
}

//--------------------------------------------------------------------+
// Host Keyboard PS/2
//--------------------------------------------------------------------+
void read_keyboard(uint32_t keyboard_t) {

    sendReportData(&keyboard_t);
}

//--------------------------------------------------------------------+
// Host PS1/PS2 Controller
//--------------------------------------------------------------------+
void read_psx_controller(uint32_t *psx_data) {

    (psx_data[2] >> 24) == 0x5a ? led_on() : led_off();

    sendReportData(psx_data);
}