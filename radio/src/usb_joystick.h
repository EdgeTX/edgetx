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

#ifndef _USB_JOYSTICK_H_
#define _USB_JOYSTICK_H_

#define USBJ_BUTTON_SIZE 32

#ifdef __cplusplus
extern "C" {
#endif
  struct usbReport_t {
    uint8_t * ptr;
    uint8_t size;
  };
#ifdef __cplusplus
}
#endif

int usbJoystickExtMode();
int usbJoystickActive();
int usbJoystickSettingsChanged();

// Initializes the data based on the model settings
// Return value (true = description changed, false = desc. update not needed)
int setupUSBJoystick();

void onUSBJoystickModelChanged();

#ifdef __cplusplus
extern "C" {
#endif
  struct usbReport_t usbReportDesc();
  uint8_t usbReportSize();
#ifdef __cplusplus
}
#endif

struct usbReport_t usbReport();

int isUSBAxisCollision(uint8_t chIdx);
int isUSBSimCollision(uint8_t chIdx);
int isUSBBtnNumCollision(uint8_t chIdx);

#endif // _USB_JOYSTICK_H_
