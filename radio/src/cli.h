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

// CLI task function
void cliStart();

// Input injection commands (key, trim, touch, rotary) are debug-only
#if defined(CLI) && defined(DEBUG) && !defined(BOOT)
#define CLI_INPUT_INJECT
// File access commands (lsl, fread, fwrite, rm, mkdir) are debug-only too
#define CLI_FILE_ACCESS
#endif

#if defined(CLI_INPUT_INJECT)
// Key mask held by the "key" CLI command, 0 when none
uint32_t cliInjectedKeys();
// Trim switch mask held by the "trim" CLI command, 0 when none
uint32_t cliInjectedTrims();
#endif

// Connect serial driver to CLI
void cliSetSerialDriver(void* ctx, const etx_serial_driver_t* drv);
