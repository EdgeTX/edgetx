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
// Arena descriptors (const, in flash)
// ---------------------------------------------------------------------------

static const uint8_t modelElemSizes[MODEL_ARENA_NUM_SECTIONS] = {
  sizeof(MixData),
  sizeof(ExpoData),
  sizeof(CurveHeader),
  sizeof(int8_t),
  sizeof(LogicalSwitchData),
  sizeof(CustomFunctionData),
  sizeof(FlightModeData),
  sizeof(GVarData),
  sizeof(gvar_t),
  LEN_INPUT_NAME,
};

const ArenaDesc modelArenaDesc = { MODEL_ARENA_NUM_SECTIONS, modelElemSizes };

static const uint8_t radioElemSizes[RADIO_ARENA_NUM_SECTIONS] = {
  sizeof(CustomFunctionData),
};

const ArenaDesc radioArenaDesc = { RADIO_ARENA_NUM_SECTIONS, radioElemSizes };

// ---------------------------------------------------------------------------
// Arena buffer allocation (platform-split)
// ---------------------------------------------------------------------------

#if !ARENA_HEAP_GROWABLE
  static uint8_t g_modelArenaBuf[MODEL_ARENA_MAX_SIZE]
      __attribute__((aligned(4)));
  static uint8_t g_radioArenaBuf[RADIO_ARENA_MAX_SIZE]
      __attribute__((aligned(4)));
#endif

Arena g_modelArena;
Arena g_radioArena;

// ---------------------------------------------------------------------------
// Arena implementation
// ---------------------------------------------------------------------------

void Arena::attach(const ArenaDesc* desc, uint8_t* buf,
                   uint32_t capacity, uint32_t maxCapacity)
{
  _desc = desc;
  _base = buf;
  _capacity = capacity;
  _maxCapacity = maxCapacity;
  _heapOwned = false;
  _usedBytes = 0;
  memset(_offsets, 0, sizeof(_offsets));
  memset(_counts, 0, sizeof(_counts));
}

void Arena::release()
{
  if (_heapOwned && _base) {
    free(_base);
  }
  _base = nullptr;
  _capacity = 0;
  _heapOwned = false;
}

uint8_t* Arena::detachHeapBuffer()
{
  if (!_heapOwned)
    return nullptr;
  uint8_t* buf = _base;
  _heapOwned = false;
  return buf;
}

