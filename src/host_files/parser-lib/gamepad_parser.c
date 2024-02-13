/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021, Ha Thach (tinyusb.org)
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

#include <stdio.h>
#include <stddef.h>
#include "hidparser.h"
#include "HID_enums.h"

static const uint8_t HAT_SWITCH_TO_DIRECTION_BUTTONS[] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8};

typedef struct __attribute((packed, aligned(1)))
{
  uint8_t  x;
  uint8_t  y;
  uint8_t  z;
  uint8_t  rz;
  uint8_t hat: 4;
  uint32_t buttons;
} pad_buttons;

HID_ReportInfo_t *info = NULL;

uint8_t generate_HID_report_info(uint8_t const *desc_report, uint16_t desc_len){
  return USB_ProcessHIDReport(desc_report, desc_len, &info);
}

void free_HID_report_info(){
  USB_FreeReportInfo(info);
  info = NULL;
}

//called from parser for filtering report items
bool CALLBACK_HIDParser_FilterHIDReportItem(HID_ReportItem_t *const CurrentItem)
{
  if (CurrentItem->ItemType != HID_REPORT_ITEM_In)
    return false;

  switch (CurrentItem->Attributes.Usage.Page)
  {
    case HID_USAGE_PAGE_DESKTOP:
      switch (CurrentItem->Attributes.Usage.Usage)
      {
        case HID_USAGE_DESKTOP_X:
        case HID_USAGE_DESKTOP_Y:
        case HID_USAGE_DESKTOP_Z:
        case HID_USAGE_DESKTOP_RZ:
        case HID_USAGE_DESKTOP_HAT_SWITCH:
          return true;
      }
      return false;
    case HID_USAGE_PAGE_BUTTON:
      return true;
  }
  return false;
}

static inline bool USB_GetHIDReportItemInfoWithReportId(const uint8_t *ReportData, HID_ReportItem_t *const ReportItem)
{
  if (ReportItem->ReportID)
  {
    if (ReportItem->ReportID != ReportData[0])
      return false;

    ReportData++;
  }
  return USB_GetHIDReportItemInfo(ReportItem->ReportID, ReportData, ReportItem);
}

void parse_report(uint8_t const *report, uint16_t len, uint8_t *parsed_report)
{
  pad_buttons current = {0};
  uint8_t buttons_counter = 0;
  HID_ReportItem_t *item = info->FirstReportItem;
  //iterate filtered reports info to match report from data
  while (item)
  {
    if (USB_GetHIDReportItemInfoWithReportId(report, item))
    {
      switch (item->Attributes.Usage.Page)
      {
      case HID_USAGE_PAGE_DESKTOP:
        switch (item->Attributes.Usage.Usage)
        {
        case HID_USAGE_DESKTOP_X:
          current.x = item->Value;
          break;
        case HID_USAGE_DESKTOP_Y:
          current.y = item->Value;
          break;
        case HID_USAGE_DESKTOP_Z:
          current.z = item->Value;
          break;
        case HID_USAGE_DESKTOP_RZ:
          current.rz = item->Value;
          break;
        case HID_USAGE_DESKTOP_HAT_SWITCH:
          current.hat |= HAT_SWITCH_TO_DIRECTION_BUTTONS[item->Value];
          break;
        }
        break;
      case HID_USAGE_PAGE_BUTTON:
      {
        if(item->Value){
          current.buttons |= (1 << buttons_counter); 
        }
        buttons_counter++;
      }
      break;
      }
    }
    item = item->Next;
  }

  /*JOYSTICKS DATA START POSITION*/
  parsed_report[0] = 1;
  /*JOYSTICKS DATA*/
  parsed_report[1] = current.x;
  parsed_report[2] = current.y;
  parsed_report[3] = current.z;
  parsed_report[4] = current.rz;
  /*BUTTONS+DPAD*/
  parsed_report[5] = ((current.buttons & 0xf)<< 4) | current.hat;
  /*THUMBS+START+SELECT+SHOULDERS*/
  parsed_report[6] = ((current.buttons >> 4) & 0xff);

}
