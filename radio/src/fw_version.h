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
#include "stamp.h"

#if defined(COLORLCD)
extern const char fw_stamp[];
extern const char vers_stamp[];
extern const char date_stamp[];
extern const char time_stamp[];
#else
extern const char vers_stamp[];
#endif

#if defined(STM32)
/**
 * Tries to find opentx version in the first 1024 byte of either firmware/bootloader
 * (the one not running) or the buffer
 * @param buffer If non-null find the firmware version in the buffer instead
 */
const char * getFirmwareVersion(const char * buffer = nullptr);
#endif
