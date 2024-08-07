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

#include "edgetx.h"

void varioWakeup()
{
  if (isFunctionActive(FUNCTION_VARIO)) {
    int varioFreq, varioDuration, varioPause=0;
    uint8_t varioFlags;

    int verticalSpeed = 0;
    if (g_model.varioData.source) {
      uint8_t item = g_model.varioData.source-1;
      if (item < MAX_TELEMETRY_SENSORS) {
        verticalSpeed = telemetryItems[item].value * g_model.telemetrySensors[item].getPrecMultiplier();
      }
    }

    int varioCenterMin = (int)g_model.varioData.centerMin * 10 - 50;
    int varioCenterMax = (int)g_model.varioData.centerMax * 10 + 50;
    int varioMax = (10+(int)g_model.varioData.max) * 100;
    int varioMin = (-10+(int)g_model.varioData.min) * 100;

    if (verticalSpeed > varioMax)
      verticalSpeed = varioMax;
    else if (verticalSpeed < varioMin)
      verticalSpeed = varioMin;

    if (verticalSpeed <= varioCenterMin) {
      varioFreq = VARIO_FREQUENCY_ZERO + (g_eeGeneral.varioPitch*10) - (((VARIO_FREQUENCY_ZERO+(g_eeGeneral.varioPitch*10)-((VARIO_FREQUENCY_ZERO + (g_eeGeneral.varioPitch*10))/2)) * (verticalSpeed-varioCenterMin)) / varioMin);
      varioDuration = 80; // continuous beep: we will enter again here before the tone ends
      varioFlags = PLAY_BACKGROUND|PLAY_NOW;
    }
    else if (verticalSpeed >= varioCenterMax || !g_model.varioData.centerSilent) {
      varioFreq = VARIO_FREQUENCY_ZERO + (g_eeGeneral.varioPitch*10) + (((VARIO_FREQUENCY_RANGE+(g_eeGeneral.varioRange*10)) * (verticalSpeed-varioCenterMin)) / varioMax);
      int varioPeriod = VARIO_REPEAT_MAX + ((VARIO_REPEAT_ZERO+(g_eeGeneral.varioRepeat*10)-VARIO_REPEAT_MAX) * (varioMax-verticalSpeed) * (varioMax-verticalSpeed)) / ((varioMax-varioCenterMin) * (varioMax-varioCenterMin));
      if (verticalSpeed >= varioCenterMax || varioCenterMin == varioCenterMax)
        varioDuration = varioPeriod / 5;
      else
        varioDuration = varioPeriod * (85 - (((verticalSpeed-varioCenterMin) * 25) / (varioCenterMax-varioCenterMin))) / 100;
      varioPause = varioPeriod - varioDuration;
      varioFlags = PLAY_BACKGROUND;
    }
    else {
      return;
    }

    AUDIO_VARIO(varioFreq, varioDuration, varioPause, varioFlags);
  }
}

