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
#include "edgetx_types.h"

// 12-bit values
#define ADC_MAX_VALUE 4095

#define ADC_INVERT_VALUE(v) (ADC_MAX_VALUE - (v))

// tune this value, bigger value - more filtering (range: 1-5) (see explanation below)
#define JITTER_FILTER_STRENGTH  4
// tune this value, bigger value - more filtering (range: 0-1) (see explanation below)
#define ANALOG_SCALE            1
#define JITTER_ALPHA            (1<<JITTER_FILTER_STRENGTH)

enum {
  ADC_INPUT_MAIN=0, // gimbals / wheel + throttle
  ADC_INPUT_FLEX,
  ADC_INPUT_VBAT,
  ADC_INPUT_RTC_BAT,
  ADC_INPUT_ALL,
};

// Air radios
enum {
  ADC_MAIN_LH,
  ADC_MAIN_LV,
  ADC_MAIN_RV,
  ADC_MAIN_RH,
};

// Surface radios
enum {
  ADC_MAIN_ST,
  ADC_MAIN_TH,
};

struct etx_hal_adc_input_t {
  const char* name;
  const char* label;
  const char* short_label;
};

struct etx_hal_adc_inputs_t {
  uint8_t                    n_inputs;
  uint8_t                    offset;
  const etx_hal_adc_input_t* inputs;
};

struct etx_hal_adc_driver_t {

  const etx_hal_adc_inputs_t* inputs; // ADC_INPUT_ALL + 1;
  potconfig_t default_pots_cfg;
  
  bool (*init)();
  bool (*start_conversion)();
  void (*wait_completion)();

  void (*set_input_mask)(uint32_t);
  uint32_t (*get_input_mask)();
};

bool adcInit(const etx_hal_adc_driver_t* driver);
// void adcDeInit();

bool     adcRead();
uint16_t getBatteryVoltage();
uint16_t getRTCBatteryVoltage();
uint16_t getAnalogValue(uint8_t index);
void setAnalogValue(uint8_t index, uint16_t value);
uint16_t* getAnalogValues();

// Run calibration steps:

// Set default values before loading radio settings
void adcCalibDefaults();

// Set mid-points and reset min/max in temporary buffer
void adcCalibSetMidPoint();

// Adjust min/max into calibration data
void adcCalibSetMinMax();

// Finalise calibration data and persist in storage
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

potconfig_t adcGetDefaultPotsConfig();

uint8_t adcGetMaxInputs(uint8_t type);
uint8_t adcGetInputOffset(uint8_t type);

uint8_t adcGetMaxCalibratedInputs();

uint16_t adcGetInputValue(uint8_t type, uint8_t idx);
const char* adcGetInputName(uint8_t type, uint8_t idx);

const char* adcGetInputName(uint8_t idx);
int adcGetInputIdx(const char* input, uint8_t len);

const char* adcGetInputLabel(uint8_t type, uint8_t idx);
const char* adcGetInputShortLabel(uint8_t type, uint8_t idx);

void adcSetInputMask(uint32_t mask);
uint32_t adcGetInputMask();

// To be implemented by the target driver
// int8_t adcGetVRTC();
// int8_t adcGetVBAT();
// const char* adcGetStickName(uint8_t idx);
// const char* adcGetPotName(uint8_t idx);
// uint8_t adcGetMaxSticks();
// uint8_t adcGetMaxPots();
