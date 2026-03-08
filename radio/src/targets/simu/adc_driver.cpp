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

#include "simulib.h"
#include "hal/adc_driver.h"

#include "definitions.h"
#include "myeeprom.h"
#include "edgetx.h"

#include "translations/translations.h"
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

extern uint16_t simuGetAnalog(uint8_t idx);

static bool simu_start_conversion()
{
  int max_input = adcGetInputOffset(ADC_INPUT_VBAT);
  int flex_offset = adcGetInputOffset(ADC_INPUT_FLEX);
  for (int i = 0; i < max_input; i++) {
    int16_t raw = (int16_t)simuGetAnalog(i);
    uint16_t adc_val;
    int flex_idx = i - flex_offset;
#if XPOTS_MULTIPOS_COUNT > 0
    if (flex_idx >= 0 && IS_POT_MULTIPOS(flex_idx)) {
      StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[i];
      int range6POS = 2048;
      if (calib->count != 0) {
        int c1 = calib->steps[calib->count - 1] * 32;
        int c2 = calib->steps[calib->count - 2] * 32;
        range6POS = c1 + (c1 - c2) / 2;
      }
      adc_val = (raw * range6POS / 2048);
    } else
#endif
    {
      adc_val = (raw * 2) + 2048;
    }
    setAnalogValue(i, adc_val);
  }

  // set batteries default voltages
  int i = adcGetInputOffset(ADC_INPUT_VBAT);
  if (i > 0) {
    // +0.5V and prec2
    uint16_t volts = (uint16_t)((g_eeGeneral.vBatWarn > 0 ? g_eeGeneral.vBatWarn
                                                          : BATTERY_WARN) +
                                5) *
                     10;
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
