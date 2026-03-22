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

#include "edgetx.h"

static uint8_t _nb_expo_lines;

ExpoData* expoAddress(uint8_t idx) { return &g_model.expoData[idx]; }

uint8_t getExpoCount() { return _nb_expo_lines; }

void insertExpo(uint8_t idx, uint8_t input)
{
  mixerTaskStop();
  ExpoData* expo = expoAddress(idx);
  memmove(expo + 1, expo, (MAX_EXPOS - (idx + 1)) * sizeof(ExpoData));
  memclear(expo, sizeof(ExpoData));
  if (input >= adcGetMaxInputs(ADC_INPUT_MAIN)) {
    expo->srcRaw = MIXSRC_FIRST_STICK + input;
  } else {
    expo->srcRaw = MIXSRC_FIRST_STICK + inputMappingChannelOrder(input);
  }
  expo->curve.type = CURVE_REF_EXPO;
  expo->mode = 3;  // pos+neg
  expo->chn = input;
  expo->weight = 100;
  mixerTaskStart();

  _nb_expo_lines += 1;
  storageDirty(EE_MODEL);
}

void deleteExpo(uint8_t idx)
{
  mixerTaskStop();
  ExpoData* expo = expoAddress(idx);
  int input = expo->chn;
  memmove(expo, expo + 1, (MAX_EXPOS - (idx + 1)) * sizeof(ExpoData));
  memclear(expoAddress(MAX_EXPOS - 1), sizeof(ExpoData));
  if (!isInputAvailable(input)) {
    memclear(&g_model.inputNames[input], LEN_INPUT_NAME);
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
  ExpoData* expo = expoAddress(dest);
  size_t trailingExpos = MAX_EXPOS - (dest + 1);
  memmove(expo + 1, expo, trailingExpos * sizeof(ExpoData));
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

  if (tgt_idx == MAX_EXPOS) {
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
  do {
    if (is_memclear(expoAddress(i), sizeof(ExpoData))) break;
  } while (++i < MAX_EXPOS);
  return i;
}

void updateExpoCount()
{
  _nb_expo_lines = _countExpoLines();
}
