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

#include "stm32_i2s.h"
#include "stm32_gpio.h"
#include "stm32_spi.h"
#include "stm32_dma.h"

#include "hal/gpio.h"

#include "stm32_hal_ll.h"

static void i2s_gpio_init(const stm32_i2s_t* i2s)
{
  stm32_i2s_pindef_t gpios[] = {
      i2s->MCK, i2s->WS, i2s->CK, i2s->SDO, i2s->SDI,
  };

  for (unsigned i = 0; i < sizeof(gpios) / sizeof(gpios[0]); i++) {
    gpio_init_af(gpios[i].pin, gpios[i].af, GPIO_PIN_SPEED_VERY_HIGH);
  }
}

static void i2s_dma_init(const stm32_i2s_t* i2s)
{
  stm32_dma_enable_clock(i2s->DMA);

  LL_DMA_DeInit(i2s->DMA, i2s->DMA_Stream);

  LL_DMA_InitTypeDef dmaInit;
  LL_DMA_StructInit(&dmaInit);

#if defined(STM32H7)
  dmaInit.Mode = LL_DMA_MODE_CIRCULAR;
  dmaInit.PeriphRequest = i2s->DMA_PeriphRequest;
  dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  dmaInit.Priority = LL_DMA_PRIORITY_VERYHIGH;
  dmaInit.PeriphOrM2MSrcAddress = LL_SPI_DMA_GetTxRegAddr(i2s->SPIx);
  dmaInit.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
  dmaInit.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dmaInit.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
#elif defined(STM32H7RS)
  dmaInit.Request = i2s->DMA_PeriphRequest;
  dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  dmaInit.Priority = LL_DMA_HIGH_PRIORITY;
  dmaInit.DestAddress = LL_SPI_DMA_GetTxRegAddr(i2s->SPIx);
  dmaInit.DestDataWidth = LL_DMA_DEST_DATAWIDTH_HALFWORD;
  dmaInit.SrcIncMode = LL_DMA_SRC_INCREMENT;
  dmaInit.SrcDataWidth = LL_DMA_SRC_DATAWIDTH_HALFWORD;
#endif
  LL_DMA_Init(i2s->DMA, i2s->DMA_Stream, &dmaInit);
}

int stm32_i2s_init(const stm32_i2s_t* i2s)
{
  if (!i2s) return -1;
  i2s_gpio_init(i2s);
  
  stm32_spi_enable_clock(i2s->SPIx);
  LL_I2S_Disable(i2s->SPIx);

  LL_I2S_InitTypeDef i2s_init;
  LL_I2S_StructInit(&i2s_init);

  i2s_init.Mode = LL_I2S_MODE_MASTER_TX;
  i2s_init.Standard = LL_I2S_STANDARD_PHILIPS;
  i2s_init.DataFormat = LL_I2S_DATAFORMAT_16B;
  i2s_init.MCLKOutput = LL_I2S_MCLK_OUTPUT_ENABLE;
  i2s_init.AudioFreq = LL_I2S_AUDIOFREQ_32K;

  if (LL_I2S_Init(i2s->SPIx, &i2s_init) != SUCCESS) return -1;
  LL_I2S_Enable(i2s->SPIx);

  if (i2s->DMA) i2s_dma_init(i2s);

  return 0;
}

void stm32_i2s_config_dma_stream(const stm32_i2s_t* i2s, void* buffer, uint32_t nb_xfer)
{
  LL_I2S_Disable(i2s->SPIx);

  // clear HT & TC flags
  stm32_dma_check_ht_flag(i2s->DMA, i2s->DMA_Stream);
  stm32_dma_check_tc_flag(i2s->DMA, i2s->DMA_Stream);

  // setup DMA buffer address & length
#if defined(STM32H7)
  LL_DMA_SetMemoryAddress(i2s->DMA, i2s->DMA_Stream, (uintptr_t)buffer);
  LL_DMA_SetDataLength(i2s->DMA, i2s->DMA_Stream, nb_xfer);
#elif defined(STM32H7RS)
  LL_DMA_SetSrcAddress(i2s->DMA, i2s->DMA_Stream, (uintptr_t)buffer);
  LL_DMA_SetBlkDataLength(i2s->DMA, i2s->DMA_Stream, nb_xfer * 2);  // bytes
#endif
}

void stm32_i2s_start_dma_stream(const stm32_i2s_t* i2s)
{
#if defined(STM32H7)
  LL_DMA_EnableStream(i2s->DMA, i2s->DMA_Stream);
#elif defined(STM32H7RS)
  LL_DMA_EnableChannel(i2s->DMA, i2s->DMA_Stream);
#endif

  // enable DMA request
  LL_SPI_EnableDMAReq_TX(i2s->SPIx);

  // enable I2S
  LL_I2S_Enable(i2s->SPIx);

  // start transfer
  LL_SPI_StartMasterTransfer(i2s->SPIx);
}

bool stm32_i2s_is_xfer_started(const stm32_i2s_t* i2s)
{
  return READ_BIT(i2s->SPIx->CR1, SPI_CR1_CSTART);
}
