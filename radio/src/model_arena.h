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

// ---------------------------------------------------------------------------
// Arena sizing per platform
//
// SDRAM targets (F429, H7, H7RS, H5): static max-sized buffer in fast
// internal RAM.  Growth just increases _capacity within the pre-reserved
// buffer — no allocation, no copy, no pointer invalidation.
//
// Non-SDRAM F4 + SIMU/Companion: heap-allocated via malloc().  Start at
// INITIAL size, grow via allocate-copy-free.  Heap is in the same fast RAM
// on these targets, so no speed concern.
// ---------------------------------------------------------------------------

#if defined(SDRAM)
  #define ARENA_HEAP_GROWABLE  0
#else
  #define ARENA_HEAP_GROWABLE  1
#endif

#if !defined(MODEL_ARENA_INITIAL_SIZE)
  #define MODEL_ARENA_INITIAL_SIZE  1024
#endif

#if !defined(MODEL_ARENA_MAX_SIZE)
  #if defined(SIMU)
    #define MODEL_ARENA_MAX_SIZE  65536
  #elif defined(STM32H7) || defined(STM32H7RS) || defined(STM32H5)
    #define MODEL_ARENA_MAX_SIZE  32768
  #elif defined(SDRAM)
    // F429 with SDRAM — static buffer in internal RAM
    #define MODEL_ARENA_MAX_SIZE  8192
  #else
    // F4 without SDRAM — heap growth in shared RAM
    #define MODEL_ARENA_MAX_SIZE  6144
  #endif
#endif

// Backward compat alias used by rtc_backup
#define MODEL_ARENA_SIZE  MODEL_ARENA_INITIAL_SIZE

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
  uint32_t _capacity;
  uint32_t _maxCapacity;
  bool     _heapOwned;

  // Section byte offsets within the arena (computed from counts + element sizes)
  uint32_t _offsets[ARENA_NUM_SECTIONS];
  uint16_t _counts[ARENA_NUM_SECTIONS];
  uint32_t _usedBytes;

  // Attempt to grow the buffer to at least minCapacity bytes.
  // Static mode: just increases _capacity (buffer is already large enough).
  // Heap mode: allocate-copy-free.
  // Returns false if minCapacity > _maxCapacity or malloc fails.
  bool grow(uint32_t minCapacity);

public:
  ModelArena() : _base(nullptr), _capacity(0),
                 _maxCapacity(0), _heapOwned(false), _usedBytes(0) {
    memset(_offsets, 0, sizeof(_offsets));
    memset(_counts, 0, sizeof(_counts));
  }

  void attach(uint8_t* buf, uint32_t capacity, uint32_t maxCapacity);

  uint8_t* base() const { return _base; }
  uint32_t capacity() const { return _capacity; }
  uint32_t maxCapacity() const { return _maxCapacity; }
  uint32_t usedBytes() const { return _usedBytes; }

  // Total growable headroom (what GUI capacity checks should use)
  uint32_t freeBytes() const { return _maxCapacity - _usedBytes; }

  // Headroom in current buffer without growing
  uint32_t currentFreeBytes() const { return _capacity - _usedBytes; }

  bool isHeapOwned() const { return _heapOwned; }
  void setHeapOwned(bool owned) { _heapOwned = owned; }

  // Free heap buffer if owned, reset to nullptr.
  void release();

  // Return heap pointer (or nullptr) and set _heapOwned = false.
  // Used for temp model save/restore to avoid double-free.
  uint8_t* detachHeapBuffer();

  // Shrink buffer to usedBytes + headroom (heap mode only).
  // Returns false on alloc failure (non-fatal, keeps current buffer).
  bool shrinkToFit(uint32_t headroom = 256);

  // Compute layout from counts (called after model load or on new model)
  void layout(const ModelDynData& dyn);

  // Get section base pointer and current count
  uint8_t* sectionBase(ArenaSectionType type) const {
    return _base + _offsets[type];
  }
  uint32_t sectionOffset(ArenaSectionType type) const {
    return _offsets[type];
  }

  // Return the number of elements currently allocated in a section
  uint16_t sectionCount(ArenaSectionType section) const {
    return _counts[section];
  }

  // Insert a slot of 'slotSize' bytes at 'byteOffset' within the arena.
  // Shifts all data after that point forward.  Attempts to grow if needed.
  // Returns false if arena cannot accommodate the slot.
  bool insertSlot(uint32_t byteOffset, uint32_t slotSize);

  // Delete a slot of 'slotSize' bytes at 'byteOffset' within the arena.
  // Shifts all data after that point backward.
  void deleteSlot(uint32_t byteOffset, uint32_t slotSize);

  // Insert an element into a specific section
  // Shifts all subsequent sections forward
  bool insertInSection(ArenaSectionType section, uint32_t indexInSection,
                       uint32_t elementSize);

  // Delete an element from a specific section
  // Shifts all subsequent sections backward
  void deleteFromSection(ArenaSectionType section, uint32_t indexInSection,
                         uint32_t elementSize);

  // Recalculate offsets from counts (after direct count modification)
  void recalcOffsets(const ModelDynData& dyn);

  // Grow a section to hold at least minCount elements.
  // Attempts to grow the buffer if needed.
  // Returns false if arena cannot accommodate the request.
  bool ensureSectionCapacity(ArenaSectionType section, uint16_t minCount);

  // Remove trailing empty elements from a section.
  // isEmpty(ptr) returns true if the element at ptr is considered empty.
  // Returns the number of elements removed.
  uint16_t trimTrailingEmpty(ArenaSectionType section,
                             bool (*isEmpty)(const uint8_t*));

  // Clear the arena data (preserves layout)
  void clear();

  // Return the element size for a given section type
  static uint32_t elementSize(ArenaSectionType type);
};

extern ModelArena g_modelArena;

// Must be called early in startup before any model data access
void modelArenaInit();
