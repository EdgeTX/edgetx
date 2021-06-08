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

#ifndef OPENTX_USB_DRIVER_H
#define OPENTX_USB_DRIVER_H

#include <stdbool.h>

// USB driver
enum usbMode {
  USB_UNSELECTED_MODE,
  USB_JOYSTICK_MODE,
  USB_MASS_STORAGE_MODE,
  USB_SERIAL_MODE,
#if defined(USB_SERIAL)
  USB_MAX_MODE=USB_SERIAL_MODE
#else
  USB_MAX_MODE=USB_MASS_STORAGE_MODE
#endif
};

int usbPlugged();
void usbInit();
void usbStart();
void usbStop();
bool usbStarted();
int getSelectedUsbMode();
void setSelectedUsbMode(int mode);

void usbSerialPutc(uint8_t c);

// Used in view_statistics.cpp
#if defined(DEBUG) && !defined(BOOT)
  extern uint16_t usbWraps;
  extern uint16_t charsWritten;
  extern volatile uint32_t APP_Rx_ptr_in;
  extern volatile uint32_t APP_Rx_ptr_out;
#endif

#endif
