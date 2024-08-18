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
#include "hal/gpio.h"

#include "stm32_dma.h"
#include "stm32_hal.h"
#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"
#include "stm32_gpio.h"

#include "hal.h"


#if defined(STORAGE_USE_SDIO)

// #include "delays_driver.h"
#include "debug.h"

#include <string.h>

#if !defined(SD_SDIO_PIN_D0)
#  define SD_SDIO_PIN_D0 GPIO_PIN(GPIOC, 8)
#  if defined(STM32H7RS)
#    define SD_SDIO_AF_D0  GPIO_AF11
#  else
#    define SD_SDIO_AF_D0  GPIO_AF12
#  endif
#endif

#if !defined(SD_SDIO_PIN_D1)
#  define SD_SDIO_PIN_D1 GPIO_PIN(GPIOC, 9)
#  if defined(STM32H7RS)
#    define SD_SDIO_AF_D1  GPIO_AF11
#  else
#    define SD_SDIO_AF_D1  GPIO_AF12
#  endif
#endif

#if !defined(SD_SDIO_PIN_D2)
#  define SD_SDIO_PIN_D2 GPIO_PIN(GPIOC, 10)
#  define SD_SDIO_AF_D2  GPIO_AF12
#endif

#if !defined(SD_SDIO_PIN_D3)
#  define SD_SDIO_PIN_D3 GPIO_PIN(GPIOC, 11)
#  if defined(STM32H7RS)
#    define SD_SDIO_AF_D3  GPIO_AF11
#  else
#    define SD_SDIO_AF_D3  GPIO_AF12
#  endif
#endif

#if !defined(SD_SDIO_PIN_CMD)
#  define SD_SDIO_PIN_CMD GPIO_PIN(GPIOD, 2)
#  if defined(STM32H7RS)
#    define SD_SDIO_AF_CMD  GPIO_AF11
#  else
#    define SD_SDIO_AF_CMD  GPIO_AF12
#  endif
#endif

#if !defined(SD_SDIO_PIN_CLK)
#  define SD_SDIO_PIN_CLK GPIO_PIN(GPIOC, 12)
#  if defined(STM32H7RS)
#    define SD_SDIO_AF_CLK  GPIO_AF11
#  else
#    define SD_SDIO_AF_CLK  GPIO_AF12
#  endif
#endif


#if defined(SDMMC1) && defined(SD_SDIO)
#  if !defined(SD_SDIO_IRQHandler)
#    define SD_SDIO_IRQHandler SDMMC1_IRQHandler
#    define SD_SDIO_IRQn       SDMMC1_IRQn
#  endif
#  define SD_SDIO_TypeDef SDMMC_TypeDef
#  define SD_SDIO_CLOCK_EDGE_RISING SDMMC_CLOCK_EDGE_RISING
#  define SD_SDIO_CLOCK_POWER_SAVE_DISABLE SDMMC_CLOCK_POWER_SAVE_DISABLE
#  define SD_SDIO_BUS_WIDE_1B SDMMC_BUS_WIDE_1B
#  define SD_SDIO_BUS_WIDE_4B SDMMC_BUS_WIDE_4B
#  define SD_SDIO_HARDWARE_FLOW_CONTROL_DISABLE SDMMC_HARDWARE_FLOW_CONTROL_DISABLE
#elif !defined(SDMMC1)
#  if !defined(SD_SDIO)
#    define SD_SDIO SDIO
#  endif
#  define _SD_SDIO_CLK_ENABLE()  __HAL_RCC_SDIO_CLK_ENABLE()
#  define _SD_SDIO_CLK_DISABLE()  __HAL_RCC_SDIO_CLK_DISABLE()
#  define SD_SDIO_IRQHandler SDIO_IRQHandler
#  define SD_SDIO_IRQn       SDIO_IRQn
#  define SD_SDIO_TypeDef    SDIO_TypeDef
#  define SD_SDIO_CLOCK_EDGE_RISING SDIO_CLOCK_EDGE_RISING
#  define SD_SDIO_CLOCK_POWER_SAVE_DISABLE SDIO_CLOCK_POWER_SAVE_DISABLE
#  define SD_SDIO_CLOCK_BYPASS_DISABLE SDIO_CLOCK_BYPASS_DISABLE
#  define SD_SDIO_BUS_WIDE_1B SDIO_BUS_WIDE_1B
#  define SD_SDIO_BUS_WIDE_4B SDIO_BUS_WIDE_4B
#  define SD_SDIO_HARDWARE_FLOW_CONTROL_DISABLE SDIO_HARDWARE_FLOW_CONTROL_DISABLE
#endif

