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

/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "stm32_hal.h"
#include "sdio_sd.h"

#include "FatFs/diskio.h"

#include <string.h>
#include "debug.h"

#if FF_MAX_SS != FF_MIN_SS
#error "Variable sector size is not supported"
#endif

#define BLOCK_SIZE FF_MAX_SS
#define SD_TIMEOUT 300 /* 300ms */

// Disk status
extern volatile uint32_t WriteStatus;
extern volatile uint32_t ReadStatus;

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
  BYTE drv                                /* Physical drive nmuber (0..) */
)
{
  DSTATUS stat = 0;

  /* Supports only single drive */
  if (drv)
  {
    stat |= STA_NOINIT;
  }

  /*-------------------------- SD Init ----------------------------- */
  SD_Error res = SD_Init();
  if (res != SD_OK)
  {
    TRACE("SD_Init() failed: %d", res);
    stat |= STA_NOINIT;
  }

  TRACE("SD card info:");
  TRACE("type: %u", (uint32_t)(SD_GetCardType()));
  TRACE("class: %u", (uint32_t)(SD_GetCardClass()));
  TRACE("sectors: %u", (uint32_t)(SD_GetSectorCount()));
  TRACE("sector size: %u", (uint32_t)(SD_GetSectorSize()));
  TRACE("block size: %u", (uint32_t)(SD_GetBlockSize()));

  return(stat);
}

DWORD scratch[BLOCK_SIZE / 4] __DMA;

/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
  BYTE drv                /* Physical drive nmuber (0..) */
)
{
  DSTATUS stat = 0;

  if (SD_Detect() != SD_PRESENT)
    stat |= STA_NODISK;

  // STA_NOTINIT - Subsystem not initailized
  // STA_PROTECTED - Write protected, MMC/SD switch if available

  return(stat);
}

uint32_t sdReadRetries = 0;

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */


// Please note:
//   this functions assumes that buff is properly aligned
//   and in the right RAM segment for DMA
//
DRESULT disk_read_dma(BYTE drv, BYTE * buff, DWORD sector, UINT count)
{
  DRESULT res = RES_ERROR;

  for (int retry = 0; retry < 3; retry++) {

    ReadStatus = 0;
    SD_Error Status = SD_ReadBlocks(buff, sector, BLOCK_SIZE, count);
    if (Status != SD_OK) {
      TRACE("SD ReadBlocks=%d, s:%u c: %u", Status, sector, (uint32_t)count);
      ++sdReadRetries;
      continue;
    }

    // Wait that the reading process is completed or a timeout occurs
    uint32_t timeout = HAL_GetTick();
    while((ReadStatus == 0) && ((HAL_GetTick() - timeout) < SD_TIMEOUT));

    if (ReadStatus == 0) {
      TRACE("SD read timeout, s:%u c:%u", sector, (uint32_t)count);
      ++sdReadRetries;
      continue;
    }
      
    ReadStatus = 0;
    timeout = HAL_GetTick();

    while((HAL_GetTick() - timeout) < SD_TIMEOUT) {
      if (SD_GetStatus() == SD_TRANSFER_OK) {
	res = RES_OK;
	break;
      }
    }

    if (res == RES_OK) {
      // exit retry loop
      break;
    }

    TRACE("SD getstatus timeout, s:%u c:%u", sector, (uint32_t)count);
  }

  return res;
}

