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

#include "myeeprom.h"

#include "board.h"
#include "edgetx.h"
#include "hal_adc_inputs.inc"

void enableVBatBridge(){}
void disableVBatBridge(){}
bool isVBatBridgeEnabled(){ return false; }

uint16_t getLuxSensorValue() { return 1024; }

uint16_t getBatteryVoltage()
{
  if (adcGetMaxInputs(ADC_INPUT_VBAT) < 1) return 0;
  return anaIn(adcGetInputOffset(ADC_INPUT_VBAT));
}

uint16_t getRTCBatteryVoltage()
{
  return 300;
}

extern uint16_t simu_get_analog(uint8_t idx);

static bool simu_start_conversion()
{
  int max_input = adcGetInputOffset(ADC_INPUT_VBAT);
  for (int i = 0; i < max_input; i++) {
    setAnalogValue(i, simu_get_analog(i));
  }

  // set batteries default voltages
  int i = adcGetInputOffset(ADC_INPUT_VBAT);
  if (i > 0) {
    uint16_t volts = (uint16_t)((g_eeGeneral.vBatWarn > 0 ? g_eeGeneral.vBatWarn : BATTERY_WARN) + 5) * 10; // +0.5V and prec2
    setAnalogValue(i, volts * 2);
  }

  return true;
}

extern const etx_hal_adc_driver_t simu_adc_driver;

const etx_hal_adc_driver_t simu_adc_driver = {
  .inputs = _hal_inputs,
  .default_pots_cfg = _pot_default_config,
  .init = nullptr,
  .start_conversion = simu_start_conversion,
  .wait_completion = nullptr,
};
