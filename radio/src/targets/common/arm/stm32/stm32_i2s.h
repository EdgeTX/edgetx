/*
 * Copyright (C) EdgeTX
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

#include "stm32_gpio.h"

struct stm32_i2s_pindef_t {
  gpio_t pin;
  gpio_af_t af;
};

struct stm32_i2s_t {
  SPI_TypeDef*       SPIx;
  stm32_i2s_pindef_t MCK;
  stm32_i2s_pindef_t WS;
  stm32_i2s_pindef_t CK;
  stm32_i2s_pindef_t SDO;
  stm32_i2s_pindef_t SDI;

  DMA_TypeDef* DMA;
  uint32_t     DMA_Stream;
  uint32_t     DMA_PeriphRequest;
};

int stm32_i2s_init(const stm32_i2s_t* i2s);

void stm32_i2s_config_dma_stream(const stm32_i2s_t* i2s, void* buffer, uint32_t nb_xfer);
void stm32_i2s_start_dma_stream(const stm32_i2s_t* i2s);

bool stm32_i2s_is_xfer_started(const stm32_i2s_t* i2s);
