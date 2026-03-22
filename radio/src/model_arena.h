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
#include <string.h>

// Arena size per platform (can be overridden per radio in board defs)
#if !defined(MODEL_ARENA_SIZE)
  #if defined(SIMU) || defined(COMPANION)
    #define MODEL_ARENA_SIZE  65536
  #elif defined(STM32H7) || defined(STM32H7RS) || defined(STM32H5)
    #define MODEL_ARENA_SIZE  8192
  #else
    #define MODEL_ARENA_SIZE  4096
  #endif
#endif

// Hard safety caps for parallel state arrays
// These bound the maximum possible elements, regardless of arena size
#define MAX_MIXERS_HARD      128
#define MAX_EXPOS_HARD       128
#define MAX_CURVES_HARD       64
#define MAX_CURVE_POINTS_HARD 1024
#define MAX_LOGICAL_SWITCHES_HARD 64
#define MAX_SPECIAL_FUNCTIONS_HARD 64

// Number of dynamic section types in the arena
#define ARENA_NUM_SECTIONS 6

enum ArenaSectionType {
  ARENA_MIXES = 0,
  ARENA_EXPOS,
  ARENA_CURVES,
  ARENA_POINTS,
  ARENA_LOGICAL_SW,
  ARENA_CUSTOM_FN,
};

// Metadata stored in ModelData describing the arena layout
struct ModelDynData {
  uint8_t   mixCount;
  uint8_t   expoCount;
  uint8_t   curveCount;
  uint16_t  pointsCount;
  uint8_t   logicalSwCount;
  uint8_t   customFnCount;
};

class ModelArena {
  uint8_t* _base;
  uint16_t _capacity;

  // Section byte offsets within the arena (computed from counts + element sizes)
  uint16_t _offsets[ARENA_NUM_SECTIONS];
  uint16_t _usedBytes;

public:
  ModelArena() : _base(nullptr), _capacity(0), _usedBytes(0) {
    memset(_offsets, 0, sizeof(_offsets));
  }

  void attach(uint8_t* buf, uint16_t capacity);

  uint8_t* base() const { return _base; }
  uint16_t capacity() const { return _capacity; }
  uint16_t usedBytes() const { return _usedBytes; }
  uint16_t freeBytes() const { return _capacity - _usedBytes; }

  // Compute layout from counts (called after model load or on new model)
  void layout(const ModelDynData& dyn);

  // Get section base pointer and current count
  uint8_t* sectionBase(ArenaSectionType type) const {
    return _base + _offsets[type];
  }
  uint16_t sectionOffset(ArenaSectionType type) const {
    return _offsets[type];
  }

  // Insert a slot of 'slotSize' bytes at 'byteOffset' within the arena.
  // Shifts all data after that point forward.
  // Returns false if arena is full.
  bool insertSlot(uint16_t byteOffset, uint16_t slotSize);

  // Delete a slot of 'slotSize' bytes at 'byteOffset' within the arena.
  // Shifts all data after that point backward.
  void deleteSlot(uint16_t byteOffset, uint16_t slotSize);

  // Insert an element into a specific section
  // Shifts all subsequent sections forward
  bool insertInSection(ArenaSectionType section, uint16_t indexInSection,
                       uint16_t elementSize);

  // Delete an element from a specific section
  // Shifts all subsequent sections backward
  void deleteFromSection(ArenaSectionType section, uint16_t indexInSection,
                         uint16_t elementSize);

  // Recalculate offsets from counts (after direct count modification)
  void recalcOffsets(const ModelDynData& dyn);

  // Clear the arena
  void clear();
};

extern ModelArena g_modelArena;
