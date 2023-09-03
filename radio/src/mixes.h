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

struct MixData;

// Get a pointer to a mixer line
MixData* mixAddress(uint8_t idx);

// Insert a new mixer line at 'idx'
void insertMix(uint8_t idx, uint8_t channel);

// Delete mixer line at 'idx'
void deleteMix(uint8_t idx);

// Duplicate mixer line at 'idx' in place
void copyMix(uint8_t idx, uint8_t dst, uint8_t channel);

// Move the mixer line at 'idx' up or down
// by one position and return the new index.
uint8_t moveMix(uint8_t idx, bool up);

uint8_t getMixCount();

// Should only be called from storage
// right after a model has been loaded
void updateMixCount();
