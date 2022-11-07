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

#include "analogs.h"
#include "dataconstants.h"
#include "opentx_helpers.h"

#include "hal/adc_driver.h"

static char _stickNames[MAX_STICKS][LEN_ANA_NAME + 1] = { 0 };
static char _potNames[MAX_STICKS][LEN_ANA_NAME + 1] = { 0 };

void analogSetCustomStickName(uint8_t idx, const char* str, size_t len)
{
  strncpy(_stickNames[idx], str, min<size_t>(LEN_ANA_NAME, len));
  _stickNames[idx][LEN_ANA_NAME] = '\0';
}

const char* analogGetCustomStickName(uint8_t idx)
{
  return _stickNames[idx];
}

bool analogHasCustomStickName(uint8_t idx)
{
  return *analogGetCustomStickName(idx) != 0;
}

void analogSetCustomPotName(uint8_t idx, const char* str, size_t len)
{
  strncpy(_potNames[idx], str, min<size_t>(LEN_ANA_NAME, len));
  _potNames[idx][LEN_ANA_NAME] = '\0';
}

const char* analogGetCustomPotName(uint8_t idx)
{
  return _potNames[idx];
}

bool analogHasCustomPotName(uint8_t idx)
{
  return *analogGetCustomPotName(idx) != 0;
}

int analogLookupPotIdx(const char* name, size_t len)
{
  for (uint8_t i = 0; i < adcGetMaxPots(); i++) {
    if (!strncmp(adcGetPotName(i), name, len)) return i;
  }

  return -1;
}

const char* analogGetCanonicalPotName(uint8_t idx)
{
  return adcGetPotName(idx);
}

