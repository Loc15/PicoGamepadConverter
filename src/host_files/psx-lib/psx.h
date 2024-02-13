#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Initialize PSX controller support
// Parameters
//     pio  - support pio number. 0 or 1
//     gpio_output - GPIO number of CMD pin, CLK and ATT pin must be on next
//            adjacent GPIO
//	   gpio_input - GPIO number of DATA pin, no is necessary that it be next to others
//			  pins
//     fn - Callback function to pass the data when it is received
void psx_init(uint pio, uint gpio_output, uint gpio_input, void (*fn)(uint32_t *));

#ifdef __cplusplus
}
#endif
