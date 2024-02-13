/*------Pin configuration------*/
//Principal LED
#define LED_PIN 25	//Original Pico LED

//Pin WEB mode
#define WEB_PIN 18

//USB PIO PIN D+
//D- is consecutive
#define PIO_USB_PIN 16

// KBD data and clock inputs must be consecutive with
// data in the lower position.
#define DAT_GPIO 19 // PS/2 data
#define CLK_GPIO 20 // PS/2 clock

// CMD, CLK, and ATT pins must be consecutive with CMD in the lower position.
// No it's necessary the DAT pin be consecutive
#define CMD 19    // PSX CMD
#define DAT 22    // PSX DAT