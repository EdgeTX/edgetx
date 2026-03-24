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

#include <stdint.h>

struct CustomFunctionData;

// Get the number of allocated model special functions
uint8_t getCustomFnCount();

// Get a pointer to a model special function (read-only; returns dummy if out of range)
CustomFunctionData* customFnAddress(uint8_t idx);

// Get a writable pointer, growing the arena section if needed
CustomFunctionData* customFnAllocAt(uint8_t idx);

// Remove trailing empty elements (swtch.isNone()) from the custom function section
void customFnTrimTrailing();

// Insert a blank special function at 'idx', shifting subsequent ones down
void insertCustomFn(uint8_t idx);

// Delete the special function at 'idx', shifting subsequent ones up
void deleteCustomFn(uint8_t idx);

// Clear a single special function at 'idx'
void clearCustomFn(uint8_t idx);
