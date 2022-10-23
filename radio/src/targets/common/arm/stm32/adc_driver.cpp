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

#include "stm32_hal_ll.h"
#include "stm32_hal_adc.h"

#include "board_common.h"

#include "hw_adc_inputs.inc"
#include "definitions.h"

// needed to prevent the compiler to eject this symbol !!!
// TODO: place this in some header used by board.cpp
extern const etx_hal_adc_driver_t _adc_driver;

constexpr uint8_t n_ADC = DIM(_ADC_adc);
constexpr uint8_t n_GPIO = DIM(_ADC_GPIOs);
constexpr uint8_t n_inputs = DIM(_ADC_inputs);

static bool adc_init()
{
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
