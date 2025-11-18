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
  //char press_hotkey;
  //char button_hotkey_pos;
  unsigned char set_features; 
  unsigned char num_enabled_features;
  unsigned char enabled_features[8];
  unsigned char data_features[8];
} features_t;

typedef enum 
{
    BLOCK_ANALOG,
    SWAP_DPAD,
    DEAD_ZONE
} FEATURE_TYPE;

void new_report_fun(void *report, MODE mode_host, void *new_report, MODE mode_device);
void set_features_from_flash(unsigned char *data);
