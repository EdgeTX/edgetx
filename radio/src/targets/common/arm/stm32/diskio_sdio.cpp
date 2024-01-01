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


#include "diskio_sdio.h"

#include "stm32_dma.h"
#include "stm32_hal.h"
#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"

#include "hal.h"

#if !defined(SD_SPI)

// #include "delays_driver.h"
#include "debug.h"

#include <string.h>

/* Configure PC.08, PC.09, PC.10, PC.11 pins: D0, D1, D2, D3 pins */
#if !defined(SD_SDIO_DATA_GPIO) && !defined(SD_SDIO_DATA_GPIO_PINS)
#define SD_SDIO_DATA_GPIO GPIOC
#define SD_SDIO_DATA_GPIO_PINS \
  (LL_GPIO_PIN_8 | LL_GPIO_PIN_9 | LL_GPIO_PIN_10 | LL_GPIO_PIN_11)
#endif

/* Configure PD.02 CMD line */
#if !defined(SD_SDIO_CMD_GPIO) && !defined(SD_SDIO_CMD_GPIO_PIN)
#define SD_SDIO_CMD_GPIO GPIOD
#define SD_SDIO_CMD_GPIO_PIN LL_GPIO_PIN_2
#endif

#if !defined(SD_SDIO_CLK_GPIO) && !defined(SD_SDIO_CLK_GPIO_PIN)
#define SD_SDIO_CLK_GPIO GPIOC
#define SD_SDIO_CLK_GPIO_PIN LL_GPIO_PIN_12
#endif

#if FF_MAX_SS != FF_MIN_SS
#error "Variable sector size is not supported"
#endif

#define BLOCK_SIZE FF_MAX_SS
#define SD_TIMEOUT 300 /* 300ms */

// HAL state
static SD_HandleTypeDef sdio;
static DMA_HandleTypeDef sdioTxDma;

// Disk status
static volatile uint32_t WriteStatus;
static volatile uint32_t ReadStatus;

