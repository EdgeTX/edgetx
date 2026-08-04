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

#include "hal/serial_driver.h"

#define SBUS_BAUDRATE         100000

// SBUS serial driver + context
void sbusSetReceiveCtx(void* ctx, const etx_serial_driver_t* drv);

// SBUS AUX idle callback
void sbusAuxFrameReceived(void* param);

// Enable / disable serial trainer input (both the AUX UART and the USB-VCP
// paths). Driven by TRAINER_MODE_MASTER_SERIAL.
void sbusAuxSetEnabled(bool enabled);

void sbusFrameReceived(void* param);

//
// SBUS byte-stream framer.
//
// For ports that deliver arbitrarily chunked buffers and have no idle-line
// detection to mark frame boundaries (USB-VCP). Attach / detach the driver's
// receive callback, and keep the partial-frame state across chunks.
//
void sbusStreamStart(void* ctx, const etx_serial_driver_t* drv);
void sbusStreamStop();

// Receive callback: feeds a chunk of the byte stream into the framer
void sbusStreamReceiveData(uint8_t* data, uint32_t len);
