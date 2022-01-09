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
#elif defined(PCBPL18)
  #define FIRST_ANALOG_ADC_FS          4
  #define NUM_ANALOGS_ADC_FS           9
  #define NUM_ANALOGS_ADC_EXT          2
  #define FIRST_ANALOG_ADC             0
  #define FIRST_ANALOG_ADC_EXT         13
  #define NUM_ANALOGS_ADC              NUM_ANALOGS
#else
  #define FIRST_ANALOG_ADC_FS          4
  #define NUM_ANALOGS_ADC_FS           7
  #define NUM_ANALOGS_ADC_EXT          1
  #define FIRST_ANALOG_ADC             0
  #define NUM_ANALOGS_ADC              NUM_ANALOGS
#endif

#define ADCMAXVALUE                    4095

// TODO: get rid of this special case
#if defined(PCBX10) || defined(PCBX12S) || defined(PCBPL18)
extern uint16_t rtcBatteryVoltage;
#endif

struct etx_hal_adc_driver_t {
  bool (*init)();
  bool (*start_conversion)();
  void (*wait_completion)();  
};

bool adcInit(const etx_hal_adc_driver_t* driver);
// void adcDeInit();

bool     adcRead();
uint16_t getBatteryVoltage();
uint16_t getRTCBatteryVoltage();
uint16_t getAnalogValue(uint8_t index);

#define JITTER_FILTER_STRENGTH  4         // tune this value, bigger value - more filtering (range: 1-5) (see explanation below)
#define ANALOG_SCALE            1         // tune this value, bigger value - more filtering (range: 0-1) (see explanation below)
#define JITTER_ALPHA            (1<<JITTER_FILTER_STRENGTH)

#if defined(JITTER_MEASURE)
extern JitterMeter<uint16_t> rawJitter[NUM_ANALOGS];
extern JitterMeter<uint16_t> avgJitter[NUM_ANALOGS];
#if defined(PCBHORUS) || defined(PCBTARANIS)
  #define JITTER_MEASURE_ACTIVE()   (menuHandlers[menuLevel] == menuRadioDiagAnalogs)
#elif defined(CLI)
  #define JITTER_MEASURE_ACTIVE()   (1)
#else
  #define JITTER_MEASURE_ACTIVE()   (0)
#endif
#endif

void getADC();
uint16_t anaIn(uint8_t chan);