#if FF_MAX_SS != FF_MIN_SS
#error "Variable sector size is not supported"
#endif

#define BLOCK_SIZE FF_MAX_SS
#define SD_TIMEOUT 300 /* 300ms */

#if defined(STM32H7) || defined(STM32H7RS) || defined(STM32F4)
extern uint32_t _sram;
extern uint32_t _heap_start;
#if defined(SDRAM)
#define _IS_DMA_BUFFER(addr)                                           \
  (((((intptr_t)(addr)) & 0xFF000000) == (((intptr_t)&_sram) & 0xFF000000)) || \
   ((((intptr_t)(addr)) & 0xFC000000) == (((intptr_t)&_heap_start) & 0xFC000000)))
#else
#define _IS_DMA_BUFFER(addr) \
  ((((intptr_t)(addr)) & 0xFF000000) == (((intptr_t)&_sram) & 0xFF000000))
#endif
#else
#define _IS_DMA_BUFFER(addr) (true)
#endif

#define _IS_ALIGNED(addr) (((intptr_t)(addr) & 3U) == 0U)

// HAL state
static SD_HandleTypeDef sdio;
#if defined(SD_SDIO_DMA)
static DMA_HandleTypeDef sdioTxDma;
#endif

// Disk status
static volatile uint32_t WriteStatus;
static volatile uint32_t ReadStatus;

static void _sd_sdio_clk_enable(SD_SDIO_TypeDef* periph)
{
#if defined(SDMMC1)
  if (periph == SDMMC1) {
    __HAL_RCC_SDMMC1_CLK_ENABLE();
  } else if(periph == SDMMC2) {
    __HAL_RCC_SDMMC2_CLK_ENABLE();
  }
#else
  (void)periph;
  __HAL_RCC_SDIO_CLK_ENABLE();
#endif
}

