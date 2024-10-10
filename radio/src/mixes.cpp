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

#include "mixes.h"
#include "tasks/mixer_task.h"
#include "hal/adc_driver.h"

#include "edgetx.h"

static uint8_t _nb_mix_lines;

MixData* mixAddress(uint8_t idx) { return &g_model.mixData[idx]; }

uint8_t getMixCount() { return _nb_mix_lines; }

void insertMix(uint8_t idx, uint8_t channel)
{
  mixerTaskStop();
  MixData * mix = mixAddress(idx);
  memmove(mix + 1, mix, (MAX_MIXERS - (idx + 1)) * sizeof(MixData));
  memclear(mix, sizeof(MixData));
  mix->destCh = channel;
  mix->srcRaw = channel + 1;
  if (!isSourceAvailable(mix->srcRaw)) {
    if (channel >= adcGetMaxInputs(ADC_INPUT_MAIN)) {
      mix->srcRaw = MIXSRC_FIRST_STICK + channel;
    } else {
      mix->srcRaw = MIXSRC_FIRST_STICK + inputMappingChannelOrder(channel);
    }
    while (!isSourceAvailable(mix->srcRaw)) {
      mix->srcRaw += 1;
    }
  }
  mix->weight = 100;
  mixerTaskStart();

  _nb_mix_lines += 1;
  storageDirty(EE_MODEL);
}

void deleteMix(uint8_t idx)
{
  mixerTaskStop();
  MixData * mix = mixAddress(idx);
  memmove(mix, mix + 1, (MAX_MIXERS - (idx + 1)) * sizeof(MixData));
  memclear(&g_model.mixData[MAX_MIXERS - 1], sizeof(MixData));
  mixerTaskStart();

  _nb_mix_lines -= 1;
  storageDirty(EE_MODEL);
}

void copyMix(uint8_t src, uint8_t dst, uint8_t channel)
{
  mixerTaskStop();
  MixData sourceMix;
  memcpy(&sourceMix, mixAddress(src), sizeof(MixData));
  MixData* mix = mixAddress(dst);
  size_t trailingMixes = MAX_MIXERS - (dst + 1);
  memmove(mix + 1, mix, trailingMixes * sizeof(MixData));
  memcpy(mix, &sourceMix, sizeof(MixData));
  mix->destCh = channel;
  mixerTaskStart();

  _nb_mix_lines += 1;
  storageDirty(EE_MODEL);
}

// Move the mixer line at 'idx' up or down
// by one position and return the new index.
uint8_t moveMix(uint8_t idx, bool up)
{
  MixData * x, * y;
  int8_t tgt_idx = (up ? idx-1 : idx+1);

  x = mixAddress(idx);

  if (tgt_idx < 0) {
    if (x->destCh > 0) {
      x->destCh--;
      storageDirty(EE_MODEL);
    }
    return idx;
  }

  if (tgt_idx == MAX_MIXERS) {
    if (x->destCh < MAX_OUTPUT_CHANNELS - 1) {
      x->destCh++;
      storageDirty(EE_MODEL);
    }
    return idx;
  }

  y = mixAddress(tgt_idx);
  uint8_t destCh = x->destCh;

  // If current target is empty or
  // assigned to a different channel:
  //
  // TODO: check what happens with the mixer
  //       when channel is changed on-the-fly
  //
  if(!y->srcRaw || destCh != y->destCh) {
    if (up) {
      if (destCh > 0) {
	x->destCh--;
	storageDirty(EE_MODEL);
      }
    }
    else {
      if (destCh < MAX_OUTPUT_CHANNELS - 1) {
	x->destCh++;
	storageDirty(EE_MODEL);
      }
    }
    return idx;
  }

  mixerTaskStop();
  memswap(x, y, sizeof(MixData));
  mixerTaskStart();

  storageDirty(EE_MODEL);
  return tgt_idx;
}

static uint8_t _countMixLines()
{
  // search for first blank
  uint8_t i = 0;
  do {
    if (is_memclear(mixAddress(i), sizeof(MixData))) break;
  } while (++i < MAX_MIXERS);

  return i;
}

void updateMixCount()
{
  _nb_mix_lines = _countMixLines();
}
