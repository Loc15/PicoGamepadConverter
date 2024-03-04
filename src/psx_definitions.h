//PS1/PS2 CONTROLLER defines and struct format from
//https://gamesx.com/controldata/psxcont/psxcont.htm
#define DATA_SHIFT(a) (a >> 24)
#define PSX_GAMEPAD_SELECT 0x01
#define PSX_GAMEPAD_R3 0x02
#define PSX_GAMEPAD_L3 0x04
#define PSX_GAMEPAD_START 0x08
#define PSX_GAMEPAD_DPAD_UP 0x10
#define PSX_GAMEPAD_DPAD_RIGHT 0x20
#define PSX_GAMEPAD_DPAD_DOWN 0x40
#define PSX_GAMEPAD_DPAD_LEFT 0x80

#define PSX_GAMEPAD_L2 0x01
#define PSX_GAMEPAD_R2 0x02
#define PSX_GAMEPAD_L1 0x04
#define PSX_GAMEPAD_R1 0x08
#define PSX_GAMEPAD_TRIANGLE 0x10
#define PSX_GAMEPAD_CIRCLE 0x20
#define PSX_GAMEPAD_CROSS 0x40
#define PSX_GAMEPAD_SQUARE 0x80


//PS1/PS2 DEVICE MODE struct format
#define PSX_DEVICE_LEFT   7
#define PSX_DEVICE_DOWN   6
#define PSX_DEVICE_RIGHT  5
#define PSX_DEVICE_UP     4
#define PSX_DEVICE_START  3
#define PSX_DEVICE_R3     2
#define PSX_DEVICE_L3     1
#define PSX_DEVICE_SELECT 0

#define PSX_DEVICE_SQUARE   7
#define PSX_DEVICE_CROSS    6
#define PSX_DEVICE_CIRCLE   5
#define PSX_DEVICE_TRIANGLE 4
#define PSX_DEVICE_R1       3
#define PSX_DEVICE_L1       2
#define PSX_DEVICE_R2       1
#define PSX_DEVICE_L2       0





