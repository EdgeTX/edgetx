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

#include "analog_inputs.h"
#include "stm32_adc.h"
#include "stm32_spi_adc.h"

#include "hal.h"

#if defined(ADC_SPI)
  #include "ads79xx.h"
#endif

#include "definitions.h"

#include "myeeprom.h"
#include "translations.h"

#include <string.h>

// generated files
#include "stm32_adc_inputs.inc"
#include "hal_adc_inputs.inc"

constexpr uint8_t n_ADC = DIM(_ADC_adc);
constexpr uint8_t n_ADC_spi = DIM(_ADC_spi);
constexpr uint8_t n_GPIO = DIM(_ADC_GPIOs);
constexpr uint8_t n_inputs = DIM(_ADC_inputs);

static bool adc_init()
{
  bool success = stm32_hal_adc_init(_ADC_adc, n_ADC, _ADC_inputs, _ADC_GPIOs, n_GPIO);
#if defined(ADC_SPI)
  if (n_ADC_spi > 0) ads79xx_init(&_ADC_spi[0]);
#endif
  return success;
}

static bool adc_start_read()
{
  bool success = stm32_hal_adc_start_read(_ADC_adc, n_ADC, _ADC_inputs, n_inputs);
#if defined(ADC_SPI)
  if (n_ADC_spi > 0) {
    success = success && ads79xx_adc_start_read(&_ADC_spi[0], _ADC_inputs);
  }
#endif
  return success;
}

static void adc_wait_completion()
{
#if defined(ADC_SPI)
  // ADS79xx does all the work in the completion function
  // so it's probably better to poll it first
  if (n_ADC_spi > 0) ads79xx_adc_wait_completion(&_ADC_spi[0], _ADC_inputs);
#endif
  stm32_hal_adc_wait_completion(_ADC_adc, n_ADC, _ADC_inputs, n_inputs);
}

const etx_hal_adc_driver_t _adc_driver = {
  _hal_inputs,
  _pot_default_config,
  adc_init,
  adc_start_read,
  adc_wait_completion
};

