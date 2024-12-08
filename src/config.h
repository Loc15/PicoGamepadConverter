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
// Alternative for PS1/PS2 device mode
#define ALT_DAT_GPIO 5 	// PS/2 data
#define ALT_CLK_GPIO 6 	// PS/2 clock

// CMD, CLK, and ATT pins must be consecutive with CMD in the lower position.
// No it's necessary the DAT pin be consecutive
#define CMD 19    		// PSX CMD
#define CLK CMD + 1     // PSX CLK
#define ATT CMD + 2     // PSX ATT
#define DAT 22    		// PSX DAT
// Alternative for PS1/PS2 device mode
#define ALT_CMD 5    		    // PSX CMD
#define ALT_CLK ALT_CMD + 1     // PSX CLK
#define ALT_ATT ALT_CMD + 2     // PSX ATT
#define ALT_DAT 8    		    // PSX DAT

// Gamecube config
#define GC_DAT_GPIO 19			// GC DATA