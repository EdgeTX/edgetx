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

/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio_spi.h"
#include "stm32_gpio_driver.h"
#include "stm32_gpio.h"
#include "hal/gpio.h"
#include "sdcard_spi.h"

#include "hal.h"

#if defined(SD_SPI)

#include <string.h>
#include "debug.h"

static const stm32_spi_t _sd_spi_hw = {
    .SPIx = SD_SPI,
    .SPI_GPIOx = SD_GPIO,
    .SPI_Pins = SD_GPIO_PIN_SCK | SD_GPIO_PIN_MISO | SD_GPIO_PIN_MOSI,
    .CS_GPIOx = SD_GPIO,
    .CS_Pin = SD_GPIO_PIN_CS,
    .DMA = SD_SPI_DMA,
    .DMA_Channel = SD_SPI_DMA_CHANNEL,
    .txDMA_Stream = SD_SPI_DMA_TX_STREAM,
    .rxDMA_Stream = SD_SPI_DMA_RX_STREAM,
    .DMA_FIFOMode = LL_DMA_FIFOMODE_ENABLE,
    .DMA_FIFOThreshold = LL_DMA_FIFOTHRESHOLD_1_2,
    .DMA_MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD,
    .DMA_MemBurst = LL_DMA_MBURST_INC4,
};

static uint32_t _sdcard_sectors;

static DSTATUS sdcard_spi_initialize(BYTE lun)
{
  sdcard_info_t card_info;
  if (sdcard_spi_init(&_sd_spi_hw, &card_info) != SDCARD_SPI_OK) {
    return STA_NOINIT;
  }

  _sdcard_sectors = sdcard_spi_get_sector_count(&card_info);

  return 0;
}

static DSTATUS sdcard_spi_status(BYTE lun)
{
  DSTATUS stat = 0;

#if defined(SD_PRESENT_GPIO)
  if (gpio_read(SD_PRESENT_GPIO)) {
    stat |= STA_NODISK;
  }
#endif

  return stat;
}

static DRESULT sdcard_spi_read(BYTE lun, BYTE * buff, DWORD sector, UINT count)
{
  // TRACE("disk_read %d %p %10d %d", lun, buff, sector, count);

  sd_rw_response_t state;
  sdcard_spi_read_blocks(sector, buff, SD_HC_BLOCK_SIZE, count, &state);
  if (state == SD_RW_OK) {
    return RES_OK;
  }

  return RES_ERROR;
}

static DRESULT sdcard_spi_write(BYTE lun, const BYTE* buff, DWORD sector, UINT count)
{
  // TRACE("disk_write %d %p %10d %d", lun, buff, sector, count);

  sd_rw_response_t state;
  sdcard_spi_write_blocks(sector, buff, SD_HC_BLOCK_SIZE, count, &state);
  if (state == SD_RW_OK) {
    return RES_OK;
  }

  return RES_ERROR;
}

static DRESULT sdcard_spi_ioctl(BYTE lun, BYTE ctrl, void *buff)
{
  DRESULT res = RES_OK;

  switch (ctrl) {
    case GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
      *(DWORD*)buff = _sdcard_sectors;
      break;

    case GET_SECTOR_SIZE :  /* Get R/W sector size (WORD) */
      *(WORD*)buff = SD_HC_BLOCK_SIZE;
      break;

    case CTRL_SYNC:
      sdcard_spi_wait_for_not_busy();
      break;

    default:
      break;
  }

  return res;
}

const diskio_driver_t sdcard_spi_driver = {
  .initialize = sdcard_spi_initialize,
  .status = sdcard_spi_status,
  .read = sdcard_spi_read,
  .write = sdcard_spi_write,
  .ioctl = sdcard_spi_ioctl,
};

#endif
