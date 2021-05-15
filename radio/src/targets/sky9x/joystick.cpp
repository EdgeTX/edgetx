/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "edgetx.h"

extern "C" {
#include "usb/device/hid-joystick/HIDDJoystickDriver.h"
extern void HIDDJoystickDriver_Initialize();
unsigned char HIDDJoystickDriver_ChangeJoystickState(const HIDDJoystickInputReport *report);
extern void USBD_Connect(void);
}

static void ConfigureUsbClock(void)
{
  /* Enable PLLB for USB */
  PMC->CKGR_PLLBR = CKGR_PLLBR_DIVB(1)
                  | CKGR_PLLBR_MULB(7)
                  | CKGR_PLLBR_PLLBCOUNT_Msk;
  while((PMC->PMC_SR & PMC_SR_LOCKB) == 0); // TODO  && (timeout++ < CLOCK_TIMEOUT));
  /* USB Clock uses PLLB */
  PMC->PMC_USB = PMC_USB_USBDIV(1)    /* /2   */
               | PMC_USB_USBS;        /* PLLB */
}

void usbJoystickUpdate()
{
  static bool initialized = false;

  if (usbPlugged()) {
    TRACE_DEBUG("usbJoystick\n\r");

    if (!initialized) {
      ConfigureUsbClock();

      HIDDJoystickDriver_Initialize();

      // VBus_Configure();
      USBD_Connect();

      initialized = true;
    }

    static uint8_t HID_Buffer[11];

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
      int16_t value = channelOutputs[i] / 8;
      if ( value > 127 ) value = 127;
      else if ( value < -127 ) value = -127;
      HID_Buffer[i+3] = static_cast<int8_t>(value);
    }

    HIDDJoystickDriver_ChangeJoystickState((HIDDJoystickInputReport*)&HID_Buffer);
  }
}
