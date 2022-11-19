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


const etx_hal_adc_driver_t* _hal_adc_driver = nullptr;

uint16_t adcValues[NUM_ANALOGS] __DMA;

bool adcInit(const etx_hal_adc_driver_t* driver)
{
  // If there is an init function, it MUST succeed
  if (driver && (!driver->init || driver->init())) {
    _hal_adc_driver = driver;
    return true;
  }

  _hal_adc_driver = nullptr;
  return false;
}

static bool adcSingleRead()
{
  if (!_hal_adc_driver)
    return false;

  if (_hal_adc_driver->start_conversion &&
      !_hal_adc_driver->start_conversion())
    return false;

  if (_hal_adc_driver->wait_completion)
    _hal_adc_driver->wait_completion();

  return true;
}

bool adcRead()
{
  adcSingleRead();
  
  // TODO: this hack needs to go away...
  if (isVBatBridgeEnabled()) {
    disableVBatBridge();
  }

#if NUM_PWMSTICKS > 0
  if (STICKS_PWM_ENABLED()) {
    sticksPwmRead(adcValues);
  }
#endif

  return true;
}

#define XPOT_DELTA 10
#define XPOT_DELAY 10 /* cycles */

void adcCalibMinMax()
{
  // get low and high vals for sticks and pots
  for (uint8_t i = 0; i < MAX_ANALOG_INPUTS; i++) {

    int16_t vt = anaIn(i);
    auto& calib = reusableBuffer.calib.inputs[i];
    calib.input.loVal = min(vt, calib.input.loVal);
    calib.input.hiVal = max(vt, calib.input.hiVal);

    if (i >= MAX_STICKS) {
      uint8_t idx = i - MAX_STICKS;
      if (IS_POT_WITHOUT_DETENT(idx)) {
        calib.input.midVal = (calib.input.hiVal + calib.input.loVal) / 2;
      } else if (IS_POT_MULTIPOS(idx)) {
        auto& xpot = calib.xpot;
        int count = xpot.stepsCount;
        if (count <= XPOTS_MULTIPOS_COUNT) {
          // use raw analog value for multipos calibraton,
          // anaIn() already has multipos decoded value
          vt = getAnalogValue(i) >> 1;
          if (xpot.lastCount == 0 || vt < xpot.lastPosition - XPOT_DELTA ||
              vt > xpot.lastPosition + XPOT_DELTA) {
            xpot.lastPosition = vt;
            xpot.lastCount = 1;
          } else {
            if (xpot.lastCount < 255) xpot.lastCount++;
          }
          if (xpot.lastCount == XPOT_DELAY) {
            int16_t position = xpot.lastPosition;
            bool found = false;
            for (int j = 0; j < count; j++) {
              int16_t step = xpot.steps[j];
              if (position >= step - XPOT_DELTA &&
                  position <= step + XPOT_DELTA) {
                found = true;
                break;
              }
            }
            if (!found) {
              if (count < XPOTS_MULTIPOS_COUNT) {
                xpot.steps[count] = position;
              }
              xpot.stepsCount += 1;
            }
          }
        }
      }
    }
  }
}

void adcCalibSetMidPoint()
{
  for (uint8_t i = 0; i < MAX_ANALOG_INPUTS; i++) {
    auto& calib = reusableBuffer.calib.inputs[i];
    if (i < MAX_STICKS || !IS_POT_MULTIPOS(i - MAX_STICKS)) {
      calib.input.loVal = 15000;
      calib.input.hiVal = -15000;
      calib.input.midVal = getAnalogValue(i) >> 1;
    } else {
      calib.xpot.stepsCount = 0;
      calib.xpot.lastCount = 0;
    }
  }
}

void adcCalibSetMinMax()
{
  for (uint8_t i = 0; i < MAX_ANALOG_INPUTS; i++) {
    auto& calib = reusableBuffer.calib.inputs[i];        
    if (abs(calib.input.loVal - calib.input.hiVal) > 50) {
      g_eeGeneral.calib[i].mid = calib.input.midVal;
      int16_t v = calib.input.midVal - calib.input.loVal;
      g_eeGeneral.calib[i].spanNeg = v - v / STICK_TOLERANCE;
      v = calib.input.hiVal - calib.input.midVal;
      g_eeGeneral.calib[i].spanPos = v - v / STICK_TOLERANCE;
    }
  }
}

void adcCalibSetXPot()
{
  for (uint8_t i = MAX_STICKS; i < MAX_ANALOG_INPUTS; i++) {
    int idx = i - MAX_STICKS;
    if (!IS_POT_MULTIPOS(idx)) continue;

    auto& xpot = reusableBuffer.calib.inputs[i].xpot;
    int count = xpot.stepsCount;
    if (count > 1 && count <= XPOTS_MULTIPOS_COUNT) {
      for (int j = 0; j < count; j++) {
        for (int k = j + 1; k < count; k++) {
          if (xpot.steps[k] < xpot.steps[j]) {
            SWAP(xpot.steps[j], xpot.steps[k]);
          }
        }
      }
      StepsCalibData* calib = (StepsCalibData*)&g_eeGeneral.calib[i];
      calib->count = count - 1;
      for (int j = 0; j < calib->count; j++) {
        calib->steps[j] = (xpot.steps[j + 1] + xpot.steps[j]) >> 5;
      }
    } else {
      // TODO: a way to provide to 6POS default calibration values
      //
      // // load 6pos calib with factory data if 6 pos was not manually calibrated
      // constexpr int16_t factoryValues[]= {0x5,0xd,0x16,0x1f,0x28};
      // StepsCalibData * calib = (StepsCalibData *) &g_eeGeneral.calib[POT3];
      // calib->count = 5;
      // for (int j=0; j<calib->count ; j++) {
      //   calib->steps[j] = factoryValues[j];
      // }

      // not enough config points -> disable
      g_eeGeneral.potsConfig &= POT_CONFIG_DISABLE_MASK(idx);
    }
  }
}

void adcCalibStore()
{
  g_eeGeneral.chkSum = evalChkSum();
  storageDirty(EE_GENERAL);
}

uint16_t getRTCBatteryVoltage()
{
  // anaIn() outputs value divided by (1 << ANALOG_SCALE)
  return (anaIn(TX_RTC_VOLTAGE) * ADC_VREF_PREC2) / (2048 >> ANALOG_SCALE);
}

uint16_t getAnalogValue(uint8_t index)
{
  if (index >= MAX_STICKS) {
    if (!IS_POT_SLIDER_AVAILABLE(index - MAX_STICKS)) {
      // Use fixed analog value for non-existing and/or non-connected pots.
      // Non-connected analog inputs will slightly follow the adjacent connected
      // analog inputs, which produces ghost readings on these inputs.
      return 0;
    }
  }

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

    uint32_t v = getAnalogValue(x) >> (1 - ANALOG_SCALE);

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

