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

#if defined(SIMU_DISKIO)
#include "edgetx.h"
#include "ff.h"
#include "diskio.h"
#include <time.h>
#include <stdio.h>
#include <sys/stat.h>

FILE * diskImage = 0;

bool _g_FATFS_init = false;

RTOS_MUTEX_HANDLE ioMutex;

int ff_cre_syncobj (BYTE vol, FF_SYNC_t* sobj) /* Create a sync object */
{
  pthread_mutex_init(&ioMutex, 0);
  return 1;
}

int ff_req_grant (FF_SYNC_t sobj)        /* Lock sync object */
{
  pthread_mutex_lock(&ioMutex);
  return 1;
}

void ff_rel_grant (FF_SYNC_t sobj)        /* Unlock sync object */
{
  pthread_mutex_unlock(&ioMutex);
}

int ff_del_syncobj (FF_SYNC_t sobj)        /* Delete a sync object */
{
  pthread_mutex_destroy(&ioMutex);
  return 1;
}

DWORD get_fattime (void)
{
  time_t tim = time(0);
  const struct tm * t = gmtime(&tim);

  /* Pack date and time into a DWORD variable */
  return ((DWORD)(t->tm_year - 80) << 25)
    | ((uint32_t)(t->tm_mon+1) << 21)
    | ((uint32_t)t->tm_mday << 16)
    | ((uint32_t)t->tm_hour << 11)
    | ((uint32_t)t->tm_min << 5)
    | ((uint32_t)t->tm_sec >> 1);
}

unsigned int noDiskStatus = 0;

void traceDiskStatus()
{
  if (noDiskStatus > 0) {
    TRACE_SIMPGMSPACE("disk_status() called %d times", noDiskStatus);
    noDiskStatus = 0;
  }
}

DSTATUS disk_initialize (BYTE pdrv)
{
  traceDiskStatus();
  TRACE_SIMPGMSPACE("disk_initialize(%u)", pdrv);
  diskImage = fopen("sdcard.image", "rb+");
  return diskImage ? (DSTATUS)0 : (DSTATUS)STA_NODISK;
}

DSTATUS disk_status (BYTE pdrv)
{
  ++noDiskStatus;
  // TRACE_SIMPGMSPACE("disk_status(%u)", pdrv);
  return (DSTATUS)0;
}

DRESULT disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
  if (diskImage == 0) return RES_NOTRDY;
  traceDiskStatus();
  TRACE_SIMPGMSPACE("disk_read(%u, %p, %u, %u)", pdrv, buff, sector, count);
  fseek(diskImage, sector*512, SEEK_SET);
  fread(buff, count, 512, diskImage);
  return RES_OK;
}

DRESULT disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
  if (diskImage == 0) return RES_NOTRDY;
  traceDiskStatus();
  TRACE_SIMPGMSPACE("disk_write(%u, %p, %u, %u)", pdrv, buff, sector, count);
  fseek(diskImage, sector*512, SEEK_SET);
  fwrite(buff, count, 512, diskImage);
  return RES_OK;
}

DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff)
{
  if (diskImage == 0) return RES_NOTRDY;
  traceDiskStatus();
  TRACE_SIMPGMSPACE("disk_ioctl(%u, %u, %p)", pdrv, cmd, buff);
  if (pdrv) return RES_PARERR;

  DRESULT res;
  BYTE *ptr = (BYTE *)buff;

  if (cmd == CTRL_POWER) {
    switch (*ptr) {
      case 0:         /* Sub control code == 0 (POWER_OFF) */
        res = RES_OK;
        break;
      case 1:         /* Sub control code == 1 (POWER_ON) */
        res = RES_OK;
        break;
      case 2:         /* Sub control code == 2 (POWER_GET) */
        *(ptr+1) = (BYTE)1;  /* fake powered */
        res = RES_OK;
        break;
      default :
        res = RES_PARERR;
    }
    return res;
  }

  switch(cmd) {
/* Generic command (Used by FatFs) */
    case CTRL_SYNC :     /* Complete pending write process (needed at _FS_READONLY == 0) */
      break;

    case GET_SECTOR_COUNT: /* Get media size (needed at _USE_MKFS == 1) */
      {
        struct stat buf;
        if (stat("sdcard.image", &buf) == 0) {
          DWORD noSectors  = buf.st_size / 512;
          *(DWORD*)buff = noSectors;
          TRACE_SIMPGMSPACE("disk_ioctl(GET_SECTOR_COUNT) = %u", noSectors);
          return RES_OK;
        }
        return RES_ERROR;
      }

    case GET_SECTOR_SIZE: /* Get sector size (needed at _MAX_SS != _MIN_SS) */
      TRACE_SIMPGMSPACE("disk_ioctl(GET_SECTOR_SIZE) = 512");
      *(WORD*)buff = 512;
      res = RES_OK;
      break;

    case GET_BLOCK_SIZE : /* Get erase block size (needed at _USE_MKFS == 1) */
      *(WORD*)buff = 512 * 4;
      res = RES_OK;
      break;

    case CTRL_TRIM : /* Inform device that the data on the block of sectors is no longer used (needed at _USE_TRIM == 1) */
      break;

/* Generic command (Not used by FatFs) */
    case CTRL_LOCK : /* Lock/Unlock media removal */
    case CTRL_EJECT: /* Eject media */
    case CTRL_FORMAT: /* Create physical format on the media */
      return RES_PARERR;


/* MMC/SDC specific ioctl command */
    // case MMC_GET_TYPE    10  /* Get card type */
    // case MMC_GET_CSD     11  /* Get CSD */
    // case MMC_GET_CID     12  /* Get CID */
    // case MMC_GET_OCR     13  /* Get OCR */
    // case MMC_GET_SDSTAT    14  /* Get SD status */

/* ATA/CF specific ioctl command */
    // case ATA_GET_REV     20  /* Get F/W revision */
    // case ATA_GET_MODEL   21  /* Get model name */
    // case ATA_GET_SN      22  /* Get serial number */
    default:
      return RES_PARERR;
  }
  return RES_OK;
}

void sdInit(void)
{
  // ioMutex = CoCreateMutex();
  // if (ioMutex >= CFG_MAX_MUTEX ) {
  //   // sd error
  //   return;
  // }

  if (f_mount(&g_FATFS_Obj, "", 1) == FR_OK) {
    // call sdGetFreeSectors() now because f_getfree() takes a long time first time it's called
    sdGetFreeSectors();

#if defined(LOG_TELEMETRY)
    f_open(&g_telemetryFile, LOGS_PATH "/telemetry.log", FA_OPEN_ALWAYS | FA_WRITE);
    if (f_size(&g_telemetryFile) > 0) {
      f_lseek(&g_telemetryFile, f_size(&g_telemetryFile)); // append
    }
#endif

#if defined(LOG_BLUETOOTH)
    f_open(&g_bluetoothFile, LOGS_PATH "/bluetooth.log", FA_OPEN_ALWAYS | FA_WRITE);
    if (f_size(&g_bluetoothFile) > 0) {
      f_lseek(&g_bluetoothFile, f_size(&g_bluetoothFile)); // append
    }
#endif
  }
  else {
    TRACE_SIMPGMSPACE("f_mount() failed");
  }
}

void sdDone()
{
  if (sdMounted()) {
    audioQueue.stopSD();
#if defined(LOG_TELEMETRY)
    f_close(&g_telemetryFile);
#endif
#if defined(LOG_BLUETOOTH)
    f_close(&g_bluetoothFile);
#endif
    f_mount(NULL, "", 0); // unmount SD
  }
}

void sdMount()
{
  TRACE("sdMount");
  
  diskCache.clear();
  
  if (f_mount(&g_FATFS_Obj, "", 1) == FR_OK) {
    // call sdGetFreeSectors() now because f_getfree() takes a long time first time it's called
    _g_FATFS_init = true;
    sdGetFreeSectors();

#if defined(LOG_TELEMETRY)
    f_open(&g_telemetryFile, LOGS_PATH "/telemetry.log", FA_OPEN_ALWAYS | FA_WRITE);
    if (f_size(&g_telemetryFile) > 0) {
      f_lseek(&g_telemetryFile, f_size(&g_telemetryFile)); // append
    }
#endif

#if defined(LOG_BLUETOOTH)
    f_open(&g_bluetoothFile, LOGS_PATH "/bluetooth.log", FA_OPEN_ALWAYS | FA_WRITE);
    if (f_size(&g_bluetoothFile) > 0) {
      f_lseek(&g_bluetoothFile, f_size(&g_bluetoothFile)); // append
    }
#endif
  }
  else {
    TRACE("f_mount() failed");
  }
}

uint32_t sdMounted()
{
  return _g_FATFS_init && (g_FATFS_Obj.fs_type != 0);
}

uint32_t sdIsHC()
{
  return sdGetSize() > 2000000;
}

uint32_t sdGetSpeed()
{
  return 330000;
}

#endif // #if defined(SIMU_DISKIO)