static void sdio_low_level_init(void)
{
  /* Enable the SDIO APB2 Clock */
  __HAL_RCC_SDIO_CLK_ENABLE();

  LL_GPIO_InitTypeDef  GPIO_InitStructure;
  LL_GPIO_StructInit(&GPIO_InitStructure);

  stm32_gpio_enable_clock(SD_SDIO_DATA_GPIO);
  stm32_gpio_enable_clock(SD_SDIO_CMD_GPIO);
  stm32_gpio_enable_clock(SD_SDIO_CLK_GPIO);

  GPIO_InitStructure.Pin = SD_SDIO_DATA_GPIO_PINS;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStructure.Alternate = LL_GPIO_AF_12; // SDIO
  LL_GPIO_Init(SD_SDIO_DATA_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = SD_SDIO_CMD_GPIO_PIN;
  LL_GPIO_Init(SD_SDIO_CMD_GPIO, &GPIO_InitStructure);

  /* Configure PC.12 pin: CLK pin */
  GPIO_InitStructure.Pin = SD_SDIO_CLK_GPIO_PIN;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(SD_SDIO_CLK_GPIO, &GPIO_InitStructure);

  // SDIO Interrupt ENABLE
  NVIC_SetPriority(SDIO_IRQn, 0);
  NVIC_EnableIRQ(SDIO_IRQn);

  // Init SDIO DMA instance
  sdioTxDma.Instance = SD_SDIO_DMA_STREAM;
  sdioTxDma.Init.Channel = SD_SDIO_DMA_CHANNEL;
  sdioTxDma.Init.PeriphInc = DMA_PINC_DISABLE;
  sdioTxDma.Init.MemInc = DMA_MINC_ENABLE;
  sdioTxDma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  sdioTxDma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  sdioTxDma.Init.Mode = DMA_PFCTRL;
  sdioTxDma.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  sdioTxDma.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
  sdioTxDma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  sdioTxDma.Init.MemBurst = DMA_MBURST_INC4;
  sdioTxDma.Init.PeriphBurst = DMA_PBURST_INC4;

  stm32_dma_enable_clock(SD_SDIO_DMA);
  HAL_DMA_Init(&sdioTxDma);

  __HAL_LINKDMA(&sdio, hdmatx, sdioTxDma);
  __HAL_LINKDMA(&sdio, hdmarx, sdioTxDma);
  
  // DMA2 STREAMx Interrupt ENABLE
  NVIC_SetPriority(SD_SDIO_DMA_IRQn, 0);
  NVIC_EnableIRQ(SD_SDIO_DMA_IRQn);
}

static DSTATUS sdio_initialize(BYTE lun)
{
  /* SDIO Peripheral Low Level Init */
  sdio_low_level_init();

  /*!< Configure the SDIO peripheral */
  /*!< SDIO_CK = SDIOCLK / (SDIO_TRANSFER_CLK_DIV + 2) */
  /*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */
  sdio.Instance = SDIO;
  sdio.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  sdio.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  sdio.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  sdio.Init.BusWide = SDIO_BUS_WIDE_1B;
  sdio.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  sdio.Init.ClockDiv = SD_SDIO_TRANSFER_CLK_DIV;
  HAL_SD_DeInit(&sdio);

  HAL_StatusTypeDef halStatus = HAL_SD_Init(&sdio);
  if (halStatus != HAL_OK) {
    TRACE("HAL_SD_Init() status=%d", halStatus);
    /*!< CMD Response TimeOut (wait for CMDSENT flag) */
    return STA_NOINIT;
  }

  HAL_SD_CardInfoTypeDef cardInfo;
  HAL_StatusTypeDef es = HAL_SD_GetCardInfo(&sdio, &cardInfo);
  if(es != HAL_OK) {
    return STA_NOINIT;
  }

  HAL_SD_ConfigWideBusOperation(&sdio, SDIO_BUS_WIDE_4B);

  TRACE("SD card info:");
  TRACE("type: %u", (uint32_t)(cardInfo.CardType));
  TRACE("class: %u", (uint32_t)(cardInfo.Class));
  TRACE("block nbr size: %u", (uint32_t)(cardInfo.BlockNbr));
  TRACE("block size: %u", (uint32_t)(cardInfo.BlockSize));

  return RES_OK;
}

// DMA scratch buffer used in case the input buffer is not aligned
static uint8_t scratch[BLOCK_SIZE] __DMA;

typedef enum
{
  SD_TRANSFER_OK  = 0,
  SD_TRANSFER_BUSY = 1,
  SD_TRANSFER_ERROR
} SDTransferState;

static SDTransferState sdio_check_card_state()
{
  HAL_SD_CardStateTypeDef cardstate = HAL_SD_GetCardState(&sdio);

  if (cardstate == HAL_SD_CARD_TRANSFER) {
    return SD_TRANSFER_OK;
  }
  else if (cardstate == HAL_SD_CARD_ERROR) {
    return SD_TRANSFER_ERROR;
  }
  
  return SD_TRANSFER_BUSY;
}

static int sdio_check_card_state_with_timeout(uint32_t timeout)
{
  uint32_t timer = HAL_GetTick();
  /* block until SDIO IP is ready again or a timeout occur */
  while(HAL_GetTick() - timer < timeout) {
    auto state = sdio_check_card_state();
    if (state != SD_TRANSFER_BUSY) {
      return state == SD_TRANSFER_OK ? 0 : -1;
    }
  }

  return -1;
}

static DSTATUS sdio_status(BYTE lun)
{
  DSTATUS stat = RES_OK;

#if defined(SD_PRESENT_GPIO)
  if ((LL_GPIO_ReadInputPort(SD_PRESENT_GPIO) & SD_PRESENT_LL_GPIO_PIN) != 0) {
    stat |= STA_NODISK;
  }
#endif

  return stat;
}

static DRESULT _read_dma(BYTE* buff, DWORD sector, UINT count)
{
  ReadStatus = 0;
  if (HAL_SD_ReadBlocks_DMA(&sdio, buff, sector, count) != HAL_OK) {
    TRACE("SD ReadBlocks failed (s:%u/c:%u)", sector, (uint32_t)count);
    return RES_ERROR;
  }

  // Wait for the reading process to complete or a timeout to occur
  uint32_t timeout = HAL_GetTick();
  while((ReadStatus == 0) && ((HAL_GetTick() - timeout) < SD_TIMEOUT));

  if (ReadStatus == 0) {
    TRACE("SD read timeout (s:%u c:%u)", sector, (uint32_t)count);
    return RES_ERROR;
  }
      
  ReadStatus = 0;
  return RES_OK;
}

static DRESULT sdio_read(BYTE lun, BYTE * buff, DWORD sector, UINT count)
{
  DRESULT res = RES_OK;

  // TRACE("disk_read %d %p %10d %d", drv, buff, sector, count);

  if (sdio_check_card_state_with_timeout(SD_TIMEOUT) < 0) {
    return RES_ERROR;
  }

  if ((DWORD)buff < SRAM_BASE || ((DWORD)buff & 3)) {
    // TRACE("disk_read bad alignment (%p)", buff);
    while (count--) {
      res = _read_dma(scratch, sector++, 1);
      if (res != RES_OK) break;
      memcpy(buff, scratch, BLOCK_SIZE);
      buff += BLOCK_SIZE;
    }
    return res;
  }

  res = _read_dma(buff, sector, count);
  if (res != RES_OK) return res;

  uint32_t timeout = HAL_GetTick();
  while((HAL_GetTick() - timeout) < SD_TIMEOUT) {
    if (sdio_check_card_state() == SD_TRANSFER_OK) {
      return RES_OK;
    }
  }

  TRACE("SD getstatus timeout, s:%u c:%u", sector, (uint32_t)count);
  return RES_ERROR;
}

static DRESULT _write_dma(const BYTE *buff, DWORD sector, UINT count)
{
  WriteStatus = 0;
  if (HAL_SD_WriteBlocks_DMA(&sdio, (uint8_t*)buff, sector, count) != HAL_OK) {
    TRACE("SD WriteBlocks failed (s:%u/c:%u)", sector, (uint32_t)count);
    return RES_ERROR;
  }

  // Wait that the writing process is completed or a timeout occurs
  uint32_t timeout = HAL_GetTick();
  while((WriteStatus == 0) && ((HAL_GetTick() - timeout) < SD_TIMEOUT));

  if (WriteStatus == 0) {
    TRACE("SD write timeout (s:%u/c:%u)", sector, (uint32_t)count);
    return RES_ERROR;
  }

  WriteStatus = 0;
  return RES_OK;
}

static DRESULT sdio_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
  DRESULT res = RES_OK;

  // TRACE("disk_write %d %p %10d %d", drv, buff, sector, count);

  if (sdio_check_card_state_with_timeout(SD_TIMEOUT) < 0) {
    return RES_ERROR;
  }

  if ((DWORD)buff < SRAM_BASE || ((DWORD)buff & 3)) {
    //TRACE("disk_write bad alignment (%p)", buff);
    while(count--) {
      memcpy(scratch, buff, BLOCK_SIZE);
      res = _write_dma((BYTE *)scratch, sector++, 1);
      if (res != RES_OK) break;
      buff += BLOCK_SIZE;
 
      if (sdio_check_card_state_with_timeout(SD_TIMEOUT) < 0) {
        return RES_ERROR;
      }
    }
    return res;
  }

  res = _write_dma(buff, sector, count);
  if (res != RES_OK) return res;
  
  uint32_t timeout = HAL_GetTick();
  while((HAL_GetTick() - timeout) < SD_TIMEOUT) {
    if (sdio_check_card_state() == SD_TRANSFER_OK) {
      return RES_OK;
    }
  }

  TRACE("SD getstatus timeout, s:%u c: %u", sector, (uint32_t)count);
  return RES_ERROR;
}

