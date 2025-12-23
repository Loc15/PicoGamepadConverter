typedef enum {
    XINPUT,
    SWITCH, 
    DINPUT,
    KBD_PS2,
    PSX,
    BLUETOOTH,
    WII,
    GC,
    N64,
    WEB
} MODE;

typedef enum
{   
    GENERIC,
    LOGITECH,
    PS4,
    EIGHT_BITDO=5,
    PS3
} HID_TYPE;

extern MODE DEVICE;

typedef struct 
{
  unsigned short hotkey_1;
  unsigned short hotkey_2;
  unsigned char set_features; 
  unsigned char num_enabled_features;
  unsigned char enabled_features[8];
  unsigned char data_features[8];
} features_t;

typedef enum 
{
    BLOCK_ANALOG,
    SWAP_DPAD,
    DEAD_ZONE,
    SWITCH_MODE
} FEATURE_TYPE;

//______________________ ____________ ____________ _________________ _______________ ______________
// Bluetooth host data  |Set Features|SizeFeatures| Features        |Data Features  | WII ADDR INFO|
//  Byte [2-8]          | Byte 9     | Byte 10    | Byte [11-18]    | Byte [19-26]  | Byte [27-33] |
//_ _ _ _ _ _ _ _ _ _ _ |_ _ _ _ _ _ |_ _ _ _ _ _ |_ _ _ _ _ _ _ _ _| _ _ _ _ _ _ _ | _ _ _ _ _ _ _|

typedef enum
{
    HOST_MODE_OFFSET,
    DEVICE_MODE_OFFSET,
    BLUETOOTH_HOST_ADDR_OFFSET = 2,
    BLUETOOTH_HOST_HID_SET_OFFSET = 8,
    SET_FEATURE_OFFSET = 9,
    SIZE_FEATURES_OFFSET = 10,
    FEATURES_OFFSET = 11,
    DATA_FEATURES_OFFSET = 19,
    WII_ADDR_SET_OFFSET = 27,
    WII_ADDR_OFFSET = 28,
    SWITCH_MODE_BUTTONS_OFFSET = 34,
} DATA_OFFSET;

typedef enum
{
    HOST_MODE_SIZE = 1,
    DEVICE_MODE_SIZE = 1,
    BLUETOOTH_HOST_ADDR_SIZE = 6,
    BLUETOOTH_HOST_HID_SET_SIZE = 1,
    SET_FEATURE_SIZE = 1,
    SIZE_FEATURES_SIZE = 1,
    FEATURES_SIZE = 8,
    DATA_FEATURES_SIZE = 8,
    WII_ADDR_SET_SIZE = 1,
    WII_ADDR_SIZE = 6,
    SWITCH_MODE_BUTTONS_SIZE = 2,
} DATA_BYTE_SIZE;

#define TOTAL_SIZE_IN_FLASH (HOST_MODE_SIZE + DEVICE_MODE_SIZE + BLUETOOTH_HOST_ADDR_SIZE + BLUETOOTH_HOST_HID_SET_SIZE \
                        + SET_FEATURE_SIZE + SIZE_FEATURES_SIZE + FEATURES_SIZE + DATA_FEATURES_SIZE + WII_ADDR_SET_SIZE \
                        + WII_ADDR_SIZE + SWITCH_MODE_BUTTONS_SIZE)

void new_report_fun(void *report, MODE mode_host, void *new_report, MODE mode_device);
void set_features_from_flash(unsigned char *data);
