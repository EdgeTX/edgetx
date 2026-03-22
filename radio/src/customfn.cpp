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

CustomFunctionData *customFnAddress(uint8_t idx)
{
  return reinterpret_cast<CustomFunctionData*>(
      g_modelArena.sectionBase(ARENA_CUSTOM_FN)) + idx;
}

void insertCustomFn(uint8_t idx)
{
  CustomFunctionData* cfn = customFnAddress(idx);
  memmove(cfn + 1, cfn,
          (MAX_SPECIAL_FUNCTIONS - idx - 1) * sizeof(CustomFunctionData));
  memset(cfn, 0, sizeof(CustomFunctionData));
  storageDirty(EE_MODEL);
}

void deleteCustomFn(uint8_t idx)
{
  CustomFunctionData* cfn = customFnAddress(idx);
  memmove(cfn, cfn + 1,
          (MAX_SPECIAL_FUNCTIONS - idx - 1) * sizeof(CustomFunctionData));
  memset(customFnAddress(MAX_SPECIAL_FUNCTIONS - 1), 0,
         sizeof(CustomFunctionData));
  storageDirty(EE_MODEL);
}

void clearCustomFn(uint8_t idx)
{
  memset(customFnAddress(idx), 0, sizeof(CustomFunctionData));
  storageDirty(EE_MODEL);
}
