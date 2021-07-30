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

#include "adc_driver.h"
#include "board.h"

// IS_POT_SLIDER_AVAILABLE()
#include "opentx.h"

const etx_hal_adc_driver_t* etx_hal_adc_driver = nullptr;

#if defined(SIMU)
  // not needed
#elif defined(RADIO_T16)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  1,1,1,  -1,1,  -1,1};
#elif defined(RADIO_T18)
  const int8_t adcDirection[NUM_ANALOGS] = { 1,-1,1,-1, -1,1,-1,  -1,1,  -1,1 };
#elif defined(RADIO_TX16S)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  1,1,1,  -1,1,  -1,1};
#elif defined(PCBX10)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  -1,1,-1,   1,1,    1, -1};
#elif defined(PCBX9E)
#if defined(HORUS_STICKS)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  -1,-1,-1,1, -1,1,1,  -1,-1,-1};
#else
  const int8_t adcDirection[NUM_ANALOGS] = {1,1,-1,-1,  -1,-1,-1,1, -1,1,1,  -1,-1,-1};
#endif
  const uint8_t adcMapping[NUM_ANALOGS] = { 0 /*STICK1*/, 1 /*STICK2*/, 2 /*STICK3*/, 3 /*STICK4*/,
                                            11 /*POT1*/, 4 /*POT2*/, 5 /*POT3*/, 6 /*POT4*/,
                                            12 /*SLIDER1*/, 13 /*SLIDER2*/, 7 /*SLIDER3*/, 8 /*SLIDER4*/,
                                            9 /*TX_VOLTAGE*/, 10 /*TX_VBAT*/ };
#elif defined(PCBX9DP)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  1,1,-1,  1,1,  1,  1};
#elif defined(PCBX9D)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  1,1,0,   1,1,  1,  1};
#elif defined(RADIO_TX12)
  const int8_t adcDirection[NUM_ANALOGS] = {-1,1,-1,1,  -1,-1,  1,  1};
#elif defined(PCBX7)
  const int8_t adcDirection[NUM_ANALOGS] = {-1,1,-1,1,  1,1,  1,  1};
#elif defined(PCBX9LITE)
  const int8_t adcDirection[NUM_ANALOGS] = {-1,1,-1,1,  1,1,  1};
#elif defined(PCBXLITE)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,-1,1,  -1,1,  1,  1};
#endif

uint16_t adcValues[NUM_ANALOGS] __DMA;

#if defined(PCBX10)
uint16_t rtcBatteryVoltage;
#endif

bool adcInit(const etx_hal_adc_driver_t* driver)
{
  etx_hal_adc_driver = driver;
  if (etx_hal_adc_driver && etx_hal_adc_driver->init)
  {
    if(!etx_hal_adc_driver->init())
        return false;

    return true;
  }
  return false;
}

static bool adcSingleRead()
{
  if (!etx_hal_adc_driver)
      return false;

  if (etx_hal_adc_driver->start_conversion)
  {
    if (!etx_hal_adc_driver->start_conversion())
        return false;
  }
  else
      return false;

  if (etx_hal_adc_driver->wait_completion)
  {
    etx_hal_adc_driver->wait_completion();
  }
  else
      return false;

  return true;
}

bool adcRead()
{
  uint16_t temp[NUM_ANALOGS] = { 0 };

  for (int i=0; i<4; i++) {
    if (!adcSingleRead())
        return false;
    for (uint8_t x=FIRST_ANALOG_ADC; x<NUM_ANALOGS; x++) {
      uint16_t val = adcValues[x];
#if defined(JITTER_MEASURE)
      if (JITTER_MEASURE_ACTIVE()) {
        rawJitter[x].measure(val);
      }
#endif
      temp[x] += val;
    }
  }

  for (uint8_t x=FIRST_ANALOG_ADC; x<NUM_ANALOGS; x++) {
    adcValues[x] = temp[x] >> 2;
  }

#if NUM_PWMSTICKS > 0
  if (STICKS_PWM_ENABLED()) {
    sticksPwmRead(adcValues);
  }
#endif
  return true;
}

#if defined(PCBX10)
uint16_t getRTCBatteryVoltage()
{
  return (rtcBatteryVoltage * 2 * ADC_VREF_PREC2) / 2048;
}
#else
uint16_t getRTCBatteryVoltage()
{
  return (getAnalogValue(TX_RTC_VOLTAGE) * ADC_VREF_PREC2) / 2048;
}
#endif

#if !defined(SIMU)
uint16_t getAnalogValue(uint8_t index)
{
  if (IS_POT(index) && !IS_POT_SLIDER_AVAILABLE(index)) {
    // Use fixed analog value for non-existing and/or non-connected pots.
    // Non-connected analog inputs will slightly follow the adjacent connected analog inputs,
    // which produces ghost readings on these inputs.
    return 0;
  }
#if defined(PCBX9E)
  index = adcMapping[index];
#endif
  if (adcDirection[index] < 0)
    return 4095 - adcValues[index];
  else
    return adcValues[index];
}
#endif // #if !defined(SIMU)
