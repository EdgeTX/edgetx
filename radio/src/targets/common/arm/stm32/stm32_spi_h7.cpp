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
#include "stm32_dma.h"
#include "stm32_gpio.h"

#include "stm32_hal.h"
#include "stm32_hal_ll.h"

#include <string.h>

#if !defined(SPI_DISABLE_DMA)
#define USE_SPI_DMA
#endif

#define SPI_DUMMY_BYTE (0xFF)
#define SPI_MAX_XFER_SIZE (16*1024)

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

static inline uint32_t _get_spi_clocksource(SPI_TypeDef* SPIx)
{
#if defined(LL_RCC_SPI123_CLKSOURCE)
  return LL_RCC_SPI123_CLKSOURCE;
#endif

#if defined(LL_RCC_SPI1_CLKSOURCE)
  if (SPIx == SPI1) return LL_RCC_SPI1_CLKSOURCE;
#endif

#if defined(LL_RCC_SPI23_CLKSOURCE)
  if (SPIx == SPI2 || SPIx == SPI3) return LL_RCC_SPI23_CLKSOURCE;
#endif

#if defined(LL_RCC_SPI45_CLKSOURCE)
  if (SPIx == SPI4 || SPIx == SPI5) return LL_RCC_SPI45_CLKSOURCE;
#endif

#if defined(LL_RCC_SPI6_CLKSOURCE)
  if (SPIx == SPI6) return LL_RCC_SPI6_CLKSOURCE;
#endif

  return 0;
}

static uint32_t _get_spi_prescaler(SPI_TypeDef *SPIx, uint32_t max_freq)
{
  LL_RCC_ClocksTypeDef RCC_Clocks;
  LL_RCC_GetSystemClocksFreq(&RCC_Clocks);

  uint32_t pclk = LL_RCC_GetSPIClockFreq(_get_spi_clocksource(SPIx));
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
  
  LL_DMA_InitTypeDef dmaInit;
  LL_DMA_StructInit(&dmaInit);

#if defined(STM32H7)
  dmaInit.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dmaInit.Priority = LL_DMA_PRIORITY_VERYHIGH;
  dmaInit.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
  dmaInit.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
#elif defined(STM32H7RS)
  dmaInit.Priority = LL_DMA_LOW_PRIORITY_HIGH_WEIGHT;
#endif

  if (spi->rxDMA_PeriphRequest) {
    LL_DMA_DeInit(spi->DMA, spi->rxDMA_Stream);

#if defined(STM32H7)
    dmaInit.PeriphRequest = spi->rxDMA_PeriphRequest;
    dmaInit.PeriphOrM2MSrcAddress = LL_SPI_DMA_GetRxRegAddr(spi->SPIx);
#elif defined(STM32H7RS)
    dmaInit.Request = spi->rxDMA_PeriphRequest;
    dmaInit.SrcAddress = LL_SPI_DMA_GetRxRegAddr(spi->SPIx);
    dmaInit.SrcIncMode = LL_DMA_SRC_FIXED;
    dmaInit.DestIncMode = LL_DMA_DEST_INCREMENT;
    dmaInit.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_BYTE;
    dmaInit.DestDataWidth = LL_DMA_DEST_DATAWIDTH_BYTE;
#endif

    dmaInit.Direction = LL_DMA_DIRECTION_PERIPH_TO_MEMORY;
    LL_DMA_Init(spi->DMA, spi->rxDMA_Stream, &dmaInit);
  }

  if (spi->txDMA_PeriphRequest) {
    LL_DMA_DeInit(spi->DMA, spi->txDMA_Stream);

#if defined(STM32H7)
    dmaInit.PeriphRequest = spi->txDMA_PeriphRequest;
    dmaInit.PeriphOrM2MSrcAddress = LL_SPI_DMA_GetTxRegAddr(spi->SPIx);
#elif defined(STM32H7RS)
    dmaInit.Request = spi->txDMA_PeriphRequest;
    dmaInit.DestAddress = LL_SPI_DMA_GetTxRegAddr(spi->SPIx);
    dmaInit.SrcIncMode = LL_DMA_SRC_INCREMENT;
    dmaInit.DestIncMode = LL_DMA_DEST_FIXED;
    dmaInit.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_BYTE;
    dmaInit.DestDataWidth = LL_DMA_DEST_DATAWIDTH_BYTE;
#endif

    dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
    LL_DMA_Init(spi->DMA, spi->txDMA_Stream, &dmaInit);
  }
}

