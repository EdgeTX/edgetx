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
#include "luminosity_sensor.h"
#include "hal/adc_driver.h"

static uint32_t luxEMA = 0;
static bool luxInitialized = false;

bool getPeriodicLuxSensorValue()
{
  if (adcGetMaxInputs(ADC_INPUT_LUX) < 1) return false;

  uint16_t newValue = anaIn(adcGetInputOffset(ADC_INPUT_LUX));

  if (!luxInitialized) {
    // Initialize EMA with first reading
    luxEMA = (uint32_t)newValue << 8;  // Scale up for precision
    luxInitialized = true;
  } else {
    // EMA formula with alpha = 26/256 ≈ 0.102
    int32_t diff = ((int32_t)newValue << 8) - (int32_t)luxEMA;
    luxEMA = luxEMA + ((diff * 26) >> 8);
  }

  return true;
}

uint16_t getLuxSensorValue()
{
  return luxEMA >> 8; // Scale back down
}