/*------Pin configuration------*/
#ifdef WAVESHARE_RP2350_USB_A
    #define USB_A_HOST
#endif
//#define CUSTOM_LED_PIN 4
//Principal LED
#ifdef CUSTOM_LED_PIN
    #define LED_PIN CUSTOM_LED_PIN           //User custom PIN
#elif PICO_DEFAULT_LED_PIN
    #define LED_PIN PICO_DEFAULT_LED_PIN	//Default LED PIN
#endif

#ifdef PICO_DEFAULT_WS2812_PIN
    #define USE_WS2812_LED                  //WS2812 LED
#endif

//Pin WEB mode
#define WEB_PIN 18

//USB PIO PIN D+
//D- is consecutive
#ifdef USB_A_HOST
    #define PIO_USB_PIN PICO_DEFAULT_PIO_USB_DP_PIN
#else
    #define PIO_USB_PIN 16
#endif

#ifdef WAVESHARE_RP2350_USB_A
    // KBD data and clock inputs must be consecutive with
    // data in the lower position.
    #define DAT_GPIO 26 // PS/2 data
    #define CLK_GPIO 27 // PS/2 clock
    // Alternative for PS1/PS2 device mode
    #define ALT_DAT_GPIO 28 	// PS/2 data
    #define ALT_CLK_GPIO 29 	// PS/2 clock

    // CMD, CLK, and ATT pins must be consecutive with CMD in the lower position.
    // No it's necessary the DAT pin be consecutive
    #define CMD 26    		// PSX CMD
    #define CLK CMD + 1     // PSX CLK
    #define ATT CMD + 2     // PSX ATT
    #define DAT 29    		// PSX DAT
    // Alternative for PS1/PS2 device mode
    #define ALT_CMD 2    		    // PSX CMD
    #define ALT_CLK ALT_CMD + 3     // PSX CLK
    #define ALT_ATT ALT_CMD + 4     // PSX ATT
    #define ALT_DAT 5    		    // PSX DAT

    // Gamecube config
    #define GC_DAT_GPIO 29			// GC DATA
#else
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
#endif
