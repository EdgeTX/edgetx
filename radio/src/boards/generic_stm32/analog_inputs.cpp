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
#include "sticks_pwm_driver.h"

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
#include "stm32_pwm_inputs.inc"
#include "hal_adc_inputs.inc"

constexpr uint8_t n_ADC = DIM(_ADC_adc);
constexpr uint8_t n_ADC_spi = DIM(_ADC_spi);
constexpr uint8_t n_GPIO = DIM(_ADC_GPIOs);
constexpr uint8_t n_inputs = DIM(_ADC_inputs);

static_assert(n_inputs <= MAX_ADC_INPUTS, "Too many ADC inputs");
static_assert(n_inputs <= MAX_ANALOG_INPUTS, "Too many analog inputs");

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
  .inputs = _hal_inputs,
  .default_pots_cfg = _pot_default_config,
  .init = adc_init,
  .start_conversion = adc_start_read,
  .wait_completion = adc_wait_completion,
  .set_input_mask = stm32_hal_set_inputs_mask,
  .get_input_mask = stm32_hal_get_inputs_mask,
};

#if defined(PWM_STICKS)
#include "stm32_gpio.h"

static const stick_pwm_timer_t _sticks_timer = {
  .GPIOx = PWM_GPIO,
  .GPIO_Pin = PWM_GPIOA_PINS,
  .GPIO_Alternate = PWM_GPIO_AF,
  .TIMx = PWM_TIMER,
  .TIM_IRQn = PWM_IRQn,
};

#if !defined(PWM_IRQHandler)
  #error "Missing PWM_IRQHandler"
#endif

extern "C" void PWM_IRQHandler(void)
{
  sticks_pwm_isr(&_sticks_timer, _PWM_inputs, DIM(_PWM_inputs));
}

bool sticksPwmDetect()
{
  return sticks_pwm_detect(&_sticks_timer, _PWM_inputs, DIM(_PWM_inputs));
}

#else

bool sticksPwmDetect()
{
  return false;
}

#endif
