#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Generate a info structure based on the HID descriptor report
// Parameters
//     desc_report  - HID descriptor report array from tinyusb
//     desc_len - Size of HID descriptor report array
uint8_t generate_HID_report_info(uint8_t const *desc_report, uint16_t desc_len);

// Free memory taken for a info structure based on the HID descriptor report
// Parameters
void free_HID_report_info();

// Parser Report
// Parameters
//     report  - HID report array from tinyusb
//     len - Size of HID report array
void parse_report(uint8_t const *report, uint16_t len, uint8_t *parsed_report);


#ifdef __cplusplus
}
#endif