bool Arena::shrinkToFit(uint32_t headroom)
{
  if (!_heapOwned)
    return true;

  if (_capacity <= MODEL_ARENA_INITIAL_SIZE)
    return true;

  uint32_t target = (_usedBytes + headroom + 3u) & ~3u;
  if (target < MODEL_ARENA_INITIAL_SIZE)
    target = MODEL_ARENA_INITIAL_SIZE;
  if (target >= _capacity)
    return true;

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

bool Arena::grow(uint32_t minCapacity)
{
  if (minCapacity > _maxCapacity)
    return false;

  if (minCapacity <= _capacity)
    return true;

  uint32_t newCap = _capacity * 2;
  if (newCap < minCapacity)
    newCap = minCapacity;
  if (newCap > _maxCapacity)
    newCap = _maxCapacity;

  newCap = (newCap + 3u) & ~3u;

  if (!_heapOwned) {
    _capacity = newCap;
    return true;
  }

  uint8_t* newBuf = (uint8_t*)malloc(newCap);
  if (!newBuf)
    return false;

  TRACE("Arena::grow %u -> %u (used %u)", _capacity, newCap, _usedBytes);

  memcpy(newBuf, _base, _usedBytes);
  memset(newBuf + _usedBytes, 0, newCap - _usedBytes);

  free(_base);

  _base = newBuf;
  _capacity = newCap;
  return true;
}

void Arena::layout(const uint16_t counts[])
{
  uint32_t offset = 0;
  for (uint8_t i = 0; i < _desc->numSections; i++) {
    _counts[i] = counts[i];
    _offsets[i] = offset;
    offset += _counts[i] * _desc->elemSizes[i];
  }
  _usedBytes = offset;
}

void Arena::clear()
{
  if (_base) {
    memset(_base, 0, _capacity);
  }
}

bool Arena::insertSlot(uint32_t byteOffset, uint32_t slotSize)
{
  if (_usedBytes + slotSize > _capacity) {
    if (!grow(_usedBytes + slotSize))
      return false;
  }

  uint32_t tailSize = _usedBytes - byteOffset;
  if (tailSize > 0) {
    memmove(_base + byteOffset + slotSize, _base + byteOffset, tailSize);
  }

  memset(_base + byteOffset, 0, slotSize);
  _usedBytes += slotSize;
  return true;
}

void Arena::deleteSlot(uint32_t byteOffset, uint32_t slotSize)
{
  uint32_t tailStart = byteOffset + slotSize;
  uint32_t tailSize = _usedBytes - tailStart;
  if (tailSize > 0) {
    memmove(_base + byteOffset, _base + tailStart, tailSize);
  }

  memset(_base + _usedBytes - slotSize, 0, slotSize);
  _usedBytes -= slotSize;
}

bool Arena::insertInSection(uint8_t section, uint32_t indexInSection,
                            uint32_t elementSize)
{
  uint32_t byteOffset = _offsets[section] + indexInSection * elementSize;

  if (!insertSlot(byteOffset, elementSize))
    return false;

  _counts[section]++;
  for (uint8_t i = section + 1; i < _desc->numSections; i++) {
    _offsets[i] += elementSize;
  }

  return true;
}

void Arena::deleteFromSection(uint8_t section, uint32_t indexInSection,
                              uint32_t elementSize)
{
  uint32_t byteOffset = _offsets[section] + indexInSection * elementSize;

  deleteSlot(byteOffset, elementSize);

  _counts[section]--;
  for (uint8_t i = section + 1; i < _desc->numSections; i++) {
    _offsets[i] -= elementSize;
  }
}

bool Arena::ensureSectionCapacity(uint8_t section, uint16_t minCount)
{
  uint32_t elemSize = _desc->elemSizes[section];
  uint32_t currentCount = _counts[section];

  if (currentCount >= minCount)
    return true;

  uint32_t needed = minCount - currentCount;
  uint32_t bytesNeeded = needed * elemSize;

  if (_usedBytes + bytesNeeded > _capacity) {
    if (!grow(_usedBytes + bytesNeeded))
      return false;
  }

  uint32_t insertOffset = _offsets[section] + currentCount * elemSize;
  uint32_t tailSize = _usedBytes - insertOffset;
  if (tailSize > 0) {
    memmove(_base + insertOffset + bytesNeeded,
            _base + insertOffset, tailSize);
  }
  memset(_base + insertOffset, 0, bytesNeeded);
  _usedBytes += bytesNeeded;
  _counts[section] = minCount;

  for (uint8_t i = section + 1; i < _desc->numSections; i++) {
    _offsets[i] += bytesNeeded;
  }

  return true;
}

uint16_t Arena::trimTrailingEmpty(uint8_t section,
                                  bool (*isEmpty)(const uint8_t*))
{
  uint32_t elemSize = _desc->elemSizes[section];
  uint16_t count = _counts[section];
  if (count == 0) return 0;

  uint8_t* base = _base + _offsets[section];
  int last = count - 1;
  while (last >= 0 && isEmpty(base + last * elemSize))
    --last;

  uint16_t newCount = (uint16_t)(last + 1);
  uint16_t removed = count - newCount;
  if (removed == 0) return 0;

  uint32_t bytesToRemove = removed * elemSize;
  uint32_t removeOffset = _offsets[section] + newCount * elemSize;

  uint32_t tailSize = _usedBytes - (removeOffset + bytesToRemove);
  if (tailSize > 0) {
    memmove(_base + removeOffset,
            _base + removeOffset + bytesToRemove, tailSize);
  }
  memset(_base + _usedBytes - bytesToRemove, 0, bytesToRemove);

  _usedBytes -= bytesToRemove;
  _counts[section] = newCount;
  for (uint8_t i = section + 1; i < _desc->numSections; i++) {
    _offsets[i] -= bytesToRemove;
  }

  return removed;
}

uint16_t Arena::trimSectionTo(uint8_t section, uint16_t newCount)
{
  uint16_t count = _counts[section];
  if (newCount >= count) return 0;

  uint32_t elemSize = _desc->elemSizes[section];
  uint16_t removed = count - newCount;
  uint32_t bytesToRemove = removed * elemSize;
  uint32_t removeOffset = _offsets[section] + newCount * elemSize;

  uint32_t tailSize = _usedBytes - (removeOffset + bytesToRemove);
  if (tailSize > 0) {
    memmove(_base + removeOffset,
            _base + removeOffset + bytesToRemove, tailSize);
  }
  memset(_base + _usedBytes - bytesToRemove, 0, bytesToRemove);

  _usedBytes -= bytesToRemove;
  _counts[section] = newCount;
  for (uint8_t i = section + 1; i < _desc->numSections; i++) {
    _offsets[i] -= bytesToRemove;
  }

  return removed;
}

// ---------------------------------------------------------------------------
// Init functions
// ---------------------------------------------------------------------------

static void arenaInit(Arena& arena, const ArenaDesc* desc,
                      uint8_t* staticBuf,
                      uint32_t initialSize, uint32_t maxSize)
{
  arena.release();

#if ARENA_HEAP_GROWABLE
  (void)staticBuf;
  uint8_t* buf = (uint8_t*)malloc(initialSize);
  arena.attach(desc, buf, initialSize, maxSize);
  arena.setHeapOwned(true);
#else
  arena.attach(desc, staticBuf, initialSize, maxSize);
#endif

  uint16_t emptyCounts[ARENA_MAX_SECTIONS] = {};
  arena.layout(emptyCounts);
  arena.clear();
}

void modelArenaInit()
{
#if !ARENA_HEAP_GROWABLE
  arenaInit(g_modelArena, &modelArenaDesc, g_modelArenaBuf,
            MODEL_ARENA_INITIAL_SIZE, MODEL_ARENA_MAX_SIZE);
#else
  arenaInit(g_modelArena, &modelArenaDesc, nullptr,
            MODEL_ARENA_INITIAL_SIZE, MODEL_ARENA_MAX_SIZE);
#endif
}

void radioArenaInit()
{
#if !ARENA_HEAP_GROWABLE
  arenaInit(g_radioArena, &radioArenaDesc, g_radioArenaBuf,
            RADIO_ARENA_INITIAL_SIZE, RADIO_ARENA_MAX_SIZE);
#else
  arenaInit(g_radioArena, &radioArenaDesc, nullptr,
            RADIO_ARENA_INITIAL_SIZE, RADIO_ARENA_MAX_SIZE);
#endif
}
