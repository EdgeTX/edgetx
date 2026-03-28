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
// Non-SDRAM F4 + SIMU: heap-allocated via malloc().  Start at INITIAL size,
// grow via allocate-copy-free.  Heap is in the same fast RAM on these targets.
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

// Radio arena sizing — only holds custom functions for now
#define RADIO_ARENA_INITIAL_SIZE  256
#define RADIO_ARENA_MAX_SIZE      1024

// ---------------------------------------------------------------------------
// Arena section layout
// ---------------------------------------------------------------------------
//
// Each arena section stores a variable-length array of fixed-size elements.
// The table below documents the runtime state cost per element for each
// section — i.e. the RAM outside the arena that scales with the hard limit.
// This is the cost of raising a hard limit even if a model uses fewer items.
//
// Section              Elem bytes  Runtime state per element (outside arena)
// -------              ----------  -----------------------------------------
// ARENA_MIXES          ~35         4 (act[]) + 6 (mixState[]) + 1 (activeMixes[]) = 11 B
// ARENA_EXPOS          ~22         (none — expos share the input pipeline)
// ARENA_CURVES         4           4-8 (curveEnd[] pointer)
// ARENA_POINTS         1           (none)
// ARENA_LOGICAL_SW     ~20         4 (LogicalSwitchContext) × MAX_FLIGHT_MODES = 36 B
// ARENA_CUSTOM_FN      16          4 (lastFunctionTime[]) per context × 2 contexts = 8 B
// ARENA_FLIGHT_MODES   ~28         2 (fp_act[]) + MAX_LOGICAL_SWITCHES×4 (lswFm[]) ≈ 258 B
// ARENA_GVAR_DATA      7           (none)
// ARENA_GVAR_VALUES    2           (none)
// ARENA_INPUT_NAMES    3-4         MAX_INPUTS (inputNameIndex[])
//
// Largest runtime cost: MAX_FLIGHT_MODES × lswFm = FM × LS × 4 bytes.
// With 9 FM × 64 LS × 4 = 2304 bytes.  Raising FM to 16 → 4096 bytes.
//
// MAX_OUTPUT_CHANNELS has no arena section but carries runtime cost:
// chans[32×4] + channelOutputs[32×2] + ex_chans[32×2] + safetyCh[32×2]
// + sum_chans512[32×4] = 448 bytes (all at hard max 32).

// Maximum number of sections any arena can have
#define ARENA_MAX_SECTIONS 10

// Model arena section indices
enum {
  ARENA_MIXES = 0,
  ARENA_EXPOS,
  ARENA_CURVES,
  ARENA_POINTS,
  ARENA_LOGICAL_SW,
  ARENA_CUSTOM_FN,
  ARENA_FLIGHT_MODES,
  ARENA_GVAR_DATA,
  ARENA_GVAR_VALUES,
  ARENA_INPUT_NAMES,
  MODEL_ARENA_NUM_SECTIONS
};

// Radio arena section indices
enum {
  RADIO_ARENA_CUSTOM_FN = 0,
  RADIO_ARENA_NUM_SECTIONS
};

// Descriptor: defines the number of sections and their element sizes.
// One const instance per arena type (model, radio).  Stored in flash.
struct ArenaDesc {
  uint8_t numSections;
  const uint8_t* elemSizes;
};

extern const ArenaDesc modelArenaDesc;
extern const ArenaDesc radioArenaDesc;

// ---------------------------------------------------------------------------
// Arena class
// ---------------------------------------------------------------------------

class Arena {
  const ArenaDesc* _desc;
  uint8_t* _base;
  uint32_t _capacity;
  uint32_t _maxCapacity;
  bool     _heapOwned;

  uint32_t _offsets[ARENA_MAX_SECTIONS];
  uint16_t _counts[ARENA_MAX_SECTIONS];
  uint32_t _usedBytes;

  bool grow(uint32_t minCapacity);

public:
  Arena() : _desc(nullptr), _base(nullptr), _capacity(0),
            _maxCapacity(0), _heapOwned(false), _usedBytes(0) {
    memset(_offsets, 0, sizeof(_offsets));
    memset(_counts, 0, sizeof(_counts));
  }

  void attach(const ArenaDesc* desc, uint8_t* buf,
              uint32_t capacity, uint32_t maxCapacity);

  const ArenaDesc* desc() const { return _desc; }
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

  void release();
  uint8_t* detachHeapBuffer();
  bool shrinkToFit(uint32_t headroom = 256);

  // Compute layout from section counts
  void layout(const uint16_t counts[]);

  // Direct access to internal counts array
  const uint16_t* counts() const { return _counts; }

  // Section accessors (section index is uint8_t, not enum — works for any arena)
  uint8_t* sectionBase(uint8_t section) const {
    return _base + _offsets[section];
  }
  uint32_t sectionOffset(uint8_t section) const {
    return _offsets[section];
  }
  uint16_t sectionCount(uint8_t section) const {
    return _counts[section];
  }

  bool insertSlot(uint32_t byteOffset, uint32_t slotSize);
  void deleteSlot(uint32_t byteOffset, uint32_t slotSize);

  bool insertInSection(uint8_t section, uint32_t indexInSection,
                       uint32_t elementSize);
  void deleteFromSection(uint8_t section, uint32_t indexInSection,
                         uint32_t elementSize);

  bool ensureSectionCapacity(uint8_t section, uint16_t minCount);

  uint16_t trimTrailingEmpty(uint8_t section,
                             bool (*isEmpty)(const uint8_t*));

  // Shrink a section to exactly newCount elements, removing from the tail.
  // Returns number of elements removed. No-op if newCount >= current count.
  uint16_t trimSectionTo(uint8_t section, uint16_t newCount);

  void clear();

  // Element size for a section (from descriptor)
  uint32_t elementSize(uint8_t section) const {
    return _desc->elemSizes[section];
  }
};

// Keep ModelArena as a typedef for backward compatibility
using ModelArena = Arena;

extern Arena g_modelArena;
extern Arena g_radioArena;

void modelArenaInit();
void radioArenaInit();
