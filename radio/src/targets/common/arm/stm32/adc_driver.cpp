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
#include "board_common.h"

#include "hw_adc_inputs.inc"
#include "definitions.h"
#include <string.h>

// needed to prevent the compiler to eject this symbol !!!
// TODO: place this in some header used by board.cpp
extern const etx_hal_adc_driver_t _adc_driver;

constexpr uint8_t n_ADC = DIM(_ADC_adc);
constexpr uint8_t n_ADC_spi = DIM(_ADC_spi);
constexpr uint8_t n_GPIO = DIM(_ADC_GPIOs);
constexpr uint8_t n_inputs = DIM(_ADC_inputs);

static int8_t _vbat_input;
static int8_t _vrtc_input;

static int8_t _find_input_name(const char* name)
{
  for (uint8_t i = 0; i < n_inputs; i++) {
    if (!strcmp(_ADC_inputs[i].name, name)) return i;
  }

  return -1;
}

static bool adc_init()
{
  _vbat_input = _find_input_name("VBAT");
  _vrtc_input = _find_input_name("RTC_BAT");

  return stm32_hal_adc_init(_ADC_adc, n_ADC, _ADC_inputs, _ADC_GPIOs, n_GPIO);
}

static bool adc_start_read()
{
  return stm32_hal_adc_start_read(_ADC_adc, n_ADC);
}

static void adc_wait_completion()
{
  return stm32_hal_adc_wait_completion(_ADC_adc, n_ADC, _ADC_inputs, n_inputs);
}

const etx_hal_adc_driver_t _adc_driver = {
  adc_init,
  adc_start_read,
  adc_wait_completion
};

uint8_t adc_get_n_inputs() { return n_inputs; }
const stm32_adc_input_t* adc_get_inputs() { return _ADC_inputs; }

const stm32_spi_adc_t* adc_spi_get()
{
  if (n_ADC_spi > 0) return &_ADC_spi[0];
  return nullptr;
}

int8_t adcGetVRTC() { return _vrtc_input; }
int8_t adcGetVBAT() { return _vbat_input; }

const char* adcGetStickName(uint8_t idx)
{
  if (idx >= DIM(_stick_inputs)) return "";
  return _stick_inputs[idx];
}

const char* adcGetPotName(uint8_t idx)
{
  if (idx >= DIM(_pot_inputs)) return "";
  return _pot_inputs[idx];
}

uint8_t adcGetMaxSticks()
{
  return DIM(_stick_inputs);
}

uint8_t adcGetMaxPots()
{
  return DIM(_pot_inputs);
}
