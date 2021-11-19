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

#pragma once

#include <stdbool.h>
#include <stdint.h>

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

// Control line state bits
#define CTRL_LINE_STATE_DTR (1 << 0)
#define CTRL_LINE_STATE_RTS (1 << 1)

int  usbPlugged();
void usbInit();
void usbStart();
void usbStop();
bool usbStarted();

int  getSelectedUsbMode();
void setSelectedUsbMode(int mode);

uint32_t usbSerialFreeSpace();
void     usbSerialPutc(uint8_t c);

uint32_t usbSerialBaudRate(void);

void usbSerialSetReceiveDataCb(void (*cb)(uint8_t* buf, uint32_t len));
void usbSerialSetCtrlLineStateCb(void (*cb)(uint16_t ctrlLineState));
void usbSerialSetBaudRateCb(void (*cb)(uint32_t baud));
