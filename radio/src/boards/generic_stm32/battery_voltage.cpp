/*
 * Copyright (C) EdgeTx
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
#include "board.h"

#include "edgetx.h"

#if defined(STM32F407xx) || defined(STM32F205xx)
  // internal bridge measure Vbat/2
  #define VBAT_RTC_DIV 2
#else
  // internal bridge measure Vbat/4
  #define VBAT_RTC_DIV 4
#endif

#define __weak __attribute__((weak))

static uint16_t scale_rtc_voltage(uint16_t v)
{
  return (v * ADC_VREF_PREC2 * VBAT_RTC_DIV) / ADC_MAX_FILTERED;
}

uint16_t getRTCBatteryVoltage()
{
  if (adcGetMaxInputs(ADC_INPUT_RTC_BAT) < 1) return 0;

  uint16_t filtered = anaIn(adcGetInputOffset(ADC_INPUT_RTC_BAT));
  return scale_rtc_voltage(filtered);
}

#if (defined(VBAT_DIV_R1) && defined(VBAT_DIV_R2)) || \
    defined(BATTERY_DIVIDER) || defined(BATT_SCALE)

#define VREF_FLOAT ((float)ADC_VREF_PREC2 / 100.0)

#if defined(VBAT_DIV_R1) && defined(VBAT_DIV_R2)
  // defined as a voltage divider
  // Vbat --- R1 --+-- ADC
  //               |
  //               +-- R2 --- GND
  // 
  #define VOLTAGE_DIVIDER(r1, r2) (((float)r1 + (float)r2) / (float)r2)

  #define VBAT_DIVIDER VOLTAGE_DIVIDER(VBAT_DIV_R1, VBAT_DIV_R2)

  // Scale of Vbat calibration setting
  #define VBAT_CAL 1000.0

#elif defined(BATT_SCALE)
  // defined as a simple ratio (old style: mostly taranis targets)
  #define VBAT_DIVIDER                               \
    (((float)BATT_SCALE * 128.0 * (float)ADC_MAX_FILTERED) / \
     ((float)BATTERY_DIVIDER * 100.0 * VREF_FLOAT))

// Scale of Vbat calibration setting
#define VBAT_CAL 128.0

#elif defined(BATTERY_DIVIDER)
  // defined a a simple factor (old style: mostly color targets)
  #define VBAT_DIVIDER \
    (((float)ADC_MAX_FILTERED * 10.0) / ((float)BATTERY_DIVIDER * VREF_FLOAT))

  // Scale of Vbat calibration setting
  #define VBAT_CAL 1000.0

#endif

// Fixed offset in hundredth of volt (*10 mV)
#if defined(VBAT_MOSFET_DROP)
  #define VBAT_OFFSET VBAT_MOSFET_DROP
#elif defined(VOLTAGE_DROP)
  #define VBAT_OFFSET VOLTAGE_DROP
#else
  #define VBAT_OFFSET 0
#endif

__weak uint16_t getBatteryVoltage()
{
  // using filtered ADC value on purpose
  if (adcGetMaxInputs(ADC_INPUT_VBAT) < 1) return 0;

  float vbat = anaIn(adcGetInputOffset(ADC_INPUT_VBAT));

  // apply calibration
  vbat = vbat * (VBAT_CAL + g_eeGeneral.txVoltageCalibration) / VBAT_CAL;

  // apply voltage divider and ADC scale
  vbat *= VBAT_DIVIDER * (VREF_FLOAT / (float)ADC_MAX_FILTERED);

  // return hundredth of volt (*10 mV)
  return (uint16_t)(100.0 * vbat) + VBAT_OFFSET;
}

#endif
