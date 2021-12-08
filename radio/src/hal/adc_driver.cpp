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
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  1,1,1,   -1,1,1,1,  -1,1 };
#elif defined(RADIO_T18)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1, -1,1,-1,  -1,1,1,1,  -1,1 };
#elif defined(RADIO_TX16S)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  1,1,1,   -1,1,1,1,  -1,1 };
#elif defined(PCBX10)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  -1,1,-1,  1,1,1,1,   1,-1 };
#elif defined(PCBX9E)
  const int8_t adcDirection[NUM_ANALOGS] = {
#if defined(HORUS_STICKS)
    1,-1,1,-1,
#else
    1,1,-1,-1,
#endif // HORUS_STICKS
    // other analogs are the same
    -1,-1,-1,1, -1,1,1,-1, -1,-1 };

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
#elif defined(RADIO_ZORRO)
  const int8_t adcDirection[NUM_ANALOGS] = {-1, 1, 1, -1, -1, 1, 1, 1};
#elif defined(PCBX7)
  const int8_t adcDirection[NUM_ANALOGS] = {-1,1,-1,1,  1,1,  1,  1};
#elif defined(PCBX9LITE)
  const int8_t adcDirection[NUM_ANALOGS] = {-1,1,-1,1,  1,1,  1};
#elif defined(PCBXLITE)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,-1,1,  -1,1,  1,  1};
#elif defined(PCBNV14)
  const uint8_t adcMapping[NUM_ANALOGS] = { 0 /*STICK1*/, 1 /*STICK2*/, 2 /*STICK3*/, 3 /*STICK4*/,
                                            4 /*POT1*/, 5 /*POT2*/, 6 /*SWA*/, 14 /*SWB*/,
                                            7 /*SWC*/,  15 /*SWD*/, 8 /*SWE*/, 9 /*SWF*/,
                                            11/*SWG*/,  10/*SWH*/,
                                            12 /*TX_VOLTAGE*/, 13 /* TX_VBAT */ };

  const int8_t adcDirection[NUM_ANALOGS] = { 0 /*STICK1*/, 0 /*STICK2*/, 0 /*STICK3*/, 0 /*STICK4*/,
                                            -1 /*POT1*/, 0 /*POT2*/, 0 /*SWA*/,  0 /*SWC*/,
                                             0 /*SWE*/, -1 /*SWF*/,  0 /*SWG*/, -1 /*SWH*/,
                                             0 /*TX_VOLTAGE*/, 0 /*TX_VBAT*/,
                                             0 /*SWB*/, 0 /*SWD*/};

#elif defined(PCBX12S)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  -1,1,-1,  -1,-1,  -1,1, 0,0,0};
#else
  #error "ADC driver does not suppport this target"
#endif

uint16_t adcValues[NUM_ANALOGS] __DMA;

#if defined(PCBX10) || defined(PCBX12S)
uint16_t rtcBatteryVoltage;
#endif

bool adcInit(const etx_hal_adc_driver_t* driver)
{
  etx_hal_adc_driver = driver;
  if (!etx_hal_adc_driver)
    return false;

  // If there is an init function
  // it should succeed
  if (etx_hal_adc_driver->init &&
      !etx_hal_adc_driver->init()) {
    return false;
  }

  return true;
}

static bool adcSingleRead()
{
  if (!etx_hal_adc_driver)
      return false;

  if (etx_hal_adc_driver->start_conversion &&
      !etx_hal_adc_driver->start_conversion())
    return false;

  if (etx_hal_adc_driver->wait_completion)
    etx_hal_adc_driver->wait_completion();

  return true;
}

// Declare adcRead() weak so it can be re-declared
#pragma weak adcRead
bool adcRead()
{
  uint16_t temp[NUM_ANALOGS] = { 0 };

  uint8_t first_analog_adc;
#if defined(RADIO_FAMILY_T16) || defined(PCBNV14)
    if (globalData.flyskygimbals)
    {
        first_analog_adc = FIRST_ANALOG_ADC_FS;
    } else
    {
        first_analog_adc = FIRST_ANALOG_ADC;
    }
#else
    first_analog_adc = FIRST_ANALOG_ADC;
#endif


  for (int i=0; i<4; i++) {
    if (!adcSingleRead())
        return false;
    for (uint8_t x=first_analog_adc; x<NUM_ANALOGS; x++) {
      uint16_t val = adcValues[x];
#if defined(JITTER_MEASURE)
      if (JITTER_MEASURE_ACTIVE()) {
        rawJitter[x].measure(val);
      }
#endif
      temp[x] += val;
    }
  }

  for (uint8_t x=first_analog_adc; x<NUM_ANALOGS; x++) {
    adcValues[x] = temp[x] >> 2;
  }

#if NUM_PWMSTICKS > 0
  if (STICKS_PWM_ENABLED()) {
    sticksPwmRead(adcValues);
  }
#endif
  return true;
}

#if !defined(SIMU)
uint16_t getRTCBatteryVoltage()
{
#if defined(HAS_TX_RTC_VOLTAGE)
  return (getAnalogValue(TX_RTC_VOLTAGE) * ADC_VREF_PREC2) / 2048;
#elif defined(PCBX10) || defined(PCBX12S)
  return (rtcBatteryVoltage * 2 * ADC_VREF_PREC2) / 2048;
#elif defined(PCBNV14)
  #warning "TODO RTC voltage"
  return 330;
#else
  #warning "RTC battery not supported on this target"
  return 0;
#endif
}

uint16_t getAnalogValue(uint8_t index)
{
  if (IS_POT(index) && !IS_POT_SLIDER_AVAILABLE(index)) {
    // Use fixed analog value for non-existing and/or non-connected pots.
    // Non-connected analog inputs will slightly follow the adjacent connected analog inputs,
    // which produces ghost readings on these inputs.
    return 0;
  }
#if defined(PCBX9E) || defined(PCBNV14)
  index = adcMapping[index];
#endif
  if (adcDirection[index] < 0)
    return 4095 - adcValues[index];
  else
    return adcValues[index];
}
#endif // #if !defined(SIMU)
