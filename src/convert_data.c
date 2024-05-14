#include <string.h>
#include <math.h>

//Device
#include "SwitchDescriptors.h"
#include "PS3_Descriptors.h"
#include "controller_simulator.h"
#include "wiimote.h"
//Host
#include "xinput_definitions.h"
#include "hid_definitions.h"
#include "psx_definitions.h"
//itself
#include "convert_data.h"

//Definitions
#define XINPUT_TO_HID_X(a) ((a >> 8) - 0x80)
#define XINPUT_TO_HID_Y(a) -((a >> 8) - 0x7F)
//#define XINPUT_TO_HID_Y(a) -((a >> 8) - 0x80)

#define HID_TO_XINPUT_X(a) ((a - 0x80) << 8)
#define HID_TO_XINPUT_Y(a) -((a - 0x7F) << 8)

#define KEYBOARD_MASK_REVERSE(a,b)    ((a >> b)&1)

#define XINPUT_TO_WIIMOTE(a) ((a>>8))
#define XINPUT_TO_CLASSIC_X(a) ((a>>10) + 32)
#define XINPUT_TO_CLASSIC_Y(a) ((a>>11) + 32)

/*XINPUT TO SWITCH*/
const uint8_t SWITCH_DEVICE_HAT[] = {SWITCH_HAT_NOTHING, SWITCH_HAT_UP, SWITCH_HAT_DOWN, SWITCH_HAT_NOTHING, SWITCH_HAT_LEFT, 
                                SWITCH_HAT_UPLEFT, SWITCH_HAT_DOWNLEFT, SWITCH_HAT_NOTHING, SWITCH_HAT_RIGHT, 
                                SWITCH_HAT_UPRIGHT, SWITCH_HAT_DOWNRIGHT};

features_t features;
void set_features(xinput_gamepad_t *host_report);