#if defined(STM32H7)
static void _config_tx_dma_data_width(const stm32_spi_t* spi, uint32_t data_width)
{
  uint32_t periph_size = LL_DMA_PDATAALIGN_BYTE;
  uint32_t memory_size = LL_DMA_MDATAALIGN_BYTE;
  if (data_width == sizeof(uint16_t)) {
    periph_size = LL_DMA_PDATAALIGN_HALFWORD;
    memory_size = LL_DMA_MDATAALIGN_HALFWORD;
  }
  LL_DMA_SetPeriphSize(spi->DMA, spi->txDMA_Stream, periph_size);
  LL_DMA_SetMemorySize(spi->DMA, spi->txDMA_Stream, memory_size);
}
#elif defined(STM32H7RS)
static void _config_tx_dma_data_width(const stm32_spi_t* spi, uint32_t data_width)
{
  uint32_t src_width = LL_DMA_SRC_DATAWIDTH_BYTE;
  uint32_t dest_width = LL_DMA_DEST_DATAWIDTH_BYTE;
  if (data_width == sizeof(uint16_t)) {
    src_width = LL_DMA_SRC_DATAWIDTH_HALFWORD;
    dest_width = LL_DMA_DEST_DATAWIDTH_HALFWORD;
  }
  LL_DMA_SetSrcDataWidth(spi->DMA, spi->txDMA_Stream, src_width);
  LL_DMA_SetDestDataWidth(spi->DMA, spi->txDMA_Stream, dest_width);
}
#endif

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

void stm32_spi_set_data_width(const stm32_spi_t* spi, uint32_t data_width)
{
  auto* SPIx = spi->SPIx;
  LL_SPI_SetDataWidth(SPIx, data_width);
}

static inline void spi_close_transfer(SPI_TypeDef* SPIx)
{
  while (!LL_SPI_IsActiveFlag_EOT(SPIx)) {
  }

  LL_SPI_ClearFlag_EOT(SPIx);
  LL_SPI_ClearFlag_TXTF(SPIx);

  LL_SPI_Disable(SPIx);
  CLEAR_BIT(SPIx->CFG1, SPI_CFG1_TXDMAEN | SPI_CFG1_RXDMAEN);
}

static void spi_transfer_8(SPI_TypeDef* SPIx, const uint8_t** tx_data,
                           unsigned tx_inc, uint8_t** rx_data, unsigned rx_inc,
                           uint16_t length)
{
  unsigned rx_len = length;
  unsigned tx_len = length;

  LL_SPI_SetTransferSize(SPIx, length);
  LL_SPI_Enable(SPIx);

  LL_SPI_StartMasterTransfer(SPIx);

  while (rx_len > 0 || tx_len > 0) {
    if (LL_SPI_IsActiveFlag_TXP(SPIx) && (tx_len > 0)) {
      LL_SPI_TransmitData8(SPIx, **tx_data);
      *tx_data += tx_inc;
      tx_len--;
    }

    if (LL_SPI_IsActiveFlag_RXP(SPIx) && (rx_len > 0)) {
      **rx_data = LL_SPI_ReceiveData8(SPIx);
      *rx_data += rx_inc;
      rx_len--;
    }
  }
  spi_close_transfer(SPIx);
}

uint32_t stm32_spi_transfer_bytes(const stm32_spi_t* spi, const uint8_t* out,
                                  uint8_t* in, uint32_t length)
{
  auto* SPIx = spi->SPIx;

  unsigned rx_inc = 1;
  unsigned tx_inc = 1;

  static uint8_t _scratch_data;

  const uint8_t* tx_data = out;
  if (!tx_data) {
    tx_data = &_scratch_data;
    tx_inc = 0;
  }

  uint8_t* rx_data = in;
  if (!rx_data) {
    rx_data = &_scratch_data;
    rx_inc = 0;
  }

  uint32_t xfer_len = length;
  while (xfer_len > 0) {
    uint16_t single_xfer_len =
        xfer_len > SPI_MAX_XFER_SIZE ? SPI_MAX_XFER_SIZE : xfer_len;
    spi_transfer_8(SPIx, &tx_data, tx_inc, &rx_data, rx_inc, single_xfer_len);
    xfer_len -= single_xfer_len;
  }

  return length;
}

static void spi_transfer_16(SPI_TypeDef* SPIx, const uint16_t** tx_data,
                            unsigned tx_inc, uint16_t** rx_data, unsigned rx_inc,
                            uint16_t length)
{
  unsigned rx_len = length;
  unsigned tx_len = length;

  LL_SPI_SetTransferSize(SPIx, length);
  LL_SPI_Enable(SPIx);

  LL_SPI_StartMasterTransfer(SPIx);

  while (rx_len > 0 || tx_len > 0) {
    if (LL_SPI_IsActiveFlag_TXP(SPIx) && (tx_len > 0)) {
      LL_SPI_TransmitData16(SPIx, **tx_data);
      *tx_data += tx_inc;
      tx_len--;
    }

    if (LL_SPI_IsActiveFlag_RXP(SPIx) && (rx_len > 0)) {
      **rx_data = LL_SPI_ReceiveData16(SPIx);
      *rx_data += rx_inc;
      rx_len--;
    }
  }
  spi_close_transfer(SPIx);
}

