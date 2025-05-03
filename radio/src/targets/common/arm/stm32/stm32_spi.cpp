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

#include "stm32_spi.h"
#include "memory_sections.h"
#include "stm32_dma.h"
#include "stm32_gpio.h"
#include "definitions.h"
#include "stm32_hal.h"

#include <stdlib.h>
#include <string.h>

#if !defined(SPI_DISABLE_DMA)
#define USE_SPI_DMA
#endif

#define SPI_DUMMY_BYTE (0xFF)

void stm32_spi_enable_clock(SPI_TypeDef *SPIx)
{
  if (SPIx == SPI1) {
    __HAL_RCC_SPI1_CLK_ENABLE();
  }
#if defined(SPI2)
  else if (SPIx == SPI2) {
    __HAL_RCC_SPI2_CLK_ENABLE();
  }
#endif
#if defined(SPI3)
  else if (SPIx == SPI3) {
    __HAL_RCC_SPI3_CLK_ENABLE();
  }
#endif
#if defined(SPI4)
  else if (SPIx == SPI4) {
    __HAL_RCC_SPI4_CLK_ENABLE();
  }
#endif
#if defined(SPI5)
  else if (SPIx == SPI5) {
    __HAL_RCC_SPI5_CLK_ENABLE();
  }
#endif
#if defined(SPI6)
  else if (SPIx == SPI6) {
    __HAL_RCC_SPI6_CLK_ENABLE();
  }
#endif
}

static inline uint32_t _get_spi_af(SPI_TypeDef *SPIx)
{
#if defined(SPI3)
  if (SPIx == SPI3) return LL_GPIO_AF_6;
#endif
  return LL_GPIO_AF_5;
}

static uint32_t _get_spi_prescaler(SPI_TypeDef *SPIx, uint32_t max_freq)
{
  LL_RCC_ClocksTypeDef RCC_Clocks;
  LL_RCC_GetSystemClocksFreq(&RCC_Clocks);

  uint32_t pclk = RCC_Clocks.PCLK2_Frequency;
#if defined(SPI2)
  if (SPIx == SPI2) {
    pclk = RCC_Clocks.PCLK1_Frequency;
  }
#endif
#if defined(SPI3)
  if (SPIx == SPI3) {
    pclk = RCC_Clocks.PCLK1_Frequency;
  }
#endif
  uint32_t divider = (pclk + max_freq) / max_freq;
  uint32_t presc;
  if (divider > 128) {
    presc = LL_SPI_BAUDRATEPRESCALER_DIV256;
  } else if (divider > 64) {
    presc = LL_SPI_BAUDRATEPRESCALER_DIV128;
  } else if (divider > 32) {
    presc = LL_SPI_BAUDRATEPRESCALER_DIV64;
  } else if (divider > 16) {
    presc = LL_SPI_BAUDRATEPRESCALER_DIV32;
  } else if (divider > 8) {
    presc = LL_SPI_BAUDRATEPRESCALER_DIV16;
  } else if (divider > 4) {
    presc = LL_SPI_BAUDRATEPRESCALER_DIV8;
  } else if (divider > 2) {
    presc = LL_SPI_BAUDRATEPRESCALER_DIV4;
  } else {
    presc = LL_SPI_BAUDRATEPRESCALER_DIV2;
  }

  return presc;
}

