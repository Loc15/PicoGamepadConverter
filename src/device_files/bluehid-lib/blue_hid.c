#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "pico/cyw43_arch.h"

#include "btstack_config.h"
#include "btstack.h"
#include "bluetooth_descriptors.h"


static uint8_t hid_service_buffer[700];
static uint8_t pnp_service_buffer[200];
static const char hid_device_name[] = "PicoGamepad";
static btstack_packet_callback_registration_t hci_event_callback_registration;
static bool connected = false;
static uint16_t hid_cid;
static btstack_timer_source_t led_state;
void (*callback_led_func_b[2])();

uint8_t report[50];

volatile BluetoothReport *bluetooth_report;

// HID Report sending
static void send_report(){
    memcpy(&report[2], (uint8_t *)bluetooth_report, sizeof(BluetoothReport));
    hid_device_send_interrupt_message(hid_cid, report, sizeof(BluetoothReport) + 2);
    hid_device_request_can_send_now_event(hid_cid);
}

static void hid_embedded_start_gamepad(void){
    printf("Start gamepad..\n");
    report[0] = 0xA1;
    report[1] = 0x03;

    hid_device_request_can_send_now_event(hid_cid);
}

static void led_handler(struct btstack_timer_source *ts)
{
    // Invert the led
    static uint8_t led_on = 0;
    led_on = !led_on;
    (*callback_led_func_b[led_on])();

    // Restart timer
    btstack_run_loop_set_timer(ts, 100);
    btstack_run_loop_add_timer(ts);
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size){
    UNUSED(channel);
    UNUSED(packet_size);
    uint8_t status;
    switch (packet_type){
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)){
                case BTSTACK_EVENT_STATE:
                    if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
                    connected = false;
                    break;

                case HCI_EVENT_USER_CONFIRMATION_REQUEST:
                    // ssp: inform about user confirmation request
                    log_info("SSP User Confirmation Request with numeric value '%06"PRIu32"'\n", hci_event_user_confirmation_request_get_numeric_value(packet));
                    log_info("SSP User Confirmation Auto accept\n");                   
                    break; 

                case HCI_EVENT_HID_META:
                    switch (hci_event_hid_meta_get_subevent_code(packet)){
                        case HID_SUBEVENT_CONNECTION_OPENED:
                            status = hid_subevent_connection_opened_get_status(packet);
                            if (status != ERROR_CODE_SUCCESS) {
                                // outgoing connection failed
                                printf("Connection failed, status 0x%x\n", status);
                                connected = false;
                                hid_cid = 0;
                                return;
                            }
                            connected = true;
                            hid_cid = hid_subevent_connection_opened_get_hid_cid(packet);
                            // Remove timer led
                            btstack_run_loop_remove_timer(&led_state);
                            // Set the led on
                            (*callback_led_func_b[1])();
                            //START
                            hid_embedded_start_gamepad();
                            break;
                        case HID_SUBEVENT_CONNECTION_CLOSED:
                            printf("HID Disconnected\n");
                            connected = false;
                            hid_cid = 0;
                            // Start led timer
                            btstack_run_loop_set_timer(&led_state, 100);
                            btstack_run_loop_add_timer(&led_state);
                            break;
                        case HID_SUBEVENT_CAN_SEND_NOW:
                            send_report();
                            break;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

int btstack_hid(void *bluetooth_data){

    bluetooth_report = bluetooth_data;

    //init_led function already init the wifi
    /*if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        return -1;
    }*/

    // allow to get found by inquiry
    gap_discoverable_control(1);
    // use Limited Discoverable Mode; Peripheral;
    gap_set_class_of_device(0x2508);
    // set local name to be identified - zeroes will be replaced by actual BD ADDR
    gap_set_local_name("PicoGamepad");
    // allow for role switch in general and sniff mode
    gap_set_default_link_policy_settings( LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );
    // allow for role switch on outgoing connections - this allow HID Host to become master when we re-connect to it
    gap_set_allow_role_switch(true);

    // L2CAP
    l2cap_init();

#ifdef ENABLE_BLE
    // Initialize LE Security Manager. Needed for cross-transport key derivation
    sm_init();
#endif

    // SDP Server
    sdp_init();
    memset(hid_service_buffer, 0, sizeof(hid_service_buffer));

    hid_sdp_record_t hid_sdp_record = {
    	// hid sevice subclass 2508 Gamepad, hid counntry code 33 US
    	0x2508,
        33,
        1,
        1,
        1,
        0,
        0,
        0xFFFF,
        0xFFFF,
        3200,
        bluetooth_report_descriptor,
        sizeof(bluetooth_report_descriptor),
        hid_device_name
    };

    hid_create_sdp_record(hid_service_buffer, sdp_create_service_record_handle(), &hid_sdp_record);
    btstack_assert(de_get_len( hid_service_buffer) <= sizeof(hid_service_buffer));
    sdp_register_service(hid_service_buffer);

	memset(pnp_service_buffer, 0, sizeof(pnp_service_buffer));
  	sdp_register_service(pnp_service_data);

  	// HID Device
  	hid_device_init(0, sizeof(bluetooth_report_descriptor),bluetooth_report_descriptor);
       
    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // register for HID events
    hid_device_register_packet_handler(&packet_handler);

    // turn on!
    hci_power_control(HCI_POWER_ON);

    btstack_run_loop_execute();

    return 0;
}

void btstack_hid_set_led(void (*led_on)(), void (*led_off)()){
    callback_led_func_b[0] = led_off;
    callback_led_func_b[1] = led_on;

    led_state.process = &led_handler;
    btstack_run_loop_set_timer(&led_state, 100);
    btstack_run_loop_add_timer(&led_state);
}