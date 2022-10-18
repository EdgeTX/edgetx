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

struct stm32_spi_adc_t {
  SPI_TypeDef*    SPIx;
  GPIO_TypeDef*   GPIOx;
  uint32_t        GPIO_Pins;
  uint32_t        GPIO_AF;
  uint32_t        GPIO_CS;
  const uint8_t*  channels;
  uint8_t         n_channels;  
};

#define _SPI_ADC_CS_HIGH(adc) LL_GPIO_SetOutputPin(adc->GPIOx, adc->GPIO_CS)
#define _SPI_ADC_CS_LOW(adc) LL_GPIO_ResetOutputPin(adc->GPIOx, adc->GPIO_CS)

