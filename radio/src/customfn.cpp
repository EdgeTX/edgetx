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

#include "customfn.h"
#include "edgetx.h"
#include "model_arena.h"

uint8_t getCustomFnCount()
{
  return g_modelArena.sectionCount(ARENA_CUSTOM_FN);
}

CustomFunctionData *customFnAddress(uint8_t idx)
{
  if (idx >= g_modelArena.sectionCount(ARENA_CUSTOM_FN)) {
    static CustomFunctionData dummy = {};
    memset(&dummy, 0, sizeof(dummy));
    return &dummy;
  }
  return reinterpret_cast<CustomFunctionData*>(
      g_modelArena.sectionBase(ARENA_CUSTOM_FN)) + idx;
}

CustomFunctionData *customFnAllocAt(uint8_t idx)
{
  if (idx >= g_modelArena.sectionCount(ARENA_CUSTOM_FN)) {
    if (!g_modelArena.ensureSectionCapacity(ARENA_CUSTOM_FN, idx + 1))
      return nullptr;
  }
  return customFnAddress(idx);
}

static bool customFnIsEmpty(const uint8_t* ptr)
{
  return reinterpret_cast<const CustomFunctionData*>(ptr)->swtch.isNone();
}

void customFnTrimTrailing()
{
  g_modelArena.trimTrailingEmpty(ARENA_CUSTOM_FN, customFnIsEmpty);
}

void insertCustomFn(uint8_t idx)
{
  if (!g_modelArena.insertInSection(ARENA_CUSTOM_FN, idx,
                                     sizeof(CustomFunctionData)))
    return;
  storageDirty(EE_MODEL);
}

void deleteCustomFn(uint8_t idx)
{
  g_modelArena.deleteFromSection(ARENA_CUSTOM_FN, idx,
                                  sizeof(CustomFunctionData));
  storageDirty(EE_MODEL);
}

void clearCustomFn(uint8_t idx)
{
  memset(customFnAddress(idx), 0, sizeof(CustomFunctionData));
  storageDirty(EE_MODEL);
}

// ---- Radio (global) custom function accessors ----

CustomFunctionData *globalFnAddress(uint8_t idx)
{
  if (idx >= g_radioArena.sectionCount(RADIO_ARENA_CUSTOM_FN)) {
    static CustomFunctionData dummy = {};
    memset(&dummy, 0, sizeof(dummy));
    return &dummy;
  }
  return reinterpret_cast<CustomFunctionData*>(
      g_radioArena.sectionBase(RADIO_ARENA_CUSTOM_FN)) + idx;
}

CustomFunctionData *globalFnAllocAt(uint8_t idx)
{
  if (idx >= g_radioArena.sectionCount(RADIO_ARENA_CUSTOM_FN)) {
    if (!g_radioArena.ensureSectionCapacity(RADIO_ARENA_CUSTOM_FN, idx + 1))
      return nullptr;
  }
  return globalFnAddress(idx);
}

void globalFnTrimTrailing()
{
  g_radioArena.trimTrailingEmpty(RADIO_ARENA_CUSTOM_FN, customFnIsEmpty);
}

void insertGlobalFn(uint8_t idx)
{
  if (!g_radioArena.insertInSection(RADIO_ARENA_CUSTOM_FN, idx,
                                     sizeof(CustomFunctionData)))
    return;
  storageDirty(EE_GENERAL);
}

void deleteGlobalFn(uint8_t idx)
{
  g_radioArena.deleteFromSection(RADIO_ARENA_CUSTOM_FN, idx,
                                  sizeof(CustomFunctionData));
  storageDirty(EE_GENERAL);
}

void clearGlobalFn(uint8_t idx)
{
  memset(globalFnAddress(idx), 0, sizeof(CustomFunctionData));
  storageDirty(EE_GENERAL);
}
