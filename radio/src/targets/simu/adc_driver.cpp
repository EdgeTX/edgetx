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
#include "edgetx.h"

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

  // set batteries default voltages
  int i = adcGetInputOffset(ADC_INPUT_VBAT);

  if (i > 0) {
    // calculate default voltage on 1st call
    uint32_t adc = (simu_get_analog(i) - 2048) * 1000 / 2000;
    // TRACE("raw ana: %d adj ana: %d", simu_get_analog(i), vbat);
    // just in case the voltage has not been initialised in sim initialisation/startup
    // these formulae must mirror OpenTxSimulator::voltageToAdc
    if (adc == 0) {
      uint32_t volts = (uint32_t)((g_eeGeneral.vBatWarn > 0 ? g_eeGeneral.vBatWarn : BATTERY_WARN) + 5) * 10; // +0.5V and prec2
#if defined(VBAT_MOSFET_DROP)
      // TRACE("volts: %d r1: %d r2: %d drop: %d vref: %d calib: %d", volts, VBAT_DIV_R1, VBAT_DIV_R2, VBAT_MOSFET_DROP, ADC_VREF_PREC2, g_eeGeneral.txVoltageCalibration);
      adc = (volts - VBAT_MOSFET_DROP) * (2 * RESX * 1000) / ADC_VREF_PREC2 / (((1000 + g_eeGeneral.txVoltageCalibration) * (VBAT_DIV_R2 + VBAT_DIV_R1)) / VBAT_DIV_R1);
#elif defined(BATT_SCALE)
      // TRACE("volts: %d div: %d drop: %d scale: %d calib: %d", volts, BATTERY_DIVIDER, VOLTAGE_DROP, BATT_SCALE, g_eeGeneral.txVoltageCalibration);
      adc = (volts - VOLTAGE_DROP) * BATTERY_DIVIDER / (128 + g_eeGeneral.txVoltageCalibration) / BATT_SCALE;
#elif defined(VOLTAGE_DROP)
      // TRACE("volts: %d div: %d drop: %d", volts, BATTERY_DIVIDER, VOLTAGE_DROP);
      adc = (volts - VOLTAGE_DROP) * BATTERY_DIVIDER / (1000 + g_eeGeneral.txVoltageCalibration);
#else
      // TRACE("volts: %d div: %d calib: %d", volts, BATTERY_DIVIDER, g_eeGeneral.txVoltageCalibration);
      adc = volts * BATTERY_DIVIDER / (1000 + g_eeGeneral.txVoltageCalibration);
#endif
      // TRACE("calc adc: %d", adc);
      adc = adc * 2; // div by 2 in firmware filtered adc calcs
    }

    setAnalogValue(i, adc);
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
