#ifndef MAIN_GAMECUBE_CONTROLLER_H
#define MAIN_GAMECUBE_CONTROLLER_H

#include <stdint.h>

void gamecube_controller_init(uint8_t pio, uint8_t joybus_pin, uint8_t *response);

void gamecube_controller_update_state();

#define GC_JOYSTICK_MID 0x80

#define GC_MASK_A (0x1)
#define GC_MASK_B (0x1 << 1)
#define GC_MASK_X (0x1 << 2)
#define GC_MASK_Y (0x1 << 3)
#define GC_MASK_START (0x1 << 4)

#define GC_MASK_DPAD 0xF
#define GC_MASK_Z (0x1 << 4)
#define GC_MASK_R (0x1 << 5)
#define GC_MASK_L (0x1 << 6)

#define GC_MASK_DPAD_UP 0x8
#define GC_MASK_DPAD_UPRIGHT 0xA
#define GC_MASK_DPAD_RIGHT 0x2
#define GC_MASK_DPAD_DOWNRIGHT 0x6
#define GC_MASK_DPAD_DOWN 0x4
#define GC_MASK_DPAD_DOWNLEFT 0x5
#define GC_MASK_DPAD_LEFT 0x1
#define GC_MASK_DPAD_UPLEFT 0x9

#endif // MAIN_GAMECUBE_CONTROLLER_H
