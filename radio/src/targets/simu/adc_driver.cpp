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

#include "hal/adc_driver.h"
#include "definitions.h"
#include "dataconstants.h"

#include "hw_inputs.inc"

void enableVBatBridge(){}
void disableVBatBridge(){}
bool isVBatBridgeEnabled(){ return false; }

extern uint16_t simu_get_analog(uint8_t idx);

static bool simu_start_conversion()
{
  for (int i=0; i<MAX_ANALOG_INPUTS; i++)
    setAnalogValue(i, simu_get_analog(i));

  return true;
}

extern const etx_hal_adc_driver_t simu_adc_driver;

const etx_hal_adc_driver_t simu_adc_driver = {
  .init = nullptr,
  .start_conversion = simu_start_conversion,
  .wait_completion = nullptr,
};

const char* adcGetStickName(uint8_t idx)
{
  if (idx >= DIM(_stick_inputs)) return "";
  return _stick_inputs[idx];
}

const char* adcGetPotName(uint8_t idx)
{
  if (idx >= DIM(_pot_inputs)) return "";
  return _pot_inputs[idx];
}

uint8_t adcGetMaxSticks()
{
  return DIM(_stick_inputs);
}

uint8_t adcGetMaxPots()
{
  return DIM(_pot_inputs);
}
