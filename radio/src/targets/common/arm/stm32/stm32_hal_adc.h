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

struct stm32_hal_adc_channel {
  const uint8_t adc_channel;
  const uint8_t rank;
  const uint8_t sample_time;
};

typedef const stm32_hal_adc_channel* (*stm32_hal_adc_get_channels)();
typedef uint8_t (*stm32_hal_adc_get_nconv)();
typedef uint16_t* (*stm32_hal_adc_get_dma_buffer)();

struct stm32_hal_adc {
  ADC_TypeDef* adc;

  DMA_Stream_TypeDef*                dma_stream;
  const stm32_hal_adc_get_dma_buffer get_dma_buffer;

  const stm32_hal_adc_get_nconv    get_nconv;
  const stm32_hal_adc_get_channels get_channels;
};

// Driver to be passed to adcInit()
extern const etx_hal_adc_driver_t stm32_hal_adc_driver;