DRESULT __disk_read(BYTE drv, BYTE * buff, DWORD sector, UINT count)
{
  // If unaligned, do the single block reads with a scratch buffer.
  // If aligned and single sector, do a single block read.
  // If aligned and multiple sectors, try multi block read.
  //    If multi block read fails, try single block reads without
  //    an intermediate buffer (move trough the provided buffer)

  // TRACE("disk_read %d %p %10d %d", drv, buff, sector, count);
  if (SD_Detect() != SD_PRESENT) {
    TRACE("SD_Detect() != SD_PRESENT");
    return RES_NOTRDY;
  }

  if (SD_CheckStatusWithTimeout(SD_TIMEOUT) < 0) {
    return RES_ERROR;
  }
  
  DRESULT res = RES_OK;
  if (count == 0) return res;

  if ((DWORD)buff < 0x20000000 || ((DWORD)buff & 3)) {
    // buffer is not aligned, use scratch buffer that is aligned
    TRACE("disk_read bad alignment (%p)", buff);
    while (count--) {
      res = disk_read_dma(drv, (BYTE *)scratch, sector++, 1);
      if (res != RES_OK) break;
      memcpy(buff, scratch, BLOCK_SIZE);
      buff += BLOCK_SIZE;
    }
    return res;
  }

  res = disk_read_dma(drv, buff, sector, count);
  if (res != RES_OK && count > 1) {
    // multi-read failed, try reading same sectors, one by one
    TRACE("disk_read() multi-block failed, trying single block reads...");
    while (count--) {
      res = disk_read_dma(drv, buff, sector++, 1);
      if (res != RES_OK) break;
      buff += BLOCK_SIZE;
    }
  }

  return res;
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
  DRESULT res = RES_OK;

  // TRACE("disk_write %d %p %10d %d", drv, buff, sector, count);

  if (SD_Detect() != SD_PRESENT)
    return RES_NOTRDY;

  if (SD_CheckStatusWithTimeout(SD_TIMEOUT) < 0) {
    return RES_ERROR;
  }

  if ((DWORD)buff < 0x20000000 || ((DWORD)buff & 3)) {
    //TRACE("disk_write bad alignment (%p)", buff);
    while(count--) {
      memcpy(scratch, buff, BLOCK_SIZE);

      res = __disk_write(drv, (BYTE *)scratch, sector++, 1);

      if (res != RES_OK)
        break;

      buff += BLOCK_SIZE;
    }
    return(res);
  }

  SD_Error Status = SD_WriteBlocks((uint8_t *)buff, sector, BLOCK_SIZE, count);
  if (Status != SD_OK) {
    TRACE("SD WriteBlocks=%d, s:%u c: %u", Status, sector, (uint32_t)count);
    return RES_ERROR;
  }

  // Wait that the reading process is completed or a timeout occurs
  uint32_t timeout = HAL_GetTick();
  while((WriteStatus == 0) && ((HAL_GetTick() - timeout) < SD_TIMEOUT));

  if (WriteStatus == 0) {
    TRACE("SD write timeout, s:%u c:%u", sector, (uint32_t)count);
    return RES_ERROR;
  }

  WriteStatus = 0;
  res = RES_ERROR;
  timeout = HAL_GetTick();

  while((HAL_GetTick() - timeout) < SD_TIMEOUT) {
    if (SD_GetStatus() == SD_TRANSFER_OK) {
      res = RES_OK;
      break;
    }
  }

  if (res != RES_OK) {
    TRACE("SD getstatus timeout, s:%u c: %u", sector, (uint32_t)count);
    res = RES_ERROR;
  }

  return res;
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
  DRESULT res;
  uint32_t tmp;

  if (drv) return RES_PARERR;

  res = RES_ERROR;

  switch (ctrl) {
    case GET_SECTOR_COUNT : /* Get number of sectors on the disk (DWORD) */
      tmp = SD_GetSectorCount();

      if(tmp == 0) {
        res = RES_ERROR;
        break;
      }

      *(DWORD*)buff = tmp;
      res = RES_OK;
      break;

    case GET_SECTOR_SIZE :  /* Get R/W sector size (WORD) */
      tmp = SD_GetSectorSize();

      if(tmp == 0) {
        res = RES_ERROR;
        break;
      }

      *(WORD*)buff = tmp;
      res = RES_OK;
      break;

    case GET_BLOCK_SIZE :   /* Get erase block size in unit of sector (DWORD) */
      tmp = SD_GetBlockSize() / BLOCK_SIZE;

      if(tmp == 0) {
        res = RES_ERROR;
        break;
      }

      *(DWORD*)buff = tmp;
      res = RES_OK;
      break;

    case CTRL_SYNC:
      /* Complete pending write process (needed at _FS_READONLY == 0) */
      while (SD_GetStatus() == SD_TRANSFER_BUSY);
      res = RES_OK;
      break;

    default:
      res = RES_OK;
      break;

  }

  return res;
}
