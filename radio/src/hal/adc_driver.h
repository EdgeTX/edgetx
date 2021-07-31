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

#pragma once

#include <stdint.h>

// TODO: move this to the targets
#if NUM_PWMSTICKS > 0
  #define FIRST_ANALOG_ADC             (STICKS_PWM_ENABLED() ? NUM_PWMSTICKS : 0)
  #define NUM_ANALOGS_ADC              (STICKS_PWM_ENABLED() ? (NUM_ANALOGS - NUM_PWMSTICKS) : NUM_ANALOGS)
#elif defined(PCBX9E)
  #define FIRST_ANALOG_ADC             0
  #define NUM_ANALOGS_ADC              11
  #define NUM_ANALOGS_ADC_EXT          (NUM_ANALOGS - NUM_ANALOGS_ADC)
#elif defined(PCBNV14) && defined(FLYSKY_HALL_STICKS)
  #define FIRST_ANALOG_ADC             4
  #define NUM_ANALOGS_ADC              10
  #define NUM_ANALOGS_ADC_EXT          2
#else
  #define FIRST_ANALOG_ADC             0
  #define NUM_ANALOGS_ADC              NUM_ANALOGS
#endif

// TODO: get rid of this special case
#if defined(PCBX10)
extern uint16_t rtcBatteryVoltage;
#endif

struct etx_hal_adc_driver_t {
  bool (*init)();
  bool (*start_conversion)();
  void (*wait_completion)();  
};

bool adcInit(const etx_hal_adc_driver_t* driver);
// void adcDeInit();

// Declare adcRead() weak so it can be re-declared
bool adcRead() __attribute__((weak));

uint16_t getRTCBatteryVoltage();
uint16_t getAnalogValue(uint8_t index);
