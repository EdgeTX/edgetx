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

#include <stddef.h>
#include <stdint.h>

void analogSetCustomStickName(uint8_t idx, const char* str, size_t len);
const char* analogGetCustomStickName(uint8_t idx);
bool analogHasCustomStickName(uint8_t idx);

void analogSetCustomPotName(uint8_t idx, const char* str, size_t len);
const char* analogGetCustomPotName(uint8_t idx);
bool analogHasCustomPotName(uint8_t idx);

int analogLookupPotIdx(const char* name, size_t len);
const char* analogGetCanonicalPotName(uint8_t idx);