void new_report_fun(void *report, MODE mode_host, void *new_report, MODE mode_device){

    // For wii case
    uint8_t guide_button = 0;
    
    xinput_gamepad_t host_report;

    switch(mode_host){
        case BLUETOOTH:
    	case DINPUT: {
                uint8_t *report_hid = report;
                /*JOYSTICKS DATA START POSITION*/
                uint8_t joystick_pos = report_hid[0];
                /*COPY JOYSTICKS DATA*/
                host_report.sThumbLX = HID_TO_XINPUT_X(report_hid[joystick_pos]);
                host_report.sThumbLY = HID_TO_XINPUT_Y(report_hid[joystick_pos + 1]);
                host_report.sThumbRX = HID_TO_XINPUT_X(report_hid[joystick_pos + 2]);
                host_report.sThumbRY = HID_TO_XINPUT_Y(report_hid[joystick_pos + 3]);
                /*BUTTONS*/
                uint8_t buttons1;
                uint8_t buttons2;
                uint8_t buttons3;
                /*BUTTONS*/
                switch (joystick_pos) {
                    case LOGITECH:
                        /*BUTTONS+DPAD*/
                        buttons1 = report_hid[5];
                        /*THUMBS+START+SELECT+SHOULDERS*/
                        buttons2 = report_hid[6];

                        host_report.wButtons =
                                LOGITECH_DPAD[buttons1 & 0xF] | (buttons1 & LOGITECH_GAMEPAD_A ? XINPUT_GAMEPAD_A : 0) |
                                (buttons1 & LOGITECH_GAMEPAD_B ? XINPUT_GAMEPAD_B : 0) |
                                (buttons1 & LOGITECH_GAMEPAD_X ? XINPUT_GAMEPAD_X : 0) |
                                (buttons1 & LOGITECH_GAMEPAD_Y ? XINPUT_GAMEPAD_Y : 0) |
                                (buttons2 & LOGITECH_GAMEPAD_START ? XINPUT_GAMEPAD_START : 0) |
                                (buttons2 & LOGITECH_GAMEPAD_BACK ? XINPUT_GAMEPAD_BACK : 0) |
                                (buttons2 & LOGITECH_GAMEPAD_LEFT_SHOULDER ? XINPUT_GAMEPAD_LEFT_SHOULDER : 0) |
                                (buttons2 & LOGITECH_GAMEPAD_RIGHT_SHOULDER ? XINPUT_GAMEPAD_RIGHT_SHOULDER : 0) |
                                (buttons2 & LOGITECH_GAMEPAD_LEFT_THUMB ? XINPUT_GAMEPAD_LEFT_THUMB : 0) |
                                (buttons2 & LOGITECH_GAMEPAD_RIGHT_THUMB ? XINPUT_GAMEPAD_RIGHT_THUMB : 0);
                        host_report.bLeftTrigger = ((buttons2 & LOGITECH_GAMEPAD_LEFT_TRIGGER) * 0xFF);
                        host_report.bRightTrigger = ((buttons2 & LOGITECH_GAMEPAD_RIGHT_TRIGGER) * 0xFF);
                        /*FALTA HOME BUTTON*/
                        break;
                    case PS3:
                        /*DPAD+SELECT+THUMBS+START*/
                        buttons1 = report_hid[2];
                        /*BUTTONS+SHOULDERS*/
                        buttons2 = report_hid[3];
                        /*GUIDE BUTTON*/
                        buttons3 = report_hid[4] & 0x1;


                        host_report.wButtons = (buttons1 & PS3_GAMEPAD_DPAD_UP ? XINPUT_GAMEPAD_DPAD_UP : 0) |
                                               (buttons1 & PS3_GAMEPAD_DPAD_RIGHT ? XINPUT_GAMEPAD_DPAD_RIGHT : 0) |
                                               (buttons1 & PS3_GAMEPAD_DPAD_DOWN ? XINPUT_GAMEPAD_DPAD_DOWN : 0) |
                                               (buttons1 & PS3_GAMEPAD_DPAD_LEFT ? XINPUT_GAMEPAD_DPAD_LEFT : 0) |
                                               (buttons1 & PS3_GAMEPAD_START ? XINPUT_GAMEPAD_START : 0) |
                                               (buttons1 & PS3_GAMEPAD_SELECT ? XINPUT_GAMEPAD_BACK : 0) |
                                               (buttons1 & PS3_GAMEPAD_L3 ? XINPUT_GAMEPAD_LEFT_THUMB : 0) |
                                               (buttons1 & PS3_GAMEPAD_R3 ? XINPUT_GAMEPAD_RIGHT_THUMB : 0) |
                                               (buttons2 & PS3_GAMEPAD_CROSS ? XINPUT_GAMEPAD_A : 0) |
                                               (buttons2 & PS3_GAMEPAD_CIRCLE ? XINPUT_GAMEPAD_B : 0) |
                                               (buttons2 & PS3_GAMEPAD_SQUARE ? XINPUT_GAMEPAD_X : 0) |
                                               (buttons2 & PS3_GAMEPAD_TRIANGLE ? XINPUT_GAMEPAD_A : 0) |
                                               (buttons2 & PS3_GAMEPAD_R1 ? XINPUT_GAMEPAD_LEFT_SHOULDER : 0) |
                                               (buttons2 & PS3_GAMEPAD_R2 ? XINPUT_GAMEPAD_RIGHT_SHOULDER : 0) |
                                               (buttons3 ? XINPUT_GAMEPAD_GUIDE : 0);
                        host_report.bLeftTrigger = ((buttons2 & PS3_GAMEPAD_L2) * 0xFF);
                        host_report.bRightTrigger = ((buttons2 & PS3_GAMEPAD_L1) * 0xFF);

                        guide_button = 1;
                        break;
                    case EIGHT_BITDO:
                        /*BUTTONS+SHOULDERS*/
                        buttons1 = report_hid[1];
                        /*TRIGGER+SELECT+THUMBS+START*/
                        buttons2 = report_hid[2];
                        /*DPAD*/
                        buttons3 = (report_hid[4] >> 4);

                        host_report.wButtons = LOGITECH_DPAD[buttons3 > 7 ? 8 : buttons3] |
                                               (buttons1 & EIGHT_BITDO_GAMEPAD_A ? XINPUT_GAMEPAD_A : 0) |
                                               (buttons1 & EIGHT_BITDO_GAMEPAD_B ? XINPUT_GAMEPAD_B : 0) |
                                               (buttons1 & EIGHT_BITDO_GAMEPAD_X ? XINPUT_GAMEPAD_X : 0) |
                                               (buttons1 & EIGHT_BITDO_GAMEPAD_Y ? XINPUT_GAMEPAD_Y : 0) |
                                               (buttons2 & EIGHT_BITDO_GAMEPAD_START ? XINPUT_GAMEPAD_START : 0) |
                                               (buttons2 & EIGHT_BITDO_GAMEPAD_BACK ? XINPUT_GAMEPAD_BACK : 0) |
                                               (buttons1 & EIGHT_BITDO_GAMEPAD_LEFT_SHOULDER ? XINPUT_GAMEPAD_LEFT_SHOULDER
                                                                                             : 0) |
                                               (buttons1 & EIGHT_BITDO_GAMEPAD_RIGHT_SHOULDER
                                                ? XINPUT_GAMEPAD_RIGHT_SHOULDER : 0) |
                                               (buttons2 & EIGHT_BITDO_GAMEPAD_LEFT_THUMB ? XINPUT_GAMEPAD_LEFT_THUMB : 0) |
                                               (buttons2 & EIGHT_BITDO_GAMEPAD_RIGHT_THUMB ? XINPUT_GAMEPAD_RIGHT_THUMB
                                                                                           : 0);
                        host_report.bLeftTrigger = report_hid[10];
                        host_report.bRightTrigger = report_hid[9];
                        break;
                    case PS4:
                        /*BUTTONS+DPAD*/
                        buttons1 = report_hid[1];
                        /*THUMBS+START+SELECT+SHOULDERS*/
                        buttons2 = report_hid[6];
                        /*GUIDE BUTTON*/
                        buttons3 = report_hid[7] & 0x1;

                        host_report.wButtons =
                                LOGITECH_DPAD[buttons1 & 0xF] | (buttons1 & LOGITECH_GAMEPAD_A ? XINPUT_GAMEPAD_A : 0) |
                                (buttons1 & LOGITECH_GAMEPAD_B ? XINPUT_GAMEPAD_B : 0) |
                                (buttons1 & LOGITECH_GAMEPAD_X ? XINPUT_GAMEPAD_X : 0) |
                                (buttons1 & LOGITECH_GAMEPAD_Y ? XINPUT_GAMEPAD_Y : 0) |
                                (buttons2 & LOGITECH_GAMEPAD_START ? XINPUT_GAMEPAD_START : 0) |
                                (buttons2 & LOGITECH_GAMEPAD_BACK ? XINPUT_GAMEPAD_BACK : 0) |
                                (buttons2 & LOGITECH_GAMEPAD_LEFT_SHOULDER ? XINPUT_GAMEPAD_LEFT_SHOULDER : 0) |
                                (buttons2 & LOGITECH_GAMEPAD_RIGHT_SHOULDER ? XINPUT_GAMEPAD_RIGHT_SHOULDER : 0) |
                                (buttons2 & LOGITECH_GAMEPAD_LEFT_THUMB ? XINPUT_GAMEPAD_LEFT_THUMB : 0) |
                                (buttons2 & LOGITECH_GAMEPAD_RIGHT_THUMB ? XINPUT_GAMEPAD_RIGHT_THUMB : 0) |
                                (buttons3 ? XINPUT_GAMEPAD_GUIDE : 0);
                        host_report.bLeftTrigger = report_hid[8];
                        host_report.bRightTrigger = report_hid[9];

                        guide_button = 1;
                        break;
                    default:
                        break;
                }
            }
            break;   
        case KBD_PS2:
            memcpy(&host_report, (uint32_t*)report, sizeof(uint16_t));
            host_report.sThumbLX = ((KEYBOARD_MASK_REVERSE(*(uint32_t*)report, 16) * -32768) | 
                                    (KEYBOARD_MASK_REVERSE(*(uint32_t*)report, 17) * 32767));
            host_report.sThumbLY = ((KEYBOARD_MASK_REVERSE(*(uint32_t*)report, 18) * 32767) | 
                                    (KEYBOARD_MASK_REVERSE(*(uint32_t*)report, 19) * -32768));
            //NO RIGHT JOYSTICK
            host_report.sThumbRX = 0;
            host_report.sThumbRY = 0;

            host_report.bLeftTrigger = (KEYBOARD_MASK_REVERSE(*(uint32_t*)report, 24) * 0xFF);
            host_report.bRightTrigger = (KEYBOARD_MASK_REVERSE(*(uint32_t*)report, 25) * 0xFF);

            guide_button = 1;
            break;
        case PSX: {
                uint32_t * psx_data = report;
                /*COPY DATA BUTTONS [3] AND [4]*/
                host_report.wButtons = (DATA_SHIFT(psx_data[3]) & PSX_GAMEPAD_DPAD_UP ? 0 : XINPUT_GAMEPAD_DPAD_UP) |
                                       ((DATA_SHIFT(psx_data[3]) & PSX_GAMEPAD_DPAD_RIGHT ? 0
                                                                                          : XINPUT_GAMEPAD_DPAD_RIGHT)) |
                                       (DATA_SHIFT(psx_data[3]) & PSX_GAMEPAD_DPAD_DOWN ? 0 : XINPUT_GAMEPAD_DPAD_DOWN) |
                                       (DATA_SHIFT(psx_data[3]) & PSX_GAMEPAD_DPAD_LEFT ? 0 : XINPUT_GAMEPAD_DPAD_LEFT) |
                                       (DATA_SHIFT(psx_data[3]) & PSX_GAMEPAD_START ? 0 : XINPUT_GAMEPAD_START) |
                                       (DATA_SHIFT(psx_data[3]) & PSX_GAMEPAD_SELECT ? 0 : XINPUT_GAMEPAD_BACK) |
                                       (DATA_SHIFT(psx_data[3]) & PSX_GAMEPAD_R3 ? 0 : XINPUT_GAMEPAD_RIGHT_THUMB) |
                                       (DATA_SHIFT(psx_data[3]) & PSX_GAMEPAD_L3 ? 0 : XINPUT_GAMEPAD_LEFT_THUMB) |
                                       (DATA_SHIFT(psx_data[4]) & PSX_GAMEPAD_R1 ? 0 : XINPUT_GAMEPAD_RIGHT_SHOULDER) |
                                       (DATA_SHIFT(psx_data[4]) & PSX_GAMEPAD_L1 ? 0 : XINPUT_GAMEPAD_LEFT_SHOULDER) |
                                       (DATA_SHIFT(psx_data[4]) & PSX_GAMEPAD_TRIANGLE ? 0 : XINPUT_GAMEPAD_Y) |
                                       (DATA_SHIFT(psx_data[4]) & PSX_GAMEPAD_CIRCLE ? 0 : XINPUT_GAMEPAD_B) |
                                       (DATA_SHIFT(psx_data[4]) & PSX_GAMEPAD_CROSS ? 0 : XINPUT_GAMEPAD_A) |
                                       (DATA_SHIFT(psx_data[4]) & PSX_GAMEPAD_SQUARE ? 0 : XINPUT_GAMEPAD_X);
                /*COPY JOYSTICKS DATA*/
                /*IS CONTROLLER MODE DIGITAL?*/
                if (DATA_SHIFT(psx_data[1]) == 0X41) {
                    host_report.sThumbLX = 0;
                    host_report.sThumbLY = 0;
                    host_report.sThumbRX = 0;
                    host_report.sThumbRY = 0;
                } else {
                    host_report.sThumbLX = HID_TO_XINPUT_X(DATA_SHIFT(psx_data[7]));
                    host_report.sThumbLY = HID_TO_XINPUT_Y(DATA_SHIFT(psx_data[8]));
                    host_report.sThumbRX = HID_TO_XINPUT_X(DATA_SHIFT(psx_data[5]));
                    host_report.sThumbRY = HID_TO_XINPUT_Y(DATA_SHIFT(psx_data[6]));
                }
                /*TRIGGERS*/
                host_report.bLeftTrigger = (DATA_SHIFT(psx_data[4]) & PSX_GAMEPAD_L2 ? 0 : 0xFF);
                host_report.bRightTrigger = (DATA_SHIFT(psx_data[4]) & PSX_GAMEPAD_R2 ? 0 : 0xFF);
            }
            break;
        default:
            /*XINPUT*/
            memcpy(&host_report, (xinput_gamepad_t*)report, sizeof(xinput_gamepad_t));
            break;		
    }


    //FEATURES ENABLED
    if(features.set_features)
        set_features(&host_report);

    switch(mode_device){
        case SWITCH:{
                SwitchReport *device_report = new_report;

                /*new data*/
                device_report->buttons = (host_report.wButtons & XINPUT_GAMEPAD_A ? SWITCH_MASK_B : 0) | (host_report.wButtons & XINPUT_GAMEPAD_B ? SWITCH_MASK_A : 0)
                                | (host_report.wButtons & XINPUT_GAMEPAD_X ? SWITCH_MASK_Y : 0) | (host_report.wButtons & XINPUT_GAMEPAD_Y ? SWITCH_MASK_X : 0)
                                | (host_report.bLeftTrigger > 0x7F ? SWITCH_MASK_L : 0) | (host_report.bRightTrigger > 0x7F ? SWITCH_MASK_R : 0) 
                                | (host_report.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER ? SWITCH_MASK_ZL : 0) | (host_report.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER ? SWITCH_MASK_ZR : 0)
                                | (host_report.wButtons & XINPUT_GAMEPAD_START ? SWITCH_MASK_PLUS : 0) | (host_report.wButtons & XINPUT_GAMEPAD_BACK ? SWITCH_MASK_MINUS : 0)
                                | (host_report.wButtons & XINPUT_GAMEPAD_LEFT_THUMB ? SWITCH_MASK_L3 : 0) | (host_report.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB ? SWITCH_MASK_R3 : 0)
                                | (host_report.wButtons & XINPUT_GAMEPAD_GUIDE ? SWITCH_MASK_HOME : 0);

                device_report->hat = SWITCH_DEVICE_HAT[host_report.wButtons&0xF];
            
                device_report->lx = XINPUT_TO_HID_X(host_report.sThumbLX);
                device_report->ly = XINPUT_TO_HID_Y(host_report.sThumbLY);
                device_report->rx = XINPUT_TO_HID_X(host_report.sThumbRX);
                device_report->ry = XINPUT_TO_HID_Y(host_report.sThumbRY);
            }
            break;
        case DINPUT:{
            HIDReport *device_report = new_report;

            /*new data*/
            device_report->square_btn = (host_report.wButtons & XINPUT_GAMEPAD_X ? 1 : 0);
            device_report->cross_btn = (host_report.wButtons & XINPUT_GAMEPAD_A ? 1 : 0);
            device_report->circle_btn = (host_report.wButtons & XINPUT_GAMEPAD_B ? 1 : 0);
            device_report->triangle_btn = (host_report.wButtons & XINPUT_GAMEPAD_Y ? 1 : 0);

            device_report->l1_btn = (host_report.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER ? 1 : 0);
            device_report->r1_btn = (host_report.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER ? 1 : 0);
            device_report->l2_btn = (host_report.bLeftTrigger > 127 ? 1 : 0);
            device_report->r2_btn = (host_report.bRightTrigger > 127 ? 1 : 0);

            device_report->select_btn = (host_report.wButtons & XINPUT_GAMEPAD_BACK ? 1 : 0);
            device_report->start_btn = (host_report.wButtons & XINPUT_GAMEPAD_START ? 1 : 0);
            device_report->l3_btn = (host_report.wButtons & XINPUT_GAMEPAD_LEFT_THUMB ? 1 : 0);
            device_report->r3_btn = (host_report.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB ? 1 : 0);
            device_report->ps_btn = (host_report.wButtons & XINPUT_GAMEPAD_GUIDE ? 1 : 0);

            device_report->direction = SWITCH_DEVICE_HAT[host_report.wButtons&0xF];

            device_report->l_x_axis = XINPUT_TO_HID_X(host_report.sThumbLX);
            device_report->l_y_axis = XINPUT_TO_HID_Y(host_report.sThumbLY);
            device_report->r_x_axis = XINPUT_TO_HID_X(host_report.sThumbRX);
            device_report->r_y_axis = XINPUT_TO_HID_Y(host_report.sThumbRY);
            }
            break;
        case PSX:{
            PSXInputState *device_report = new_report;

            /*new data*/
            device_report->buttons1 = ~(((host_report.wButtons & XINPUT_GAMEPAD_DPAD_UP ? 1 : 0)<<PSX_DEVICE_UP) | ((host_report.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT ? 1 : 0)<<PSX_DEVICE_RIGHT) |
                                        ((host_report.wButtons & XINPUT_GAMEPAD_DPAD_DOWN ? 1 : 0)<<PSX_DEVICE_DOWN) | ((host_report.wButtons & XINPUT_GAMEPAD_DPAD_LEFT ? 1 : 0)<<PSX_DEVICE_LEFT) |
                                        ((host_report.wButtons & XINPUT_GAMEPAD_START ? 1 : 0)<<PSX_DEVICE_START) | ((host_report.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB ? 1 : 0)<<PSX_DEVICE_R3) |
                                        ((host_report.wButtons & XINPUT_GAMEPAD_LEFT_THUMB ? 1 : 0)<<PSX_DEVICE_L3) | ((host_report.wButtons & XINPUT_GAMEPAD_BACK ? 1 : 0)<<PSX_DEVICE_SELECT));

            device_report->buttons2 = ~(((host_report.wButtons & XINPUT_GAMEPAD_A ? 1 : 0)<<PSX_DEVICE_CROSS) | ((host_report.wButtons & XINPUT_GAMEPAD_B ? 1 : 0)<<PSX_DEVICE_CIRCLE) |
                                        ((host_report.wButtons & XINPUT_GAMEPAD_X ? 1 : 0)<<PSX_DEVICE_SQUARE) | ((host_report.wButtons & XINPUT_GAMEPAD_Y ? 1 : 0)<<PSX_DEVICE_TRIANGLE) |
                                        ((host_report.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER ? 1 : 0)<<PSX_DEVICE_R1) | ((host_report.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER ? 1 : 0)<<PSX_DEVICE_L1) |
                                        ((host_report.bRightTrigger > 127 ? 1 : 0)<<PSX_DEVICE_R2) | ((host_report.bLeftTrigger > 127 ? 1 : 0)<<PSX_DEVICE_L2));

            device_report->lx = XINPUT_TO_HID_X(host_report.sThumbLX);
            device_report->ly = XINPUT_TO_HID_Y(host_report.sThumbLY);
            device_report->rx = XINPUT_TO_HID_X(host_report.sThumbRX);
            device_report->ry = XINPUT_TO_HID_Y(host_report.sThumbRY);

            device_report->l2 = host_report.bLeftTrigger;
            device_report->r2 = host_report.bRightTrigger;
        }
            break;
        case WII:{
            WiimoteReport *device_report = new_report;

            // No guide button for some controllers then using right thumbstick
            if((mode_host != XINPUT) || (!guide_button)){
                host_report.wButtons |= (host_report.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB ? XINPUT_GAMEPAD_GUIDE : 0);
            }

            //wiimote or classic
            switch(device_report->mode){
                case 0:
                    /*new data*/
                    device_report->wiimote.a = host_report.wButtons & XINPUT_GAMEPAD_B;
                    device_report->wiimote.b = host_report.wButtons & XINPUT_GAMEPAD_A;
                    device_report->wiimote.minus = host_report.wButtons & XINPUT_GAMEPAD_BACK;
                    device_report->wiimote.plus = host_report.wButtons & XINPUT_GAMEPAD_START;
                    device_report->wiimote.home = host_report.wButtons & XINPUT_GAMEPAD_GUIDE;
                    device_report->wiimote.one = host_report.wButtons & XINPUT_GAMEPAD_X;
                    device_report->wiimote.two = host_report.wButtons & XINPUT_GAMEPAD_Y;
                    device_report->wiimote.up = host_report.wButtons & XINPUT_GAMEPAD_DPAD_UP;
                    device_report->wiimote.down = host_report.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                    device_report->wiimote.left = host_report.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
                    device_report->wiimote.right = host_report.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
                    device_report->wiimote.ir_x = XINPUT_TO_WIIMOTE(host_report.sThumbLX);
                    device_report->wiimote.ir_y = XINPUT_TO_WIIMOTE(host_report.sThumbLY);

                    /*check if it wanna change the mode*/
                    if(device_report->wiimote.one & device_report->wiimote.two){
                        device_report->switch_mode = 1;
                    }
                        /*wait to change to zero*/
                    else{
                        if(device_report->switch_mode){
                            device_report->mode = 1;
                            device_report->switch_mode = 0;
                        }
                    }
                    break;
                case 1:
                    /*new data*/
                    device_report->classic.a = host_report.wButtons & XINPUT_GAMEPAD_B;
                    device_report->classic.b = host_report.wButtons & XINPUT_GAMEPAD_A;
                    device_report->classic.x = host_report.wButtons & XINPUT_GAMEPAD_Y;
                    device_report->classic.y = host_report.wButtons & XINPUT_GAMEPAD_X;
                    device_report->classic.home = host_report.wButtons & XINPUT_GAMEPAD_GUIDE;
                    device_report->classic.minus = host_report.wButtons & XINPUT_GAMEPAD_BACK;
                    device_report->classic.plus = host_report.wButtons & XINPUT_GAMEPAD_START;
                    device_report->classic.up = host_report.wButtons & XINPUT_GAMEPAD_DPAD_UP;
                    device_report->classic.down = host_report.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
                    device_report->classic.left = host_report.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
                    device_report->classic.right = host_report.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
                    device_report->classic.lz = host_report.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
                    device_report->classic.rz = host_report.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
                    device_report->classic.lt = host_report.bLeftTrigger;
                    device_report->classic.rt = host_report.bRightTrigger;
                    device_report->classic.ltrigger = (host_report.bLeftTrigger > 32 ? 1: 0);         //digital button of lt
                    device_report->classic.rtrigger = (host_report.bRightTrigger > 32 ? 1 : 0);        //digital button of rt
                    device_report->classic.ls_x = XINPUT_TO_CLASSIC_X(host_report.sThumbLX);
                    device_report->classic.ls_y = XINPUT_TO_CLASSIC_Y(host_report.sThumbLY);

                    /*check if it wants change the mode*/
                    if(device_report->classic.x & device_report->classic.y){
                        device_report->switch_mode = 1;
                    }
                        /*wait to change to zero*/
                    else{
                        if(device_report->switch_mode){
                            device_report->mode = 0;
                            device_report->switch_mode = 0;
                            device_report->reset_ir = 1;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
            break;  
        default:
            /*XINPUT*/
            //diferent structure host -> device
            memcpy(new_report+2, &host_report, sizeof(xinput_gamepad_t));
            break;
    }
}



//--------------------------------------------------------------------+
// Features
//--------------------------------------------------------------------+

void set_features_from_flash(unsigned char *data){
    //set features 
    //void incorrect data from flash
    features.set_features = (data[0] > 1 ? 0 : data[0]);
    //num features
    features.num_enabled_features = data[1];
    //enabled features
    memcpy(features.enabled_features, &data[2], 8);
    //data features
    memcpy(features.data_features, &data[10], 8);
}

void set_deadzone(xinput_gamepad_t *host_report, int select_analog, uint8_t p_dead_zone){

    float dead_zone = (float)p_dead_zone / 100;

    int16_t *analog_X;
    int16_t *analog_Y;

    if(select_analog == 1){
        analog_X = &host_report->sThumbLX;
        analog_Y = &host_report->sThumbLY;
    }
    else{
        analog_X = &host_report->sThumbRX;
        analog_Y = &host_report->sThumbRY;
    } 

    double magnitude = sqrt(((*analog_X) * (*analog_X)) + ((*analog_Y) * (*analog_Y)));

    if (magnitude > 32767)
        magnitude = 32767;

    //normalized values
    float normalized_X = *analog_X / magnitude;
    float normalized_Y = *analog_Y / magnitude;

    magnitude = magnitude / 32768;

    if(magnitude < dead_zone){
        *analog_X = 0;
        *analog_Y = 0;
    }
    else{
        float fix = ((magnitude - dead_zone) / (1 - dead_zone));
        normalized_X = normalized_X * fix;
        normalized_Y = normalized_Y * fix;
        *analog_X = normalized_X * 32767;
        *analog_Y = normalized_Y * 32767;
    }

}

void set_swap_dpad(xinput_gamepad_t *host_report){

    uint8_t copy_dpad = host_report->wButtons&0xF;

    //Set to zero dpad data
    host_report->wButtons &= ~0xF; 

    //ANALOG DATA
    if (host_report->sThumbLX > 16383){
        host_report->wButtons |= XINPUT_GAMEPAD_DPAD_RIGHT;
    }
    else if(host_report->sThumbLX < -16383){
        host_report->wButtons |= XINPUT_GAMEPAD_DPAD_LEFT;
    }

    if (host_report->sThumbLY > 16383){
        host_report->wButtons |= XINPUT_GAMEPAD_DPAD_UP;
    }
    else if(host_report->sThumbLY < -16383){
        host_report->wButtons |= XINPUT_GAMEPAD_DPAD_DOWN;
    }

    //DPAD
    host_report->sThumbLX =  (copy_dpad & XINPUT_GAMEPAD_DPAD_RIGHT ? 32767 : 0) | 
            (copy_dpad & XINPUT_GAMEPAD_DPAD_LEFT ? -32768 : 0);

    host_report->sThumbLY =  (copy_dpad & XINPUT_GAMEPAD_DPAD_UP ? 32767 : 0) | 
            (copy_dpad & XINPUT_GAMEPAD_DPAD_DOWN ? -32768 : 0);
}

void set_block_analog(xinput_gamepad_t *host_report, int select_analog){

    switch(select_analog){
        case 1:
            host_report->sThumbLX = 0;
            host_report->sThumbLY = 0;
            break;
        case 2:
            host_report->sThumbRX = 0;
            host_report->sThumbRY = 0;
            break;
        default:
            break;
    }

}

void set_features(xinput_gamepad_t *host_report){

    uint8_t feature;

    for(int i = 0; i < features.num_enabled_features; i++){
        feature = features.enabled_features[i];

        switch(feature){
        case BLOCK_ANALOG:
            if(features.data_features[BLOCK_ANALOG]&0x1){
                set_block_analog(host_report, 1);
            }
            if((features.data_features[BLOCK_ANALOG]>>1)&0x1){
                set_block_analog(host_report, 2);
            }
            break;
        case SWAP_DPAD:
            set_swap_dpad(host_report);
            break;
        case DEAD_ZONE:
            if((features.data_features[DEAD_ZONE - 1] >> 7)){
                set_deadzone(host_report, 1, features.data_features[DEAD_ZONE - 1] & 0x7F);
            }
            if((features.data_features[DEAD_ZONE] >> 7)){
                set_deadzone(host_report, 2, features.data_features[DEAD_ZONE] & 0x7F);
            } 
            break;
        default:
            break;
        }
    }

}
