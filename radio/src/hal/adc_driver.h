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

// tune this value, bigger value - more filtering (range: 1-5) (see explanation below)
#define JITTER_FILTER_STRENGTH  4
// tune this value, bigger value - more filtering (range: 0-1) (see explanation below)
#define ANALOG_SCALE            1
#define JITTER_ALPHA            (1<<JITTER_FILTER_STRENGTH)

enum {
  ADC_INPUT_STICK=0,
  ADC_INPUT_POT,
  ADC_INPUT_AXIS,
  ADC_INPUT_VBAT,
  ADC_INPUT_RTC_BAT,
  ADC_INPUT_ALL,
};

struct etx_hal_adc_inputs_t {
  uint8_t      n_inputs;
  uint8_t      offset;
  const char** names;
};

struct etx_hal_adc_driver_t {

  const etx_hal_adc_inputs_t* inputs; // ADC_INPUT_ALL + 1;
  
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
void setAnalogValue(uint8_t index, uint16_t value);
uint16_t* getAnalogValues();

// Run calibration steps
void adcCalibMinMax();
void adcCalibSetMidPoint();
void adcCalibSetMinMax();
void adcCalibSetXPot();
void adcCalibStore();

#if defined(JITTER_MEASURE)
extern JitterMeter<uint16_t> rawJitter[MAX_ANALOG_INPUTS];
extern JitterMeter<uint16_t> avgJitter[MAX_ANALOG_INPUTS];
#endif

void getADC();
uint16_t anaIn(uint8_t chan);
uint32_t anaIn_diag(uint8_t chan);
uint16_t getBatteryVoltage();

// Warning:
//   STM32 uses a 25K+25K voltage divider bridge to measure the battery voltage
//   Measuring VBAT puts considerable drain (22 µA) on the battery instead of
//   normal drain (~10 nA)
void enableVBatBridge();
void disableVBatBridge();
bool isVBatBridgeEnabled();

uint8_t adcGetMaxInputs(uint8_t type);
uint8_t adcGetInputOffset(uint8_t type);

uint16_t adcGetInputValue(uint8_t type, uint8_t idx);
const char* adcGetInputName(uint8_t type, uint8_t idx);

// To be implemented by the target driver
// int8_t adcGetVRTC();
// int8_t adcGetVBAT();
// const char* adcGetStickName(uint8_t idx);
// const char* adcGetPotName(uint8_t idx);
// uint8_t adcGetMaxSticks();
// uint8_t adcGetMaxPots();
