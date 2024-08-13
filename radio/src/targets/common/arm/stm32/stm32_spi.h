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
#include <stdint.h>


struct stm32_spi_t {
  SPI_TypeDef*   SPIx;
  GPIO_TypeDef*  SPI_GPIOx;
  uint32_t       SPI_Pins; // SCK, MISO, MOSI
  GPIO_TypeDef*  CS_GPIOx;
  uint32_t       CS_Pin;   // CS

  DMA_TypeDef*   DMA;
#if defined(STM32H7) || defined(STM32H7RS)
  uint32_t       txDMA_PeriphRequest;
  uint32_t       rxDMA_PeriphRequest;
#else
  uint32_t       DMA_Channel;
#endif
  uint32_t       txDMA_Stream;
  uint32_t       rxDMA_Stream;
  uint32_t       DMA_FIFOMode;
  uint32_t       DMA_FIFOThreshold;
  uint32_t       DMA_MemoryOrM2MDstDataSize;
  uint32_t       DMA_MemBurst;         
};

void stm32_spi_enable_clock(SPI_TypeDef *SPIx);

void stm32_spi_init(const stm32_spi_t* spi, uint32_t data_width);
void stm32_spi_deinit(const stm32_spi_t* spi);

void stm32_spi_select(const stm32_spi_t* spi);
void stm32_spi_unselect(const stm32_spi_t* spi);

void stm32_spi_set_max_baudrate(const stm32_spi_t* spi, uint32_t baudrate);

uint8_t stm32_spi_transfer_byte(const stm32_spi_t* spi, uint8_t out);
uint16_t stm32_spi_transfer_word(const stm32_spi_t* spi, uint16_t out);

uint16_t stm32_spi_transfer_bytes(const stm32_spi_t* spi, const uint8_t* out,
                                  uint8_t* in, uint16_t length);

uint16_t stm32_spi_dma_receive_bytes(const stm32_spi_t* spi, uint8_t* data,
                                     uint16_t length);

uint16_t stm32_spi_dma_transmit_bytes(const stm32_spi_t* spi, const uint8_t* data,
                                      uint16_t length);
