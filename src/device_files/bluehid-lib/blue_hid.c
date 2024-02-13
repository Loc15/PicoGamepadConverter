#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "pico/cyw43_arch.h"

#include "btstack_config.h"
#include "btstack.h"
#include "../switch/SwitchDescriptors.h"
#include "BtStackUtils.h"


static uint8_t hid_service_buffer[700];
static uint8_t pnp_service_buffer[200];
static const char hid_device_name[] = "Wireless Gamepad";
static btstack_packet_callback_registration_t hci_event_callback_registration;
static bool connected = false;
static uint16_t hid_cid;

uint8_t report[50];

volatile SwitchReport *bluetooth_switch_report;

// HID Report sending
static void send_report(){
    memcpy(&report[2], (uint8_t *)bluetooth_switch_report, sizeof(SwitchReport));
    hid_device_send_interrupt_message(hid_cid, report, sizeof(SwitchReport) + 2);
    hid_device_request_can_send_now_event(hid_cid);
}

void new_data(void *switch_report){

    memcpy(&report[2], (uint8_t *)switch_report, sizeof(SwitchReport));
}

static void hid_embedded_start_gamepad(void){
    printf("Start gamepad..\n");
    report[0] = 0xA1;
    report[1] = 0x3F;

    hid_device_request_can_send_now_event(hid_cid);
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
                            //TESTING
                            hid_embedded_start_gamepad();
                            //TESTING
                            break;
                        case HID_SUBEVENT_CONNECTION_CLOSED:
                            printf("HID Disconnected\n");
                            connected = false;
                            hid_cid = 0;
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

int btstack_hid(void *switch_data){

    bluetooth_switch_report = switch_data;

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
    gap_set_local_name("Pro Controller");
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
        switch_bt_report_descriptor,
        sizeof(switch_bt_report_descriptor),
        hid_device_name
    };


    create_sdp_hid_record(hid_service_buffer, &hid_sdp_record);
    sdp_register_service(hid_service_buffer);

	memset(pnp_service_buffer, 0, sizeof(pnp_service_buffer));
  	create_sdp_pnp_record(pnp_service_buffer, DEVICE_ID_VENDOR_ID_SOURCE_USB,
                        0x057E, 0x2009, 0x0001);
  	sdp_register_service(pnp_service_buffer);

  	// HID Device
  	hid_device_init(1, sizeof(switch_bt_report_descriptor),switch_bt_report_descriptor);

       
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