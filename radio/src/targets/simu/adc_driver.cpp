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
#include "translations.h"

#include "hal_adc_inputs.inc"
#include "board.h"

void enableVBatBridge(){}
void disableVBatBridge(){}
bool isVBatBridgeEnabled(){ return false; }

extern uint16_t simu_get_analog(uint8_t idx);

static bool simu_start_conversion()
{
  int max_input = adcGetInputOffset(ADC_INPUT_VBAT);
  for (int i = 0; i < max_input; i++) {
    setAnalogValue(i, simu_get_analog(i));
  }

  // set VBAT / RTC_BAT
  if (adcGetMaxInputs(ADC_INPUT_VBAT) > 0) {
#if defined(VBAT_MOSFET_DROP)
    uint32_t vbat = (2 * (BATTERY_MAX + BATTERY_MIN) * (VBAT_DIV_R2 + VBAT_DIV_R1)) / VBAT_DIV_R1;
#elif defined(BATT_SCALE)
    uint32_t vbat = (BATTERY_MAX + BATTERY_MIN) * 5; // * 10 / 2
    vbat = ((vbat - VOLTAGE_DROP) * BATTERY_DIVIDER) / (BATT_SCALE * 128);
#else
    uint32_t vbat = (BATTERY_MAX + BATTERY_MIN) * 5; // * 10 / 2
    vbat = (vbat * BATTERY_DIVIDER) / 1000;
#endif
    setAnalogValue(adcGetInputOffset(ADC_INPUT_VBAT), vbat * 2);
  }

  if (adcGetMaxInputs(ADC_INPUT_RTC_BAT) > 0) {
    uint32_t rtc_bat = (300 * (2048 >> ANALOG_SCALE)) / ADC_VREF_PREC2;
    setAnalogValue(adcGetInputOffset(ADC_INPUT_RTC_BAT), rtc_bat * 2);
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
