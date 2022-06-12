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

#include <climits>

#include "opentx.h"

// Minimum space after the last PPM pulse in us
#define PPM_SAFE_MARGIN 3000 // 3ms

template <class T>
uint16_t setupPulsesPPM(PpmPulsesData<T>* ppmPulsesData, uint8_t channelsStart,
                        int8_t channelsCount)
{
  uint16_t total = 0;
  int16_t PPM_range = g_model.extendedLimits ?
    // range of 0.7 .. 1.7msec
    (512*LIMIT_EXT_PERCENT/100) * 2 : 512 * 2;

  // Total frame length = 22.5msec
  // each pulse is 0.7..1.7ms long with a 0.3ms stop tail
  // The pulse ISR is 2mhz that's why everything is multiplied by 2

  uint8_t firstCh = channelsStart;
  uint8_t lastCh =
      min<uint8_t>(MAX_OUTPUT_CHANNELS, firstCh + 8 + channelsCount);

  ppmPulsesData->ptr = ppmPulsesData->pulses;

  for (uint32_t i = firstCh; i < lastCh; i++) {
    int16_t v =
        limit((int16_t)-PPM_range, channelOutputs[i], (int16_t)PPM_range) +
        2 * PPM_CH_CENTER(i);
    *ppmPulsesData->ptr++ = v;
    total += v;
  }

  return total;
}

void setupPulsesPPMTrainer()
{
  uint16_t total = setupPulsesPPM<trainer_pulse_duration_t>(
      &trainerPulsesData.ppm, g_model.trainerData.channelsStart,
      g_model.trainerData.channelsCount);

  uint32_t rest = PPM_TRAINER_PERIOD_HALF_US();
  if ((uint32_t)total < rest + PPM_SAFE_MARGIN * 2)
    rest -= total;
  else
    rest = PPM_SAFE_MARGIN * 2;

  // restrict to 16 bit max
  if (rest >= USHRT_MAX - 1)
    rest = USHRT_MAX - 1;
    
  *trainerPulsesData.ppm.ptr++ = (uint16_t)rest;

  // stop mark so that IRQ-based sending
  // knows when to stop
  *trainerPulsesData.ppm.ptr = 0;
}

static void setupPulsesPPMModule(uint8_t module)
{
  PpmPulsesData<pulse_duration_t>* data = nullptr;
#if defined(PCBTARANIS) && defined(INTERNAL_MODULE_PPM)
  if (module == INTERNAL_MODULE) {
    data = &intmodulePulsesData.ppm;
  } else
#endif
  {
    data = &extmodulePulsesData.ppm;
  }

  setupPulsesPPM(&extmodulePulsesData.ppm,
                 g_model.moduleData[module].channelsStart,
                 g_model.moduleData[module].channelsCount);

  // Set the final period to 1ms after which the
  // PPM will be switched OFF
  *data->ptr++ = PPM_SAFE_MARGIN * 2;
}

#if defined(PCBTARANIS) && defined(INTERNAL_MODULE_PPM)
void setupPulsesPPMInternalModule()
{
  setupPulsesPPMModule(INTERNAL_MODULE);
}
#endif

void setupPulsesPPMExternalModule()
{
  setupPulsesPPMModule(EXTERNAL_MODULE);
}
