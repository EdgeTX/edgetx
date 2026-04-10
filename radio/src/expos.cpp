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

#include "expos.h"
#include "tasks/mixer_task.h"
#include "hal/adc_driver.h"
#include "model_arena.h"
#include "dataconstants.h"

#include "edgetx.h"

#include <string.h>

static uint8_t _nb_expo_lines;

ExpoData* expoAddress(uint8_t idx) {
  return reinterpret_cast<ExpoData*>(
      g_modelArena.sectionBase(ARENA_EXPOS)) + idx;
}

ExpoData* expoAllocAt(uint8_t idx) {
  if (idx >= g_modelArena.sectionCount(ARENA_EXPOS)) {
    if (!g_modelArena.ensureSectionCapacity(ARENA_EXPOS, idx + 1))
      return nullptr;
  }
  return expoAddress(idx);
}

uint8_t getExpoCount() { return _nb_expo_lines; }

constexpr SourceTypeMask insertExpoMask =
  SRC_TYPE_BIT(SOURCE_TYPE_STICK) |
  SRC_TYPE_BIT(SOURCE_TYPE_POT) |
  SRC_TYPE_BIT(SOURCE_TYPE_SWITCH);

void insertExpo(uint8_t idx, uint8_t input)
{
  mixerTaskStop();

  if (!g_modelArena.insertInSection(ARENA_EXPOS, idx, sizeof(ExpoData))) {
    mixerTaskStart();
    return;
  }

  ExpoData* expo = expoAddress(idx);
  if (input < adcGetMaxInputs(ADC_INPUT_MAIN)) {
    expo->srcRaw = SourceRef_(SOURCE_TYPE_STICK, (uint16_t)inputMappingChannelOrder(input));
  } else {
    expo->srcRaw = nthAvailableSource(input, insertExpoMask);
  }
  expo->curve.type = CURVE_REF_EXPO;
  expo->mode = 3;  // pos+neg
  expo->chn = input;
  expo->weight.setNumeric(100);
  mixerTaskStart();

  _nb_expo_lines += 1;
  storageDirty(EE_MODEL);
}

void deleteExpo(uint8_t idx)
{
  mixerTaskStop();
  int input = expoAddress(idx)->chn;

  g_modelArena.deleteFromSection(ARENA_EXPOS, idx, sizeof(ExpoData));

  if (!isInputAvailable(input)) {
    inputNameClear(input);
  }
  mixerTaskStart();

  _nb_expo_lines -= 1;
  storageDirty(EE_MODEL);
}

void copyExpo(uint8_t source, uint8_t dest, uint8_t input)
{
  mixerTaskStop();
  ExpoData sourceExpo;
  memcpy(&sourceExpo, expoAddress(source), sizeof(ExpoData));

  if (!g_modelArena.insertInSection(ARENA_EXPOS, dest, sizeof(ExpoData))) {
    mixerTaskStart();
    return;
  }

  ExpoData* expo = expoAddress(dest);
  memcpy(expo, &sourceExpo, sizeof(ExpoData));
  expo->chn = input;
  mixerTaskStart();

  _nb_expo_lines += 1;
  storageDirty(EE_MODEL);
}

uint8_t moveExpo(uint8_t idx, bool up)
{
  ExpoData* x;
  ExpoData* y;
  int8_t tgt_idx = (up ? idx - 1 : idx + 1);

  x = expoAddress(idx);

  if (tgt_idx < 0) {
    if (x->chn > 0) {
      x->chn--;
      storageDirty(EE_MODEL);
    }
    return idx;
  }

  if (tgt_idx == getExpoCount()) {
    if (x->chn < MAX_INPUTS - 1) {
      x->chn++;
      storageDirty(EE_MODEL);
    }
    return idx;
  }

  y = expoAddress(tgt_idx);
  uint8_t chn = x->chn;

  if (!EXPO_VALID(y) || chn != y->chn) {
    if (up) {
      if (chn > 0) {
        x->chn--;
        storageDirty(EE_MODEL);
      }
    } else {
      if (chn < MAX_INPUTS - 1) {
        x->chn++;
        storageDirty(EE_MODEL);
      }
    }
    return idx;
  }

  mixerTaskStop();
  memswap(x, y, sizeof(ExpoData));
  mixerTaskStart();

  storageDirty(EE_MODEL);
  return tgt_idx;
}

static uint8_t _countExpoLines()
{
  uint8_t i = 0;
  uint8_t limit = g_modelArena.sectionCount(ARENA_EXPOS);
  while (i < limit) {
    if (is_memclear(expoAddress(i), sizeof(ExpoData))) break;
    i++;
  }
  return i;
}

void updateExpoCount()
{
  _nb_expo_lines = _countExpoLines();
}

// ---------------------------------------------------------------------------
// Input name accessors (arena-backed, sparse via side-array index)
// ---------------------------------------------------------------------------

#define INPUT_NAME_NO_SLOT 0xFF

void inputNameIndexReset()
{
  memset(g_model.inputNameIndex, INPUT_NAME_NO_SLOT,
         sizeof(g_model.inputNameIndex));
}

static inline char* _inputNameSlot(uint8_t slot)
{
  return reinterpret_cast<char*>(
      g_modelArena.sectionBase(ARENA_INPUT_NAMES)) + slot * LEN_INPUT_NAME;
}

const char* inputName(uint8_t input)
{
  uint8_t slot = g_model.inputNameIndex[input];
  if (slot == INPUT_NAME_NO_SLOT)
    return nullptr;
  return _inputNameSlot(slot);
}

bool hasInputName(uint8_t input)
{
  return g_model.inputNameIndex[input] != INPUT_NAME_NO_SLOT;
}

char* inputNameAlloc(uint8_t input)
{
  uint8_t slot = g_model.inputNameIndex[input];
  if (slot != INPUT_NAME_NO_SLOT)
    return _inputNameSlot(slot);

  // Append a new slot
  uint16_t count = g_modelArena.sectionCount(ARENA_INPUT_NAMES);
  if (!g_modelArena.ensureSectionCapacity(ARENA_INPUT_NAMES, count + 1))
    return nullptr;

  g_model.inputNameIndex[input] = count;
  return _inputNameSlot(count);
}

void inputNameClear(uint8_t input)
{
  uint8_t slot = g_model.inputNameIndex[input];
  if (slot == INPUT_NAME_NO_SLOT)
    return;

  uint16_t count = g_modelArena.sectionCount(ARENA_INPUT_NAMES);
  uint16_t lastSlot = count - 1;

  if (slot != lastSlot) {
    // Find which input owns the last slot and swap
    for (uint8_t i = 0; i < MAX_INPUTS; i++) {
      if (g_model.inputNameIndex[i] == lastSlot) {
        memcpy(_inputNameSlot(slot), _inputNameSlot(lastSlot), LEN_INPUT_NAME);
        g_model.inputNameIndex[i] = slot;
        break;
      }
    }
  }

  g_model.inputNameIndex[input] = INPUT_NAME_NO_SLOT;
  g_modelArena.trimSectionTo(ARENA_INPUT_NAMES, lastSlot);
}
