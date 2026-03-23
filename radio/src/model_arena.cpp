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

static uint8_t g_modelArenaBuf[MODEL_ARENA_SIZE] __attribute__((aligned(4)));

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

void ModelArena::attach(uint8_t* buf, uint32_t capacity)
{
  _base = buf;
  _capacity = capacity;
  _usedBytes = 0;
  memset(_offsets, 0, sizeof(_offsets));
}

void ModelArena::recalcOffsets(const ModelDynData& dyn)
{
  uint16_t offset = 0;
  for (int i = 0; i < ARENA_NUM_SECTIONS; i++) {
    _offsets[i] = offset;
    offset += getSectionCount(dyn, (ArenaSectionType)i) * sectionElementSize[i];
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

  // Update offsets for all subsequent sections
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

  // Update offsets for all subsequent sections
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
  // Determine current section count from offset difference
  uint32_t elemSize = sectionElementSize[section];
  uint32_t endOffset;
  if (section + 1 < ARENA_NUM_SECTIONS)
    endOffset = _offsets[section + 1];
  else
    endOffset = _usedBytes;
  uint32_t currentCount = (endOffset - _offsets[section]) / elemSize;

  if (currentCount >= minCount)
    return true;

  uint32_t needed = minCount - currentCount;
  uint32_t bytesNeeded = needed * elemSize;

  if (_usedBytes + bytesNeeded > _capacity)
    return false;

  // Bulk-insert at the end of the section
  uint32_t insertOffset = endOffset;
  uint32_t tailSize = _usedBytes - insertOffset;
  if (tailSize > 0) {
    memmove(_base + insertOffset + bytesNeeded,
            _base + insertOffset, tailSize);
  }
  memset(_base + insertOffset, 0, bytesNeeded);
  _usedBytes += bytesNeeded;

  // Update subsequent section offsets
  for (int i = section + 1; i < ARENA_NUM_SECTIONS; i++) {
    _offsets[i] += bytesNeeded;
  }

  return true;
}

// Initialize arena on startup with empty layout.
// Sections are allocated on demand during YAML parsing or via explicit insert.
void modelArenaInit()
{
  g_modelArena.attach(g_modelArenaBuf, MODEL_ARENA_SIZE);
  ModelDynData emptyDyn = {};
  g_modelArena.layout(emptyDyn);
  g_modelArena.clear();
}
