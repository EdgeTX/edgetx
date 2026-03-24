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

#include "model_arena.h"
#include "dataconstants.h"
#include "datastructs_private.h"

#include <stdlib.h>

// ---------------------------------------------------------------------------
// Arena buffer allocation (platform-split)
// ---------------------------------------------------------------------------

#if !ARENA_HEAP_GROWABLE
  // Static mode: max-sized buffer in fast internal RAM.
  // BSS goes to internal RAM on all SDRAM targets (RAM on F429, RAM_D1 on H7),
  // so no section attribute is needed — the buffer naturally lands in fast SRAM.
  static uint8_t g_modelArenaBuf[MODEL_ARENA_MAX_SIZE]
      __attribute__((aligned(4)));
#endif

ModelArena g_modelArena;

// Element sizes for each section type (in bytes)
static const uint8_t sectionElementSize[ARENA_NUM_SECTIONS] = {
  sizeof(MixData),              // ARENA_MIXES
  sizeof(ExpoData),             // ARENA_EXPOS
  sizeof(CurveHeader),          // ARENA_CURVES
  sizeof(int8_t),               // ARENA_POINTS
  sizeof(LogicalSwitchData),    // ARENA_LOGICAL_SW
  sizeof(CustomFunctionData),   // ARENA_CUSTOM_FN
};

static uint16_t getSectionCount(const ModelDynData& dyn, ArenaSectionType type)
{
  switch (type) {
    case ARENA_MIXES:      return dyn.mixCount;
    case ARENA_EXPOS:      return dyn.expoCount;
    case ARENA_CURVES:     return dyn.curveCount;
    case ARENA_POINTS:     return dyn.pointsCount;
    case ARENA_LOGICAL_SW: return dyn.logicalSwCount;
    case ARENA_CUSTOM_FN:  return dyn.customFnCount;
    default:               return 0;
  }
}

void ModelArena::attach(uint8_t* buf, uint32_t capacity, uint32_t maxCapacity)
{
  _base = buf;
  _capacity = capacity;
  _maxCapacity = maxCapacity;
  _heapOwned = false;
  _usedBytes = 0;
  memset(_offsets, 0, sizeof(_offsets));
  memset(_counts, 0, sizeof(_counts));
}

void ModelArena::release()
{
  if (_heapOwned && _base) {
    free(_base);
  }
  _base = nullptr;
  _capacity = 0;
  _heapOwned = false;
}

uint8_t* ModelArena::detachHeapBuffer()
{
  if (!_heapOwned)
    return nullptr;
  uint8_t* buf = _base;
  _heapOwned = false;
  return buf;
}

bool ModelArena::shrinkToFit(uint32_t headroom)
{
  if (!_heapOwned) {
    // Static mode: buffer is fixed, nothing to shrink
    return true;
  }

  // Don't reallocate if we haven't grown beyond the initial allocation
  if (_capacity <= MODEL_ARENA_INITIAL_SIZE)
    return true;

  // Shrink back to initial size at most — avoids realloc churn for
  // buffers that only grew slightly
  uint32_t target = (_usedBytes + headroom + 3u) & ~3u;  // 4-byte aligned
  if (target < MODEL_ARENA_INITIAL_SIZE)
    target = MODEL_ARENA_INITIAL_SIZE;
  if (target >= _capacity)
    return true;  // already right-sized or smaller

  uint8_t* newBuf = (uint8_t*)malloc(target);
  if (!newBuf)
    return false;

  memcpy(newBuf, _base, _usedBytes);
  memset(newBuf + _usedBytes, 0, target - _usedBytes);

  free(_base);

  _base = newBuf;
  _capacity = target;
  return true;
}

bool ModelArena::grow(uint32_t minCapacity)
{
  if (minCapacity > _maxCapacity)
    return false;

  if (minCapacity <= _capacity)
    return true;

  // Growth policy: double, capped at _maxCapacity
  uint32_t newCap = _capacity * 2;
  if (newCap < minCapacity)
    newCap = minCapacity;
  if (newCap > _maxCapacity)
    newCap = _maxCapacity;

  // 4-byte align
  newCap = (newCap + 3u) & ~3u;

  if (!_heapOwned) {
    // Static mode: buffer is already MODEL_ARENA_MAX_SIZE, just widen the view.
    // This is safe because the static buffer was allocated at max size.
    _capacity = newCap;
    return true;
  }

  // Heap mode: allocate new buffer, copy data, free old
  uint8_t* newBuf = (uint8_t*)malloc(newCap);
  if (!newBuf)
    return false;

  memcpy(newBuf, _base, _usedBytes);
  memset(newBuf + _usedBytes, 0, newCap - _usedBytes);

  free(_base);

  _base = newBuf;
  _capacity = newCap;
  return true;
}

void ModelArena::recalcOffsets(const ModelDynData& dyn)
{
  uint16_t offset = 0;
  for (int i = 0; i < ARENA_NUM_SECTIONS; i++) {
    _counts[i] = getSectionCount(dyn, (ArenaSectionType)i);
    _offsets[i] = offset;
    offset += _counts[i] * sectionElementSize[i];
  }
  _usedBytes = offset;
}

void ModelArena::layout(const ModelDynData& dyn)
{
  recalcOffsets(dyn);
}

void ModelArena::clear()
{
  if (_base) {
    memset(_base, 0, _capacity);
  }
  // Note: preserves layout (offsets and usedBytes) - only zeros data
}

