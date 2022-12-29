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

static char _custom_names[MAX_ANALOG_INPUTS][LEN_ANA_NAME + 1] = { 0 };

void analogSetCustomName(uint8_t type, uint8_t idx, const char* str, size_t len)
{
  if (idx >= adcGetMaxInputs(type)) return;
  idx += adcGetInputOffset(type);

  strncpy(_custom_names[idx], str, min<size_t>(LEN_ANA_NAME, len));
  _custom_names[idx][LEN_ANA_NAME] = '\0';
}

const char* analogGetCustomName(uint8_t type, uint8_t idx)
{
  if (idx >= adcGetMaxInputs(type)) return "";
  idx += adcGetInputOffset(type);

  return _custom_names[idx];
}

bool analogHasCustomName(uint8_t type, uint8_t idx)
{
  return *analogGetCustomName(type, idx) != 0;
}

// TODO: needs to be generic
static const char* _stick_names[] = {
  "Rud", "Ele", "Thr", "Ail",
};

// TODO: remove special case
static int analogLookupStickIdx(const char* name, size_t len)
{
  for (uint8_t i = 0; i < DIM(_stick_names); i++) {
    if (!strncmp(_stick_names[i], name, len)) return i;
  }

  return -1;
}

// TODO: remove special case
static const char* analogGetCanonicalStickName(uint8_t idx)
{
  if (idx >= DIM(_stick_names)) return "";
  return _stick_names[idx];
}

int analogLookupIdx(uint8_t type, const char* name, size_t len)
{
  auto max_inputs = adcGetMaxInputs(type);
  if (!max_inputs) return -1;

  // TODO: remove special case
  if (type == ADC_INPUT_STICK)
    return analogLookupStickIdx(name, len);
  
  for (uint8_t i = 0; i < max_inputs; i++) {
    if (!strncmp(adcGetInputName(type, i), name, len)) return i;
  }

  return -1;
}

const char* analogGetCanonicalName(uint8_t type, uint8_t idx)
{
  // TODO: remove special case
  if (type == ADC_INPUT_STICK)
    return analogGetCanonicalStickName(idx);

  return adcGetInputName(type, idx);
}

