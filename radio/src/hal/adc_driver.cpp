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
#elif defined(RADIO_TX12MK2)
  const int8_t adcDirection[NUM_ANALOGS] = {-1,1,-1,1,  1,-1};
#elif defined(RADIO_ZORRO)
  const int8_t adcDirection[NUM_ANALOGS] = {-1, 1, 1, -1, -1, 1, 1, 1};
#elif defined(RADIO_TPRO)
  const int8_t adcDirection[NUM_ANALOGS] = {1,-1,1,-1,  1,1,  1,  1};
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
    return ADCMAXVALUE - adcValues[index];
  else
    return adcValues[index];
}

// used by diaganas
uint32_t s_anaFilt[NUM_ANALOGS];

#define ANALOG_MULTIPLIER       (1<<ANALOG_SCALE)
#define ANA_FILT(chan)          (s_anaFilt[chan] / (JITTER_ALPHA * ANALOG_MULTIPLIER))
#if (JITTER_ALPHA * ANALOG_MULTIPLIER > 32)
  #error "JITTER_FILTER_STRENGTH and ANALOG_SCALE are too big, their summ should be <= 5 !!!"
#endif

uint16_t anaIn(uint8_t chan)
{
  return ANA_FILT(chan);
}

#if defined(JITTER_MEASURE)
JitterMeter<uint16_t> rawJitter[NUM_ANALOGS];
JitterMeter<uint16_t> avgJitter[NUM_ANALOGS];
tmr10ms_t jitterResetTime = 0;
#endif

uint16_t getBatteryVoltage()
{
  // using filtered ADC value on purpose
  int32_t instant_vbat = anaIn(TX_VOLTAGE);

#if defined(BATT_SCALE)
  instant_vbat =
      (instant_vbat * BATT_SCALE * (128 + g_eeGeneral.txVoltageCalibration)) /
      BATTERY_DIVIDER;
  // add voltage drop because of the diode TODO check if this is needed, but
  // removal will break existing calibrations!
  instant_vbat += VOLTAGE_DROP;
  return (uint16_t)instant_vbat;
#else
  return (uint16_t)((instant_vbat * (1000 + g_eeGeneral.txVoltageCalibration)) /
                    BATTERY_DIVIDER);
#endif
}

extern uint16_t get_flysky_hall_adc_value(uint8_t ch);

void getADC()
{
#if defined(JITTER_MEASURE)
  if (JITTER_MEASURE_ACTIVE() && jitterResetTime < get_tmr10ms()) {
    // reset jitter measurement every second
    for (uint32_t x=0; x<NUM_ANALOGS; x++) {
      rawJitter[x].reset();
      avgJitter[x].reset();
    }
    jitterResetTime = get_tmr10ms() + 100;  //every second
  }
#endif

  DEBUG_TIMER_START(debugTimerAdcRead);
  if (!adcRead())
      TRACE("adcRead failed");
  DEBUG_TIMER_STOP(debugTimerAdcRead);

  for (uint8_t x=0; x<NUM_ANALOGS; x++) {
    uint32_t v;
#if defined(RADIO_FAMILY_T16) || defined(PCBNV14)
    if (globalData.flyskygimbals)
    {
        if (x < 4) {
          v = get_flysky_hall_adc_value(x) >> (1 - ANALOG_SCALE);
        } else {
        v = getAnalogValue(x) >> (1 - ANALOG_SCALE);
        }
    }
    else
#endif
    {
        v = getAnalogValue(x) >> (1 - ANALOG_SCALE);
    }

    // Jitter filter:
    //    * pass trough any big change directly
    //    * for small change use Modified moving average (MMA) filter
    //
    // Explanation:
    //
    // Normal MMA filter has this formula:
    //            <out> = ((ALPHA-1)*<out> + <in>)/ALPHA
    //
    // If calculation is done this way with integer arithmetics, then any small change in
    // input signal is lost. One way to combat that, is to rearrange the formula somewhat,
    // to store a more precise (larger) number between iterations. The basic idea is to
    // store undivided value between iterations. Therefore an new variable <filtered> is
    // used. The new formula becomes:
    //           <filtered> = <filtered> - <filtered>/ALPHA + <in>
    //           <out> = <filtered>/ALPHA  (use only when out is needed)
    //
    // The above formula with a maximum allowed ALPHA value (we are limited by
    // the 16 bit s_anaFilt[]) was tested on the radio. The resulting signal still had
    // some jitter (a value of 1 was observed). The jitter might be bigger on other
    // radios.
    //
    // So another idea is to use larger input values for filtering. So instead of using
    // input in a range from 0 to 2047, we use twice larger number (temp[x] is divided less)
    //
    // This also means that ALPHA must be lowered (remember 16 bit limit), but test results
    // have proved that this kind of filtering gives better results. So the recommended values
    // for filter are:
    //     JITTER_FILTER_STRENGTH  4
    //     ANALOG_SCALE            1
    //
    // Variables mapping:
    //   * <in> = v
    //   * <out> = s_anaFilt[x]
    uint32_t previous = s_anaFilt[x] / JITTER_ALPHA;
    uint32_t diff = (v > previous) ? (v - previous) : (previous - v);

    // Combine ADC jitter filter setting form radio and model.
    // Model can override (on or off) or use setting from radio setup.
    // Model setting is active when 1, radio setting is active when 0
    uint8_t useJitterFilter = 0;
    if (g_model.jitterFilter == OVERRIDE_GLOBAL) {
       // Use radio setting - which is inverted
      useJitterFilter = !g_eeGeneral.noJitterFilter;
    } else {
      // Enable if value is "On", disable if "Off"
      useJitterFilter = (g_model.jitterFilter == OVERRIDE_ON)?1:0;
    }

    if (useJitterFilter && diff < (10*ANALOG_MULTIPLIER)) {
      // apply jitter filter
      s_anaFilt[x] = (s_anaFilt[x] - previous) + v;
    }
    else {
      // use unfiltered value
      s_anaFilt[x] = v * JITTER_ALPHA;
    }

#if defined(JITTER_MEASURE)
    if (JITTER_MEASURE_ACTIVE()) {
      avgJitter[x].measure(ANA_FILT(x));
    }
#endif

    #define ANAFILT_MAX    (2 * RESX * JITTER_ALPHA * ANALOG_MULTIPLIER - 1)
    StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[x];
    if (IS_POT_MULTIPOS(x) && IS_MULTIPOS_CALIBRATED(calib)) {
      // TODO: consider adding another low pass filter to eliminate multipos switching glitches
      uint8_t vShifted = ANA_FILT(x) >> 4;
      s_anaFilt[x] = ANAFILT_MAX;
      for (uint32_t i=0; i<calib->count; i++) {
        if (vShifted < calib->steps[i]) {
          s_anaFilt[x] = (i * ANAFILT_MAX) / calib->count;
          break;
        }
      }
    }
  }
}

#endif // #if !defined(SIMU)