bool ModelArena::insertSlot(uint32_t byteOffset, uint32_t slotSize)
{
  if (_usedBytes + slotSize > _capacity) {
    if (!grow(_usedBytes + slotSize))
      return false;
  }

  // Shift everything from byteOffset forward by slotSize
  uint32_t tailSize = _usedBytes - byteOffset;
  if (tailSize > 0) {
    memmove(_base + byteOffset + slotSize, _base + byteOffset, tailSize);
  }

  // Clear the new slot
  memset(_base + byteOffset, 0, slotSize);
  _usedBytes += slotSize;
  return true;
}

void ModelArena::deleteSlot(uint32_t byteOffset, uint32_t slotSize)
{
  uint32_t tailStart = byteOffset + slotSize;
  uint32_t tailSize = _usedBytes - tailStart;
  if (tailSize > 0) {
    memmove(_base + byteOffset, _base + tailStart, tailSize);
  }

  // Clear freed space at the end
  memset(_base + _usedBytes - slotSize, 0, slotSize);
  _usedBytes -= slotSize;
}

bool ModelArena::insertInSection(ArenaSectionType section,
                                 uint32_t indexInSection,
                                 uint32_t elementSize)
{
  uint32_t byteOffset = _offsets[section] + indexInSection * elementSize;

  if (!insertSlot(byteOffset, elementSize))
    return false;

  _counts[section]++;
  for (int i = section + 1; i < ARENA_NUM_SECTIONS; i++) {
    _offsets[i] += elementSize;
  }

  return true;
}

void ModelArena::deleteFromSection(ArenaSectionType section,
                                   uint32_t indexInSection,
                                   uint32_t elementSize)
{
  uint32_t byteOffset = _offsets[section] + indexInSection * elementSize;

  deleteSlot(byteOffset, elementSize);

  _counts[section]--;
  for (int i = section + 1; i < ARENA_NUM_SECTIONS; i++) {
    _offsets[i] -= elementSize;
  }
}

uint32_t ModelArena::elementSize(ArenaSectionType type)
{
  return sectionElementSize[type];
}

bool ModelArena::ensureSectionCapacity(ArenaSectionType section,
                                       uint16_t minCount)
{
  uint32_t elemSize = sectionElementSize[section];
  uint32_t currentCount = _counts[section];

  if (currentCount >= minCount)
    return true;

  uint32_t needed = minCount - currentCount;
  uint32_t bytesNeeded = needed * elemSize;

  if (_usedBytes + bytesNeeded > _capacity) {
    if (!grow(_usedBytes + bytesNeeded))
      return false;
  }

  // Bulk-insert at the end of the section
  uint32_t insertOffset = _offsets[section] + currentCount * elemSize;
  uint32_t tailSize = _usedBytes - insertOffset;
  if (tailSize > 0) {
    memmove(_base + insertOffset + bytesNeeded,
            _base + insertOffset, tailSize);
  }
  memset(_base + insertOffset, 0, bytesNeeded);
  _usedBytes += bytesNeeded;
  _counts[section] = minCount;

  // Update subsequent section offsets
  for (int i = section + 1; i < ARENA_NUM_SECTIONS; i++) {
    _offsets[i] += bytesNeeded;
  }

  return true;
}

uint16_t ModelArena::trimTrailingEmpty(ArenaSectionType section,
                                       bool (*isEmpty)(const uint8_t*))
{
  uint32_t elemSize = sectionElementSize[section];
  uint16_t count = _counts[section];
  if (count == 0) return 0;

  // Find last non-empty element
  uint8_t* base = _base + _offsets[section];
  int last = count - 1;
  while (last >= 0 && isEmpty(base + last * elemSize))
    --last;

  uint16_t newCount = (uint16_t)(last + 1);
  uint16_t removed = count - newCount;
  if (removed == 0) return 0;

  uint32_t bytesToRemove = removed * elemSize;
  uint32_t removeOffset = _offsets[section] + newCount * elemSize;

  // Shift subsequent section data backward
  uint32_t tailSize = _usedBytes - (removeOffset + bytesToRemove);
  if (tailSize > 0) {
    memmove(_base + removeOffset,
            _base + removeOffset + bytesToRemove, tailSize);
  }
  memset(_base + _usedBytes - bytesToRemove, 0, bytesToRemove);

  _usedBytes -= bytesToRemove;
  _counts[section] = newCount;
  for (int i = section + 1; i < ARENA_NUM_SECTIONS; i++) {
    _offsets[i] -= bytesToRemove;
  }

  return removed;
}

// Initialize arena on startup with empty layout.
// Sections are allocated on demand during YAML parsing or via explicit insert.
void modelArenaInit()
{
  g_modelArena.release();

#if ARENA_HEAP_GROWABLE
  uint8_t* buf = (uint8_t*)malloc(MODEL_ARENA_INITIAL_SIZE);
  g_modelArena.attach(buf, MODEL_ARENA_INITIAL_SIZE, MODEL_ARENA_MAX_SIZE);
  g_modelArena.setHeapOwned(true);
#else
  g_modelArena.attach(g_modelArenaBuf, MODEL_ARENA_INITIAL_SIZE,
                      MODEL_ARENA_MAX_SIZE);
#endif

  ModelDynData emptyDyn = {};
  g_modelArena.layout(emptyDyn);
  g_modelArena.clear();
}
