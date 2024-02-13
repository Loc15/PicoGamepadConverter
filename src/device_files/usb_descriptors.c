/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "XInputDescriptors.h"
#include "SwitchDescriptors.h"
#include "PS3_Descriptors.h"
#include "webserver_descriptors.h"
#include "tusb.h"

//Utils
#include "convert_data.h"

/* A combination of interfaces must have a unique product id, since PC will save
 * device driver after the first plug. Same VID/PID with different interface e.g
 * MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n) ((CFG_TUD_##itf) << (n))

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb(void) {
  switch(DEVICE){
  case XINPUT:
    return xinput_device_descriptor;
  case WEB:
    return ((uint8_t const *) &webserver_device_descriptor);
  case SWITCH:
    return switch_device_descriptor;
  case DINPUT:
    return hid_device_descriptor;
  default:
    return 0;
  }
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb(uint8_t itf) { 
  switch(DEVICE){
  case SWITCH:
    return switch_report_descriptor;
  case DINPUT:
    return hid_report_descriptor; 
  default:
    return 0;
  }
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+



// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
  switch(DEVICE){
  case XINPUT:
    return xinput_configuration_descriptor;
  case WEB:
    return rndis_configuration;
  case SWITCH:
    return switch_configuration_descriptor;
  case DINPUT:
    return hid_configuration_descriptor;
  default:
    return 0;
  }
}


//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long
// enough for transfer to complete
/*uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  
      (void)langid;

        uint8_t chr_count;

        const uint8_t *p_string_descriptors;

        switch(XINPUT){
          case XINPUT:
            p_string_descriptors = xinput_string_descriptors;
            break;
          default:
            p_string_descriptors = switch_string_descriptors;
            break;
        }

        if (index == 0) {
          memcpy(&_desc_str[1], p_string_descriptors[0], 2);
          chr_count = 1;
        } else {
          // Convert ASCII string into UTF-16

          if (!(index < sizeof(p_string_descriptors) / sizeof(p_string_descriptors[0])))
            return NULL;

          const char *str = p_string_descriptors[index];

          // Cap at max char
          chr_count = strlen(str);
          if (chr_count > 31)
            chr_count = 31;

          for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
          }
        }

        // first byte is length (including header), second byte is string type
        _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

        return _desc_str;
  
 
}*/

/*
uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
  
      (void)langid;

        uint8_t chr_count;

        if (index == 0) {
          memcpy(&_desc_str[1], xinput_string_descriptors[0], 2);
          chr_count = 1;
        } else {
          // Convert ASCII string into UTF-16

          if (!(index < sizeof(xinput_string_descriptors) / sizeof(xinput_string_descriptors[0])))
            return NULL;

          const char *str = xinput_string_descriptors[index];

          // Cap at max char
          chr_count = strlen(str);
          if (chr_count > 31)
            chr_count = 31;

          for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
          }
        }

        // first byte is length (including header), second byte is string type
        _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);

        return _desc_str;
  
 
}*/
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;

  unsigned int chr_count = 0;

  const uint8_t *p_string_descriptor[4];

  switch(DEVICE){
  case XINPUT:
    memcpy(p_string_descriptor, xinput_string_descriptors, (sizeof(const uint8_t *) * 4));
    break;
  case WEB:
    memcpy(p_string_descriptor, webserver_string_descriptors, (sizeof(const uint8_t *) * 4));
    break;
  case SWITCH:
    memcpy(p_string_descriptor, switch_string_descriptors, (sizeof(const uint8_t *) * 4));
    break;
  case DINPUT:
    memcpy(p_string_descriptor, hid_string_descriptors, (sizeof(const uint8_t *) * 4));
    break;
  default:
    break;
  }
  

  if (STRID_LANGID == index)
  {
    memcpy(&_desc_str[1], p_string_descriptor[STRID_LANGID], 2);
    chr_count = 1;
  }
  else if (STRID_MAC == index)
  {
    // Convert MAC address into UTF-16

    for (unsigned i=0; i<sizeof(tud_network_mac_address); i++)
    {
      _desc_str[1+chr_count++] = "0123456789ABCDEF"[(tud_network_mac_address[i] >> 4) & 0xf];
      _desc_str[1+chr_count++] = "0123456789ABCDEF"[(tud_network_mac_address[i] >> 0) & 0xf];
    }
  }
  else
  {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if ( !(index < sizeof(p_string_descriptor)/sizeof(p_string_descriptor[0])) ) return NULL;

    const uint8_t* str = p_string_descriptor[index];

    // Cap at max char
    chr_count = (uint8_t) strlen(str);
    if ( chr_count > (TU_ARRAY_SIZE(_desc_str) - 1)) chr_count = TU_ARRAY_SIZE(_desc_str) - 1;

    // Convert ASCII string into UTF-16
    for (unsigned int i=0; i<chr_count; i++)
    {
      _desc_str[1+i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (uint16_t) ((TUSB_DESC_STRING << 8 ) | (2*chr_count + 2));

  return _desc_str;
}

/*REVISAR ESTA ULTIMA FUNCION*/