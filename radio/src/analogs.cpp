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
#include "edgetx_helpers.h"

#include "hal/adc_driver.h"

static char _custom_names[MAX_ANALOG_INPUTS][LEN_ANA_NAME + 1] = { 0 };

void analogSetCustomLabel(uint8_t type, uint8_t idx, const char* str, size_t len)
{
  if (idx >= adcGetMaxInputs(type)) return;
  idx += adcGetInputOffset(type);

  strncpy(_custom_names[idx], str, min<size_t>(LEN_ANA_NAME, len));
  _custom_names[idx][LEN_ANA_NAME] = '\0';
}

const char* analogGetCustomLabel(uint8_t type, uint8_t idx)
{
  if (idx >= adcGetMaxInputs(type)) return "";
  idx += adcGetInputOffset(type);

  return _custom_names[idx];
}

bool analogHasCustomLabel(uint8_t type, uint8_t idx)
{
  return *analogGetCustomLabel(type, idx) != 0;
}

static int _lookup_input_idx(uint8_t type, const char* name, size_t len,
                             const char* (*fct)(uint8_t,uint8_t))
{
  auto max_inputs = adcGetMaxInputs(type);
  if (!max_inputs) return -1;

  for (uint8_t i = 0; i < max_inputs; i++) {
    if (!strncmp(fct(type, i), name, len)) return i;
  }

  return -1;
}

int analogLookupPhysicalIdx(uint8_t type, const char* name, size_t len)
{
  return _lookup_input_idx(type, name, len, adcGetInputName);
}

static int analogLookupLabelIdx(uint8_t type, const char* name, size_t len)
{
  return _lookup_input_idx(type, name, len, adcGetInputLabel);
}

const char* analogGetPhysicalName(uint8_t type, uint8_t idx)
{
  return adcGetInputName(type, idx);
}

const char* analogGetCanonicalName(uint8_t type, uint8_t idx)
{
  // Main controls are special cases here as
  // we use the label slot to place the specific names
  // (2-gimbal radios vs. surface radios)

  if (type == ADC_INPUT_MAIN)
    return adcGetInputLabel(type, idx);

  return adcGetInputName(type, idx);
}

int analogLookupCanonicalIdx(uint8_t type, const char* name, size_t len)
{
  if (type == ADC_INPUT_MAIN) {
    return analogLookupLabelIdx(type, name, len);
  }

  return analogLookupPhysicalIdx(type, name, len);
}

