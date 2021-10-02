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

// TODO: enable masstorage & implement choice between masstorage and joystick

extern "C" {
#include "usb/device/hid-joystick/HIDDJoystickDriver.h"
#include "usb/device/massstorage/MSDDriver.h"
}

extern "C" void USBDDriverCallbacks_ConfigurationChanged(unsigned char cfgnum)
{
//  if (selectedUsbMode == USB_JOYSTICK_MODE) {
    (void)cfgnum;
//  } else if (selectedUsbMode == USB_MASS_STORAGE_MODE){
//    MSDDriver_Reset();
//  }
}

extern "C" void USBDCallbacks_RequestReceived(const USBGenericRequest *request)
{
//  if (selectedUsbMode == USB_JOYSTICK_MODE) {
    HIDDJoystickDriver_RequestHandler(request);
//  } else if (selectedUsbMode == USB_MASS_STORAGE_MODE){
//    MSDDriver_RequestHandler(request);
//  }
}
