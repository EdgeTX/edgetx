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

//
// CRSF trainer input over a serial port.
//
// Unlike the SBUS trainer path, which relies on the USART idle line to mark
// frame boundaries, CRSF is self-framing: every frame carries a length byte
// and a CRC. That makes it usable on ports that deliver arbitrarily chunked
// buffers with no idle-line detection at all, i.e. USB-VCP.
//
// Attach / detach the driver's receive callback:
void crsfTrainerStart(void* ctx, const etx_serial_driver_t* drv);
void crsfTrainerStop();

// Receive callback: feeds a chunk of the byte stream into the frame assembler
void crsfTrainerReceiveData(uint8_t* data, uint32_t len);
