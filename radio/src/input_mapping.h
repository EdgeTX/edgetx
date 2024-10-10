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


#define MAX_INPUT_MODES 4

// Is mapping of main controls used? (aka. "Modes")
bool inputMappingModesUsed();

// Return the throttle input regardless of radio type
uint8_t inputMappingGetThrottle();

// Map first 4 channels from/to Mode 1
// according to radio settings
uint8_t inputMappingConvertMode(uint8_t ch);

// Map first 4 channels from/to Mode 1
// according to mode
uint8_t inputMappingConvertMode(uint8_t mode, uint8_t ch);

// Return channels in order (ex: RETA, AETR, etc.)
// according to radio settings
uint8_t inputMappingChannelOrder(uint8_t ch);

// Return channels in order (ex: RETA, AETR, etc.)
// according to the order index passed (0..23)
uint8_t inputMappingChannelOrder(uint8_t order, uint8_t ch);

// Return channel order from an index (0..23)
uint8_t inputMappingGetChannelOrder(uint8_t order);

// Return the maximum number of order permutations
uint8_t inputMappingGetMaxChannelOrder();
