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

#include "stm32_hal_ll.h"
#include "hal/adc_driver.h"

enum {
  ADC_INPUT_STICK=0,
  ADC_INPUT_POT,
  ADC_INPUT_SLIDER,
  ADC_INPUT_EXT,
  ADC_INPUT_SWITCH,
  ADC_INPUT_BATT,
  ADC_INPUT_AXIS,
};

struct stm32_adc_input_t {
  const char*   name;
  uint8_t       type;
  GPIO_TypeDef* GPIOx;
  uint32_t      GPIO_Pin;
  uint32_t      ADC_Channel;
  uint8_t       inverted;
};

struct stm32_adc_t {
  ADC_TypeDef*         ADCx;
  DMA_TypeDef*         DMAx;
  uint32_t             DMA_Channel;
  uint32_t             DMA_Stream;
  IRQn_Type            DMA_Stream_IRQn;
  const uint8_t*       channels;
  uint8_t              n_channels;
  uint8_t              sample_time;
};

struct stm32_adc_gpio_t {
  GPIO_TypeDef*   GPIOx;
  const uint32_t* pins;
  uint8_t         n_pins;
};

struct stm32_spi_adc_t {
  SPI_TypeDef*    SPIx;
  const uint8_t*  channels;
  uint8_t         n_channels;  
};

// Driver to be passed to adcInit()
//extern const etx_hal_adc_driver_t stm32_hal_adc_driver;

bool stm32_hal_adc_init(const stm32_adc_t* ADCs, uint8_t n_ADC,
                        const stm32_adc_input_t* inputs,
                        const stm32_adc_gpio_t* ADC_GPIOs, uint8_t n_GPIO);

bool stm32_hal_adc_start_read(const stm32_adc_t* ADCs, uint8_t n_ADC);

void stm32_hal_adc_wait_completion(const stm32_adc_t* ADCs, uint8_t n_ADC,
                                   const stm32_adc_input_t* inputs, uint8_t n_inputs);

void stm32_hal_adc_disable_oversampling();

void stm32_hal_adc_isr(const stm32_adc_t* adc);