static DRESULT sdio_get_sector_count(DWORD* sectors)
{
  HAL_SD_CardInfoTypeDef cardInfo;
  if(HAL_SD_GetCardInfo(&sdio, &cardInfo) != HAL_OK) {
    return RES_ERROR;
  }

  *sectors = cardInfo.LogBlockNbr;
  return RES_OK;
}

static DRESULT sdio_get_sector_size(DWORD* sector_size)
{
  HAL_SD_CardInfoTypeDef cardInfo;
  if(HAL_SD_GetCardInfo(&sdio, &cardInfo) != HAL_OK) {
    return RES_ERROR;
  }

  *sector_size = cardInfo.LogBlockSize;
  return RES_OK;
}

static DRESULT sdio_get_block_size(DWORD* block_size)
{
  DWORD sector_size;
  DRESULT res = sdio_get_sector_size(&sector_size);
  if (res == RES_OK) {
    *block_size = sector_size / BLOCK_SIZE;
  }
  return res;
}

DRESULT sdio_ioctl(BYTE lun, BYTE ctrl, void *buff)
{
  DRESULT res = RES_OK;
  switch (ctrl) {
    case GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
      res = sdio_get_sector_count((DWORD*)buff);
      break;

    case GET_SECTOR_SIZE :  /* Get R/W sector size (WORD) */
      res = sdio_get_sector_size((DWORD*)buff);
      break;

    case GET_BLOCK_SIZE :   /* Get erase block size in unit of sector (DWORD) */
      res = sdio_get_block_size((DWORD*)buff);
      break;

    case CTRL_SYNC:
      /* Complete pending write process */
      while (sdio_check_card_state() == SD_TRANSFER_BUSY);
      break;

    default:
      break;
  }

  return res;
}

const diskio_driver_t sdio_diskio_driver = {
  .initialize = sdio_initialize,
  .status = sdio_status,
  .read = sdio_read,
  .write = sdio_write,
  .ioctl = sdio_ioctl,
};

/**
* @brief Tx Transfer completed callbacks
* @param hsd: SD handle
* @retval None
*/

extern "C" void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
  UNUSED(hsd);
  WriteStatus = 1;
}

/**
* @brief Rx Transfer completed callbacks
* @param hsd: SD handle
* @retval None
*/

extern "C" void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
  UNUSED(hsd);
  ReadStatus = 1;
}

extern "C" void SDIO_IRQHandler(void)
{
  DEBUG_INTERRUPT(INT_SDIO);
  HAL_SD_IRQHandler(&sdio);
}
extern "C" void SD_SDIO_DMA_IRQHANDLER(void)
{
  DEBUG_INTERRUPT(INT_SDIO_DMA);
  HAL_DMA_IRQHandler(&sdioTxDma);
}
#endif