static void sdio_low_level_init(void)
{
  _sd_sdio_clk_enable(SD_SDIO);

  // data pins
  gpio_init(SD_SDIO_PIN_D0, GPIO_IN_PU, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_set_af(SD_SDIO_PIN_D0, SD_SDIO_AF_D0);

  gpio_init(SD_SDIO_PIN_D1, GPIO_IN_PU, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_set_af(SD_SDIO_PIN_D1, SD_SDIO_AF_D1);
  
  gpio_init(SD_SDIO_PIN_D2, GPIO_IN_PU, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_set_af(SD_SDIO_PIN_D2, SD_SDIO_AF_D2);

  gpio_init(SD_SDIO_PIN_D3, GPIO_IN_PU, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_set_af(SD_SDIO_PIN_D3, SD_SDIO_AF_D3);

  gpio_init(SD_SDIO_PIN_CMD, GPIO_IN_PU, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_set_af(SD_SDIO_PIN_CMD, SD_SDIO_AF_CMD);
  
  gpio_init(SD_SDIO_PIN_CLK, GPIO_IN_PU, GPIO_PIN_SPEED_VERY_HIGH);
  gpio_set_af(SD_SDIO_PIN_CLK, SD_SDIO_AF_CLK);

#if defined(SD_SDIO_HAS_TRANSCEIVER)
  gpio_init_af(SD_SDIO_PIN_CKIN, SD_SDIO_AF_CKIN, GPIO_PIN_SPEED_HIGH);
  gpio_init_af(SD_SDIO_PIN_CDIR, SD_SDIO_AF_CDIR, GPIO_PIN_SPEED_HIGH);
  gpio_init_af(SD_SDIO_PIN_D0DIR, SD_SDIO_AF_D0DIR, GPIO_PIN_SPEED_HIGH);
  gpio_init_af(SD_SDIO_PIN_D123DIR, SD_SDIO_AF_D123DIR, GPIO_PIN_SPEED_HIGH);
#endif

  // SDIO Interrupt ENABLE
  NVIC_SetPriority(SD_SDIO_IRQn, 0);
  NVIC_EnableIRQ(SD_SDIO_IRQn);

#if defined(SD_SDIO_DMA)
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
#endif
}

HAL_SD_CardInfoTypeDef cardInfo;
static DSTATUS sdio_initialize(BYTE lun)
{
  /* SDIO Peripheral Low Level Init */
  sdio_low_level_init();

  /*!< Configure the SDIO peripheral */
  /*!< SDIO_CK = SDIOCLK / (SDIO_TRANSFER_CLK_DIV + 2) */
  /*!< on STM32F4xx devices, SDIOCLK is fixed to 48MHz */

  sdio.Instance = SD_SDIO;
  sdio.Init.ClockEdge = SD_SDIO_CLOCK_EDGE_RISING;
  sdio.Init.ClockPowerSave = SD_SDIO_CLOCK_POWER_SAVE_DISABLE;
  sdio.Init.BusWide = SD_SDIO_BUS_WIDE_1B;
  sdio.Init.HardwareFlowControl = SD_SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
#if ( USE_SD_TRANSCEIVER > 0U )
  sdio.Init.TranceiverPresent = SDMMC_TRANSCEIVER_PRESENT;
#endif
#if defined(SD_SDIO_CLOCK_BYPASS_DISABLE)
  sdio.Init.ClockBypass = SD_SDIO_CLOCK_BYPASS_DISABLE;
#endif
  sdio.Init.ClockDiv = SD_SDIO_TRANSFER_CLK_DIV;

  HAL_StatusTypeDef halStatus = HAL_SD_Init(&sdio);
  if (halStatus != HAL_OK) {
    TRACE("HAL_SD_Init() status=%d", halStatus);
    /*!< CMD Response TimeOut (wait for CMDSENT flag) */
    return STA_NOINIT;
  }

  HAL_StatusTypeDef es = HAL_SD_GetCardInfo(&sdio, &cardInfo);
  if(es != HAL_OK) {
    return STA_NOINIT;
  }

  halStatus = HAL_SD_ConfigWideBusOperation(&sdio, SD_SDIO_BUS_WIDE_4B);
  if (halStatus != HAL_OK) {
    TRACE("HAL_SD_ConfigWideBusOperation() status=%d", halStatus);
  } else {
    TRACE("SD Bus width successfully set to 4 bits");
  }

  TRACE("SD card info:");
  TRACE("type: %u", (uint32_t)(cardInfo.CardType));
  TRACE("class: %u", (uint32_t)(cardInfo.Class));
  TRACE("block nbr size: %u", (uint32_t)(cardInfo.BlockNbr));
  TRACE("block size: %u", (uint32_t)(cardInfo.BlockSize));

  return RES_OK;
}

// #if defined(SRAM_BASE)
// DMA scratch buffer used in case the input buffer is not aligned
static uint8_t scratch[BLOCK_SIZE] __DMA;
// #endif

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
  if (gpio_read(SD_PRESENT_GPIO)) {
    stat |= STA_NODISK;
  }
#endif

  return stat;
}

static DRESULT _read_dma(BYTE* buff, DWORD sector, UINT count)
{
  ReadStatus = 0;
#if __CORTEX_M >= 0x07
  SCB_CleanInvalidateDCache();
#endif

  if (HAL_SD_ReadBlocks_DMA(&sdio, buff, sector, count) != HAL_OK) {
    TRACE("SD ReadBlocks failed (s:%u/c:%u)", sector, (uint32_t)count);
    return RES_ERROR;
  }

#if __CORTEX_M >= 0x07
  SCB_CleanInvalidateDCache();
#endif

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

  if (!_IS_DMA_BUFFER(buff) || !_IS_ALIGNED(buff)) {
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

  if (sdio_check_card_state_with_timeout(SD_TIMEOUT) < 0) {
    TRACE("SD getstatus timeout, s:%u c:%u", sector, (uint32_t)count);
    return RES_ERROR;
  }

  return RES_OK;
}

static DRESULT _write_dma(const BYTE *buff, DWORD sector, UINT count)
{
#if __CORTEX_M >= 0x07
  SCB_CleanDCache();
#endif

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

  if (!_IS_DMA_BUFFER(buff) || !_IS_ALIGNED(buff)) {
    while (count--) {
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

  if (sdio_check_card_state_with_timeout(SD_TIMEOUT) < 0) {
    TRACE("SD getstatus timeout, s:%u c: %u", sector, (uint32_t)count);
    return RES_ERROR;
  }

  return RES_OK;
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

extern "C" void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
  UNUSED(hsd);
  WriteStatus = 2;
}

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

extern "C" void SD_SDIO_IRQHandler(void)
{
  DEBUG_INTERRUPT(INT_SDIO);
  HAL_SD_IRQHandler(&sdio);
}

#if defined(SD_SDIO_DMA)
extern "C" void SD_SDIO_DMA_IRQHANDLER(void)
{
  DEBUG_INTERRUPT(INT_SDIO_DMA);
  HAL_DMA_IRQHandler(&sdioTxDma);
}
#endif

#endif
