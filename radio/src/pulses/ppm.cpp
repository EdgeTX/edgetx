/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "edgetx.h"

template<class T>
void setupPulsesPPM(PpmPulsesData<T> * ppmPulsesData, uint8_t channelsStart, int8_t channelsCount, int8_t frameLength)
{
  int16_t PPM_range = g_model.extendedLimits ? (512*LIMIT_EXT_PERCENT/100) * 2 : 512 * 2; // range of 0.7 .. 1.7msec

  // Total frame length = 22.5msec
  // each pulse is 0.7..1.7ms long with a 0.3ms stop tail
  // The pulse ISR is 2mhz that's why everything is multiplied by 2

  uint8_t firstCh = channelsStart;
  uint8_t lastCh = min<uint8_t>(MAX_OUTPUT_CHANNELS, firstCh + 8 + channelsCount);

#if defined(STM32)
  ppmPulsesData->ptr = ppmPulsesData->pulses;
#else
  uint16_t * ptr = ppmPulsesData->pulses;
  ppmPulsesData->ptr = ptr;
#endif

  int32_t rest = 22500u * 2;
  rest += int32_t(frameLength) * 1000;
  for (uint32_t i=firstCh; i<lastCh; i++) {
    int16_t v = limit((int16_t)-PPM_range, channelOutputs[i], (int16_t)PPM_range) + 2*PPM_CH_CENTER(i);
    rest -= v;
#if defined(STM32)
    *ppmPulsesData->ptr++ = v; /* as Pat MacKenzie suggests */
#else
    *ptr++ = v; /* as Pat MacKenzie suggests */
#endif
  }
  rest = limit<int32_t>(9000, rest, 65535); /* avoids that CCR2 is bigger than ARR which would cause reboot */
#if defined(STM32)
  *ppmPulsesData->ptr++ = rest;
  *ppmPulsesData->ptr = 0; // it's needed in case PPM is sent without DMA (we stop when we reach this 0)
#else
  *ptr = rest;
  *(ptr + 1) = 0;
#endif
}

void setupPulsesPPMTrainer()
{
  setupPulsesPPM<trainer_pulse_duration_t>(&trainerPulsesData.ppm, g_model.trainerData.channelsStart, g_model.trainerData.channelsCount, g_model.trainerData.frameLength);
}

#if defined(PCBTARANIS) && defined(INTERNAL_MODULE_PPM)
void setupPulsesPPMInternalModule()
{
  setupPulsesPPM(&intmodulePulsesData.ppm, g_model.moduleData[INTERNAL_MODULE].channelsStart, g_model.moduleData[INTERNAL_MODULE].channelsCount, g_model.moduleData[INTERNAL_MODULE].ppm.frameLength);
}
#endif

void setupPulsesPPMExternalModule()
{
  setupPulsesPPM(&extmodulePulsesData.ppm, g_model.moduleData[EXTERNAL_MODULE].channelsStart, g_model.moduleData[EXTERNAL_MODULE].channelsCount, g_model.moduleData[EXTERNAL_MODULE].ppm.frameLength);
}


