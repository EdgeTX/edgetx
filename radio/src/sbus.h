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

// The SBUS trainer receiver is a single consumer (trainerInput[]): only one
// source can feed it at a time. Which one is decided by the trainer mode, so
// the trainer owns the context and arms/disarms the IDLE callback itself.

// Claim the SBUS trainer receiver for a serial port.
// Returns false if the port cannot be used as SBUS trainer input.
bool sbusTrainerAcquire(void* ctx, const etx_serial_driver_t* drv);

// Release the SBUS trainer receiver (no-op if nothing is claimed)
void sbusTrainerRelease();

// Is a source currently feeding the SBUS trainer receiver?
bool sbusTrainerActive();

// Release the SBUS trainer receiver, but only if 'ctx' is the current owner.
// Used when tearing down a serial port that may or may not be in use.
void sbusTrainerReleaseCtx(void* ctx);
