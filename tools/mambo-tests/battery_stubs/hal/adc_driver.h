// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <cstdint>
#define ADC_MAX_FILTERED 2047
enum { ADC_INPUT_VBAT, ADC_INPUT_RTC_BAT };
unsigned adcGetMaxInputs(unsigned type);
unsigned adcGetInputOffset(unsigned type);
uint16_t anaIn(unsigned input);
uint16_t getBatteryVoltage();
