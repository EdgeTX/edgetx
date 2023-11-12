/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#if defined(USBJ_EX)
#include "usb_joystick.h"
#endif

extern "C" {
#include "usbd_conf.h"
#include "usbd_core.h"
#include "usbd_msc.h"
#include "usbd_desc.h"
#include "usbd_hid.h"
#include "usbd_cdc.h"
}

#include "stm32_hal_ll.h"
#include "stm32_hal.h"

#include "hal/usb_driver.h"

#include "hal.h"
#include "debug.h"

static bool usbDriverStarted = false;
#if defined(BOOT)
static usbMode selectedUsbMode = USB_MASS_STORAGE_MODE;
#else
static usbMode selectedUsbMode = USB_UNSELECTED_MODE;
#endif

USBD_HandleTypeDef hUsbDeviceFS;

int getSelectedUsbMode()
{
  return selectedUsbMode;
}

void setSelectedUsbMode(int mode)
{
  selectedUsbMode = usbMode(mode);
}

#if defined(USB_GPIO_PIN_VBUS)
int usbPlugged()
{
  static uint8_t debouncedState = 0;
  static uint8_t lastState = 0;

  uint8_t state = LL_GPIO_IsInputPinSet(USB_GPIO, USB_GPIO_PIN_VBUS);

  if (state == lastState)
    debouncedState = state;
  else
    lastState = state;
  
  return debouncedState;
}
#endif

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

extern "C" void OTG_FS_IRQHandler()
{
  DEBUG_INTERRUPT(INT_OTG_FS);
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

void usbInit()
{
  // Initialize hardware
  __HAL_RCC_GPIOA_CLK_ENABLE();

  LL_GPIO_InitTypeDef GPIO_InitStruct;
  LL_GPIO_StructInit(&GPIO_InitStruct);

#if defined(USB_GPIO_PIN_VBUS)
  GPIO_InitStruct.Pin = USB_GPIO_PIN_VBUS;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(USB_GPIO, &GPIO_InitStruct);
#endif

  GPIO_InitStruct.Pin = USB_GPIO_PIN_DM | USB_GPIO_PIN_DP;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_10; // USB
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  usbDriverStarted = false;
}

extern void usbInitLUNs();
extern USBD_HandleTypeDef hUsbDeviceFS;
extern "C" USBD_StorageTypeDef USBD_Storage_Interface_fops_FS;
extern USBD_CDC_ItfTypeDef USBD_Interface_fops_FS;

void usbStart()
{
  USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
  switch (getSelectedUsbMode()) {
#if !defined(BOOT)
    case USB_JOYSTICK_MODE:
      // initialize USB as HID device
#if defined(USBJ_EX)
      setupUSBJoystick();
#endif
      //USBD_Init(&hUsbDeviceFS, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_HID_cb, &USR_cb);
      //MX_USB_DEVICE_Init();
      USBD_RegisterClass(&hUsbDeviceFS, &USBD_HID);
      break;
#if defined(USB_SERIAL)
    case USB_SERIAL_MODE:
      // initialize USB as CDC device (virtual serial port)
      //USBD_Init(&hUsbDeviceFS, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);
      //MX_USB_DEVICE_Init();
      USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
      USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
      break;
#endif
#endif
    default:
    case USB_MASS_STORAGE_MODE:
      // initialize USB as MSC device
      usbInitLUNs();
      //MX_USB_DEVICE_Init();
      USBD_RegisterClass(&hUsbDeviceFS, &USBD_MSC);
      USBD_MSC_RegisterStorage(&hUsbDeviceFS, &USBD_Storage_Interface_fops_FS);
      //USBD_Init(&hUsbDeviceFS, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_MSC_cb, &USR_cb);
      break;
  }
  USBD_Start(&hUsbDeviceFS);
  usbDriverStarted = true;
}

void usbStop()
{
  usbDriverStarted = false;
  USBD_DeInit(&hUsbDeviceFS);
}


bool usbStarted()
{
  return usbDriverStarted;
}

#if !defined(BOOT)

#if defined(USBJ_EX)
extern "C" void delay_ms(uint32_t count);
void usbJoystickRestart()
{
  if (!usbDriverStarted || getSelectedUsbMode() != USB_JOYSTICK_MODE) return;

  USBD_DeInit(&hUsbDeviceFS);
  delay_ms(100);
  USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
  USBD_RegisterClass(&hUsbDeviceFS, &USBD_HID);
  USBD_Start(&hUsbDeviceFS);
}
#else
// TODO: fix after HAL conversion is complete
#warning channelOutputs should come from "globals.h"

#define MAX_OUTPUT_CHANNELS 32
extern int16_t channelOutputs[MAX_OUTPUT_CHANNELS];
#endif



/*
  Prepare and send new USB data packet

  The format of HID_Buffer is defined by
  USB endpoint description can be found in
  file usb_hid_joystick.c, variable HID_JOYSTICK_ReportDesc
*/
void usbJoystickUpdate()
{
#if !defined(USBJ_EX)
   static uint8_t HID_Buffer[0/*HID_IN_PACKET*/];

   //buttons
   HID_Buffer[0] = 0;
   HID_Buffer[1] = 0;
   HID_Buffer[2] = 0;
   for (int i = 0; i < 8; ++i) {
     if ( channelOutputs[i+8] > 0 ) {
       HID_Buffer[0] |= (1 << i);
     }
     if ( channelOutputs[i+16] > 0 ) {
       HID_Buffer[1] |= (1 << i);
     }
     if ( channelOutputs[i+24] > 0 ) {
       HID_Buffer[2] |= (1 << i);
     }
   }

   //analog values
   //uint8_t * p = HID_Buffer + 1;
   for (int i = 0; i < 8; ++i) {

     int16_t value = channelOutputs[i] + 1024;
     if ( value > 2047 ) value = 2047;
     else if ( value < 0 ) value = 0;
     HID_Buffer[i*2 +3] = static_cast<uint8_t>(value & 0xFF);
     HID_Buffer[i*2 +4] = static_cast<uint8_t>((value >> 8) & 0x07);

   }
   USBD_HID_SendReport(&hUsbDeviceFS, HID_Buffer, HID_IN_PACKET);
#else
  usbReport_t ret = usbReport();
  USBD_HID_SendReport(&hUsbDeviceFS, ret.ptr, ret.size);
#endif
}
#endif