uint32_t stm32_spi_transfer_words(const stm32_spi_t* spi, const uint16_t* out,
                                  uint16_t* in, uint32_t length)
{
  auto* SPIx = spi->SPIx;

  unsigned rx_inc = 1;
  unsigned tx_inc = 1;

  static uint16_t _scratch_data;

  const uint16_t* tx_data = out;
  if (!tx_data) {
    tx_data = &_scratch_data;
    tx_inc = 0;
  }

  uint16_t* rx_data = in;
  if (!rx_data) {
    rx_data = &_scratch_data;
    rx_inc = 0;
  }

  uint32_t xfer_len = length;
  while (xfer_len > 0) {
    uint16_t single_xfer_len =
        xfer_len > SPI_MAX_XFER_SIZE ? SPI_MAX_XFER_SIZE : xfer_len;
    spi_transfer_16(SPIx, &tx_data, tx_inc, &rx_data, rx_inc, single_xfer_len);
    xfer_len -= single_xfer_len;
  }

  return length;
}

uint8_t stm32_spi_transfer_byte(const stm32_spi_t* spi, uint8_t out)
{
  uint8_t in;
  stm32_spi_transfer_bytes(spi, &out, &in, 1);
  return in;
}

uint16_t stm32_spi_transfer_word(const stm32_spi_t* spi, uint16_t out)
{
  uint16_t in;
  stm32_spi_transfer_words(spi, &out, &in, 1);
  return in;
}

#if defined(USE_SPI_DMA)
static void _dma_enable_stream(DMA_TypeDef* DMAx, uint32_t stream,
			       const void* data, uint32_t length)
{
  stm32_dma_check_tc_flag(DMAx, stream);
#if !defined(STM32H7RS)
  LL_DMA_SetMemoryAddress(DMAx, stream, (uintptr_t)data);
  LL_DMA_SetDataLength(DMAx, stream, length);
  LL_DMA_EnableStream(DMAx, stream);
#endif
}
#endif

static void spi_receive_dma(const stm32_spi_t* spi, void* data, uint16_t length)
{
  _dma_enable_stream(spi->DMA, spi->rxDMA_Stream, data, length);

  LL_SPI_SetTransferSize(spi->SPIx, length);
  LL_SPI_EnableDMAReq_RX(spi->SPIx);
  LL_SPI_Enable(spi->SPIx);
  LL_SPI_StartMasterTransfer(spi->SPIx);

  // Wait for end of DMA transfer
  while (!stm32_dma_check_tc_flag(spi->DMA, spi->rxDMA_Stream));
  spi_close_transfer(spi->SPIx);
}

uint32_t stm32_spi_dma_receive_bytes(const stm32_spi_t* spi, uint8_t* data,
                                     uint32_t length)
{
#if !defined(USE_SPI_DMA)
  return stm32_spi_transfer_bytes(spi, nullptr, data, length);
#else
  uint32_t xfer_len = length;
  while (xfer_len > 0) {
    uint16_t single_xfer_len =
        xfer_len > SPI_MAX_XFER_SIZE ? SPI_MAX_XFER_SIZE : xfer_len;
    spi_receive_dma(spi, data, single_xfer_len);
    xfer_len -= single_xfer_len;
    data += single_xfer_len;
  }

  return length;
#endif
}

static void spi_transmit_dma(const stm32_spi_t* spi, const void* data, uint16_t length)
{
  _dma_enable_stream(spi->DMA, spi->txDMA_Stream, data, length);

  LL_SPI_SetTransferSize(spi->SPIx, length);
  LL_SPI_EnableDMAReq_TX(spi->SPIx);
  LL_SPI_Enable(spi->SPIx);
  LL_SPI_StartMasterTransfer(spi->SPIx);

  // Wait for end of DMA transfer
  while (!stm32_dma_check_tc_flag(spi->DMA, spi->txDMA_Stream));
  spi_close_transfer(spi->SPIx);
}

uint32_t stm32_spi_dma_transmit_bytes(const stm32_spi_t* spi,
                                      const uint8_t* data, uint32_t length)
{
#if !defined(USE_SPI_DMA)
  return stm32_spi_transfer_bytes(spi, data, nullptr, length);
#else
  _config_tx_dma_data_width(spi, sizeof(uint8_t));

  uint32_t xfer_len = length;
  while (xfer_len > 0) {
    uint16_t single_xfer_len =
        xfer_len > SPI_MAX_XFER_SIZE ? SPI_MAX_XFER_SIZE : xfer_len;
    spi_transmit_dma(spi, data, single_xfer_len);
    xfer_len -= single_xfer_len;
    data += single_xfer_len;
  }

  return length;
#endif
}

uint32_t stm32_spi_dma_transmit_words(const stm32_spi_t* spi,
                                      const uint16_t* data, uint32_t length)
{
#if !defined(USE_SPI_DMA)
  return stm32_spi_transfer_words(spi, data, nullptr, length);
#else
  _config_tx_dma_data_width(spi, sizeof(uint16_t));

  uint32_t xfer_len = length;
  while (xfer_len > 0) {
    uint16_t single_xfer_len =
        xfer_len > SPI_MAX_XFER_SIZE ? SPI_MAX_XFER_SIZE : xfer_len;
    spi_transmit_dma(spi, data, single_xfer_len);
    xfer_len -= single_xfer_len;
    data += single_xfer_len;
  }

  return length;
#endif
}