static void _init_gpios(const stm32_spi_t* spi)
{
  gpio_init_af(spi->MISO, _get_spi_af(spi->SPIx), GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(spi->SCK, _get_spi_af(spi->SPIx), GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init_af(spi->MOSI, _get_spi_af(spi->SPIx), GPIO_PIN_SPEED_VERY_HIGH);
  gpio_init(spi->CS, GPIO_OUT, GPIO_PIN_SPEED_HIGH);
}

#if defined(USE_SPI_DMA)
static void _config_dma_streams(const stm32_spi_t* spi)
{
  stm32_dma_enable_clock(spi->DMA);
  LL_DMA_DeInit(spi->DMA, spi->rxDMA_Stream);
  LL_DMA_DeInit(spi->DMA, spi->txDMA_Stream);
  
  LL_DMA_InitTypeDef dmaInit;
  LL_DMA_StructInit(&dmaInit);

  dmaInit.Channel = spi->DMA_Channel;
  dmaInit.PeriphOrM2MSrcAddress = LL_SPI_DMA_GetRegAddr(spi->SPIx);
  dmaInit.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dmaInit.Priority = LL_DMA_PRIORITY_VERYHIGH;
  dmaInit.FIFOMode = spi->DMA_FIFOMode;
  dmaInit.FIFOThreshold = spi->DMA_FIFOThreshold;
  dmaInit.MemoryOrM2MDstDataSize = spi->DMA_MemoryOrM2MDstDataSize;
  dmaInit.MemBurst = spi->DMA_MemBurst;
  dmaInit.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
  LL_DMA_Init(spi->DMA, spi->rxDMA_Stream, &dmaInit);

  dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  LL_DMA_Init(spi->DMA, spi->txDMA_Stream, &dmaInit);
}
#endif

void stm32_spi_init(const stm32_spi_t* spi, uint32_t data_width)
{
  _init_gpios(spi);

  auto SPIx = spi->SPIx;
  stm32_spi_enable_clock(SPIx);
  LL_SPI_DeInit(SPIx);

  LL_SPI_InitTypeDef spiInit;
  LL_SPI_StructInit(&spiInit);

  spiInit.TransferDirection = LL_SPI_FULL_DUPLEX;
  spiInit.Mode = LL_SPI_MODE_MASTER;
  spiInit.NSS = LL_SPI_NSS_SOFT;
  spiInit.DataWidth = data_width;

  LL_SPI_Init(SPIx, &spiInit);
  LL_SPI_Enable(SPIx);

#if defined(USE_SPI_DMA)
  if (spi->DMA) {
    _config_dma_streams(spi);
  }
#endif
}

void stm32_spi_select(const stm32_spi_t* spi)
{
  gpio_clear(spi->CS);
}

void stm32_spi_unselect(const stm32_spi_t* spi)
{
  gpio_set(spi->CS);
}

void stm32_spi_set_max_baudrate(const stm32_spi_t* spi, uint32_t baudrate)
{
  auto* SPIx = spi->SPIx;
  uint32_t presc = _get_spi_prescaler(SPIx, baudrate);
  LL_SPI_SetBaudRatePrescaler(SPIx, presc);
}

uint8_t stm32_spi_transfer_byte(const stm32_spi_t* spi, uint8_t out)
{
  auto* SPIx = spi->SPIx;

  while (!LL_SPI_IsActiveFlag_TXE(SPIx));
  LL_SPI_TransmitData8(SPIx, out);

  while (!LL_SPI_IsActiveFlag_RXNE(SPIx));
  return LL_SPI_ReceiveData8(SPIx);
}

uint32_t stm32_spi_transfer_bytes(const stm32_spi_t* spi, const uint8_t* out,
                                  uint8_t* in, uint32_t length)
{
  unsigned trans_bytes = 0;
  uint8_t in_temp;

  for (trans_bytes = 0; trans_bytes < length; trans_bytes++) {
    if (out != nullptr) {
      in_temp = stm32_spi_transfer_byte(spi, out[trans_bytes]);
    } else {
      in_temp = stm32_spi_transfer_byte(spi, SPI_DUMMY_BYTE);
    }
    if (in != nullptr) {
      in[trans_bytes] = in_temp;
    }
  }

  return trans_bytes;
}

uint16_t stm32_spi_transfer_word(const stm32_spi_t* spi, uint16_t out)
{
  auto* SPIx = spi->SPIx;
  while (!LL_SPI_IsActiveFlag_TXE(SPIx));
  LL_SPI_TransmitData16(SPIx, out);

  while (!LL_SPI_IsActiveFlag_RXNE(SPIx));
  return LL_SPI_ReceiveData16(SPIx);
}

#if defined(USE_SPI_DMA)
static uint16_t _scratch_byte __DMA_NO_CACHE;
static uint8_t _scratch_buffer[512] __DMA_NO_CACHE;

#if defined(STM32F4)
extern uint32_t _sram;
extern uint32_t _eram;
#define _IS_DMA_BUFFER(addr) \
  ((intptr_t)(addr) >= (intptr_t)&_sram && (intptr_t)(addr) <= (intptr_t)&_eram)
#else
#define _IS_DMA_BUFFER(addr) (true)
#endif

#define _IS_ALIGNED(addr) (((intptr_t)(addr) & 3U) == 0U)

static void _dma_enable_stream(DMA_TypeDef* DMAx, uint32_t stream,
                               const void* data, uint32_t length)
{
  stm32_dma_check_tc_flag(DMAx, stream);
  LL_DMA_SetMemoryAddress(DMAx, stream, (uintptr_t)data);
  LL_DMA_SetDataLength(DMAx, stream, length);
  LL_DMA_EnableStream(DMAx, stream);
}
#endif

uint32_t stm32_spi_dma_receive_bytes(const stm32_spi_t* spi, uint8_t* data,
                                     uint32_t length)
{
#if defined(USE_SPI_DMA)
  if (!spi->DMA) {
    return stm32_spi_transfer_bytes(spi, nullptr, data, length);
  }

  bool use_scratch_buffer = !_IS_DMA_BUFFER(data) || !_IS_ALIGNED(data);
  if (use_scratch_buffer) {
    _dma_enable_stream(spi->DMA, spi->rxDMA_Stream, _scratch_buffer, length);
  } else {
    _dma_enable_stream(spi->DMA, spi->rxDMA_Stream, data, length);
  }
  LL_SPI_EnableDMAReq_RX(spi->SPIx);

  _scratch_byte = 0xFFFF;
  LL_DMA_SetMemoryIncMode(spi->DMA, spi->txDMA_Stream, LL_DMA_MEMORY_NOINCREMENT);
  _dma_enable_stream(spi->DMA, spi->txDMA_Stream, &_scratch_byte, length);
  LL_SPI_EnableDMAReq_TX(spi->SPIx);

  // Wait for end of DMA transfer
  while(!stm32_dma_check_tc_flag(spi->DMA, spi->rxDMA_Stream));

  // Wait for TXE=1
  while (!LL_SPI_IsActiveFlag_TXE(spi->SPIx));
  
  // Wait for BSY=0
  while(LL_SPI_IsActiveFlag_BSY(spi->SPIx));

  // Disable SPI TX/RX DMA requests
  LL_SPI_DisableDMAReq_TX(spi->SPIx);
  LL_SPI_DisableDMAReq_RX(spi->SPIx);

  if (use_scratch_buffer) {
    memcpy(data, _scratch_buffer, length);
  }
  
  return length;
#else
  return stm32_spi_transfer_bytes(spi, nullptr, data, length);
#endif
}

uint32_t stm32_spi_dma_transmit_bytes(const stm32_spi_t* spi,
                                      const uint8_t* data, uint32_t length)
{
#if defined(USE_SPI_DMA)
  if (!spi->DMA) {
    return stm32_spi_transfer_bytes(spi, data, nullptr, length);
  }

  bool use_scratch_buffer = !_IS_DMA_BUFFER(data) || !_IS_ALIGNED(data);
  if (use_scratch_buffer) {
    memcpy(_scratch_buffer, data, length);
    data = _scratch_buffer;
  }

  LL_DMA_SetMemoryIncMode(spi->DMA, spi->txDMA_Stream, LL_DMA_MEMORY_INCREMENT);
  _dma_enable_stream(spi->DMA, spi->txDMA_Stream, data, length);
  LL_SPI_EnableDMAReq_TX(spi->SPIx);

  // Wait for end of DMA transfer
  while (!stm32_dma_check_tc_flag(spi->DMA, spi->txDMA_Stream));

  // Wait for TXE=1
  while (!LL_SPI_IsActiveFlag_TXE(spi->SPIx));

  // Wait for BSY=0
  while (LL_SPI_IsActiveFlag_BSY(spi->SPIx));

  // Clear data register
  if (LL_SPI_IsActiveFlag_RXNE(spi->SPIx)) {
    (void)LL_SPI_ReceiveData8(spi->SPIx);
  }

  // Disable SPI TX DMA requests
  LL_SPI_DisableDMAReq_TX(spi->SPIx);

  return length;
#else
  return stm32_spi_transfer_bytes(spi, data, 0, length);
#endif
}
