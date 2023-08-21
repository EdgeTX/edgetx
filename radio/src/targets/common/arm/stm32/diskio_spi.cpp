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

#include "stm32_gpio_driver.h"
#include "sdcard_spi.h"

#include "FatFs/diskio.h"

#include <string.h>
#include "debug.h"

#include "hal.h"

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
};

static sdcard_spi_t _sdcard_spi = {nullptr, false};
static uint32_t     _sdcard_sectors;

static void _sd_present_gpio_init()
{
#if defined(SD_PRESENT_GPIO)
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin = SD_PRESENT_GPIO_PIN;
  pinInit.Mode = LL_GPIO_MODE_INPUT;
  pinInit.Pull = LL_GPIO_PULL_UP;

  stm32_gpio_enable_clock(SD_PRESENT_GPIO);
  LL_GPIO_Init(SD_PRESENT_GPIO, &pinInit);
#endif
}

bool sdcard_present()
{
#if defined(SD_PRESENT_GPIO)
  return !LL_GPIO_IsInputPinSet(SD_PRESENT_GPIO, SD_PRESENT_GPIO_PIN);
#else
  return true;
#endif
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
  BYTE drv                                /* Physical drive nmuber (0..) */
)
{
  /* Supports only single drive */
  if (drv > 0) {
    return STA_NOINIT | STA_NODISK;
  }

  if (!sdcard_present()) {
    return STA_NODISK;
  }

  if (_sdcard_spi.spi != nullptr) {
    return 0;
  }

  _sd_present_gpio_init();

  sdcard_info_t card_info;
  if (sdcard_spi_init(&_sd_spi_hw, &card_info) != SDCARD_SPI_OK) {
    return STA_NOINIT;
  }

  _sdcard_spi.spi = &_sd_spi_hw;
  _sdcard_spi.use_block_addr = card_info.use_block_addr;
  _sdcard_sectors = sdcard_spi_get_sector_count(&card_info);

  return 0;
}

/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
  BYTE drv                /* Physical drive nmuber (0..) */
)
{
  DSTATUS stat = 0;

  /* Supports only single drive */
  if (drv > 0) {
    return STA_NOINIT | STA_NODISK;
  }

  if (!sdcard_present()) {
    stat |= STA_NODISK;
  }

  if (_sdcard_spi.spi == nullptr) {
    stat |= STA_NOINIT;
  }

  return stat;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read(BYTE drv, BYTE * buff, DWORD sector, UINT count)
{
  // TRACE("disk_read %d %p %10d %d", drv, buff, sector, count);
  if (!sdcard_present()) {
    return RES_NOTRDY;
  }

  sd_rw_response_t state;
  sdcard_spi_read_blocks(&_sdcard_spi, sector, buff, SD_HC_BLOCK_SIZE, count, &state);
  if (state == SD_RW_OK) {
    return RES_OK;
  }

  return RES_ERROR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT __disk_write(
  BYTE drv,                       /* Physical drive nmuber (0..) */
  const BYTE *buff,               /* Data to be written */
  DWORD sector,                   /* Sector address (LBA) */
  UINT count                      /* Number of sectors to write (1..255) */
)
{
  if (!sdcard_present()) {
    return RES_NOTRDY;
  }

  sd_rw_response_t state;
  sdcard_spi_write_blocks(&_sdcard_spi, sector, buff, SD_HC_BLOCK_SIZE, count, &state);
  if (state == SD_RW_OK) {
    return RES_OK;
  }

  return RES_ERROR;
}
#endif /* _READONLY */

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
  BYTE drv,               /* Physical drive nmuber (0..) */
  BYTE ctrl,              /* Control code */
  void *buff              /* Buffer to send/receive control data */
)
{
  if (drv > 0) return RES_PARERR;

  DRESULT res = RES_ERROR;

  switch (ctrl) {
    case GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
      *(DWORD*)buff = _sdcard_sectors;
      res = RES_OK;
      break;

    case GET_SECTOR_SIZE :  /* Get R/W sector size (WORD) */
      *(WORD*)buff = SD_HC_BLOCK_SIZE;
      res = RES_OK;
      break;

    // case CTRL_SYNC:
    //   /* Complete pending write process (needed at _FS_READONLY == 0) */
    //   while (SD_GetStatus() == SD_TRANSFER_BUSY);
    //   res = RES_OK;
    //   break;

    default:
      res = RES_OK;
      break;

  }

  return res;
}
