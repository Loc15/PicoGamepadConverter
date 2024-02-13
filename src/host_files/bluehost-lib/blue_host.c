#include <inttypes.h>
#include <stdio.h>
#include "pico/cyw43_arch.h"
#include "btstack_config.h"
#include "btstack.h"

#include "pico_flash.h"
#include "gamepad_parser.h"

typedef enum{
    GENERIC,
    PS4=2,
    EIGHT_BITDO=5,
}HID_TYPE;

#define MAX_ATTRIBUTE_VALUE_SIZE 512

//MAC ADDRESS
static bd_addr_t remote_addr;
//PACKET HANDLER STRUCTURE
static btstack_packet_callback_registration_t hci_event_callback_registration;
//HID DESCRIPTOR STORAGE
static uint8_t hid_descriptor_storage[MAX_ATTRIBUTE_VALUE_SIZE];
static bool hid_host_descriptor_available = false;
static uint16_t hid_host_cid = 0;
//Descriptor parse from generic mode
static bool first_time = false;

HID_TYPE TYPE = GENERIC;
void (*callback_send)( void * ) = {0}; //callback from the main

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void process_hid_event(uint8_t* packet);

static void connected_led(int state) {
  cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state);
}

void read_data(){
    //First the mac address
    for(int i = 0; i < 6; i++){
        remote_addr[i] = read_flash(2 + i);
    }
    //HID type controller
    TYPE = read_flash(8);
}

static void hid_host_setup(void){

    // Initialize L2CAP
    l2cap_init();

    // Initialize HID Host
    hid_host_init(hid_descriptor_storage, sizeof(hid_descriptor_storage));
    hid_host_register_packet_handler(packet_handler);

    // Allow sniff mode requests by HID device and support role switch
    gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_SNIFF_MODE | LM_LINK_POLICY_ENABLE_ROLE_SWITCH);

    // try to become master on incoming connections
    hci_set_master_slave_policy(HCI_ROLE_MASTER);

    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);
}


static void hid_host_handle_interrupt_report(const uint8_t * report, uint16_t report_len){
    // check if HID Input Report
    if (report_len < 1) return;
    if (*report != 0xa1) return;

    uint8_t report_copy[report_len];

    switch(TYPE){
    case EIGHT_BITDO:
        //the data it's different to USB mode
        report_copy[0] = TYPE;  //ojo con la data que se manda web
        report_copy[1] = report[9];
        report_copy[2] = report[10];
        report_copy[4] = (report[2] << 4);
        //Joystick data
        report_copy[5] = report[3];
        report_copy[6] = report[4];
        report_copy[7] = report[5];
        report_copy[8] = report[6];
        //triggers data
        report_copy[9] = report[7];
        report_copy[10] = report[8];
        //send_data
        callback_send(report_copy);
        break;
    case PS4:
        //Set same format of USB MODE
        //Joystick data
        memcpy(&report_copy[2], &report[2], 4);
        //remaining data
        memcpy(&report_copy[6], &report[7], 4);
        //dpad + buttons
        report_copy[1] = report[6];
        report_copy[0] = TYPE;
        //send_data
        callback_send(report_copy);
        break;
    case GENERIC:
        //Same parse from USB MODE
        report++;
        report_len--;
        if(!first_time){
            uint8_t ret = generate_HID_report_info(hid_descriptor_storage_get_descriptor_data(hid_host_cid), 
                hid_descriptor_storage_get_descriptor_len(hid_host_cid));
            if(ret != 0){
                printf("Error: USB_ProcessHIDReport failed: %d\r\n", ret);
                return;
            }
            first_time = true;
        }
        //parse data
        uint8_t report_copy_g[7];
        parse_report(report, report_len, report_copy_g);
        //send_data
        callback_send(report_copy_g);
        break;
    default:
        break;
    }
}


static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    uint8_t   event;
    bd_addr_t event_addr;
    uint8_t status;

    if(packet_type != HCI_EVENT_PACKET)
        return;

    event = hci_event_packet_get_type(packet);
    switch(event) {
        case BTSTACK_EVENT_STATE: {
            if(btstack_event_state_get_state(packet) == HCI_STATE_WORKING) {
                status = hid_host_connect(remote_addr, HID_PROTOCOL_MODE_REPORT, &hid_host_cid);
                //----------------
                if (status != ERROR_CODE_SUCCESS){
                    printf("HID host connect failed, status 0x%02x.\n", status);
                }
                //---------------
            }
            break;
        }
        case HCI_EVENT_PIN_CODE_REQUEST: {
            hci_event_pin_code_request_get_bd_addr(packet, event_addr);
            gap_pin_code_response(event_addr, "0000");
            break;
        }
        case HCI_EVENT_HID_META: {
            process_hid_event(packet);
            break;
        }
        case HCI_EVENT_DISCONNECTION_COMPLETE: {
            connected_led(0);
            break;
            
        }
    }
}


static void process_hid_event(uint8_t* packet){

	int8_t hid_event = hci_event_hid_meta_get_subevent_code(packet);
    bd_addr_t event_addr;
    uint8_t status;

    switch(hid_event) {
        case HID_SUBEVENT_INCOMING_CONNECTION: {
            hid_subevent_incoming_connection_get_address(packet, event_addr);
            hid_host_accept_connection(hid_subevent_incoming_connection_get_hid_cid(packet), HID_PROTOCOL_MODE_REPORT);
            break;
        }
        case HID_SUBEVENT_CONNECTION_OPENED: {
            uint8_t status = hid_subevent_connection_opened_get_status(packet);
            hid_subevent_connection_opened_get_bd_addr(packet, event_addr);
            if(status != ERROR_CODE_SUCCESS) {
            	printf("Connection failed, status 0x%02x\n", status);
                hid_host_cid = 0;
                return;
            }
            hid_host_descriptor_available = false;
            hid_host_cid = hid_subevent_connection_opened_get_hid_cid(packet);
            break;
        }
        case HID_SUBEVENT_DESCRIPTOR_AVAILABLE: {
            uint8_t status = hid_subevent_descriptor_available_get_status(packet);
            if(status == ERROR_CODE_SUCCESS) {
                hid_host_descriptor_available = true;
                connected_led(1);
                //PS4
                //hid_host_send_get_report(hid_host_cid, HID_REPORT_TYPE_FEATURE, 0x05);
            }
            break;
        }
        case HID_SUBEVENT_REPORT: {
            if(hid_host_descriptor_available) {
                hid_host_handle_interrupt_report(hid_subevent_report_get_report(packet), hid_subevent_report_get_report_len(packet));
            }
            break;
        }
        case HID_SUBEVENT_CONNECTION_CLOSED: {
            // The connection was closed.
            hid_host_cid = 0;
            hid_host_descriptor_available = false;
            free_HID_report_info();
            first_time = false;
            break;
        }
    }
}


void btstack_host(void (*fn)(void *)){

    //init_led function already init the wifi
    /*if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        return;
    }*/

    //Read data from flash
    read_data();

    //callback send report function
    callback_send = fn;

    hid_host_setup();

    printf("la direccion seteada es %s\n", bd_addr_to_str(remote_addr));
    printf("El tipo de HID es %d\n", TYPE);

    // Turn on the device 
    hci_power_control(HCI_POWER_ON);

    btstack_run_loop_execute();


}