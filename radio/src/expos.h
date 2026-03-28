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

struct ExpoData;

// Get a pointer to an expo/input line (read-only; returns dummy if out of range)
ExpoData* expoAddress(uint8_t idx);

// Get a writable pointer, growing the arena section if needed
ExpoData* expoAllocAt(uint8_t idx);

// Insert a new expo/input line at 'idx' for 'input'
void insertExpo(uint8_t idx, uint8_t input);

// Delete expo/input line at 'idx'
void deleteExpo(uint8_t idx);

// Duplicate expo/input line from 'source' to 'dest' position
void copyExpo(uint8_t source, uint8_t dest, uint8_t input);

// Move the expo/input line at 'idx' up or down
// by one position and return the new index.
uint8_t moveExpo(uint8_t idx, bool up);

uint8_t getExpoCount();

// Should only be called from storage
// right after a model has been loaded
void updateExpoCount();

// --- Input name accessors (arena-backed, sparse) ---

// Read-only lookup. Returns pointer to name or nullptr if input has no name.
const char* inputName(uint8_t input);

// Returns true if the input has a name allocated.
bool hasInputName(uint8_t input);

// Ensure a name slot exists for 'input' and return a writable pointer.
// Allocates a new arena slot if needed. Returns nullptr on arena exhaustion.
char* inputNameAlloc(uint8_t input);

// Remove the name for 'input', freeing the arena slot (swap-with-last).
void inputNameClear(uint8_t input);

// Clear the entire inputNameIndex (set all to 0xFF).
// Called on model init / clear before loading names.
void inputNameIndexReset();

