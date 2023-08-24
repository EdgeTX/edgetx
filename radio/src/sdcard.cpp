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

#include <stdio.h>
#include <stdint.h>

#include "opentx.h"
#include "diskio.h"

#if defined(LIBOPENUI)
  #include "libopenui.h"
#endif

#if FF_MAX_SS != FF_MIN_SS
#error "Variable sector size is not supported"
#endif

#define BLOCK_SIZE FF_MAX_SS

#define SDCARD_MIN_FREE_SPACE_MB 50 // Maintain a 50MB free space buffer to prevent crashes


#if !defined(SIMU) || defined(SIMU_DISKIO)
uint32_t sdGetSize()
{
  return (sdGetNoSectors() / 1000000) * BLOCK_SIZE;
}

uint32_t sdGetFreeSectors()
{
  DWORD nofree;
  FATFS * fat;
  if (f_getfree("", &nofree, &fat) != FR_OK) {
    return 0;
  }
  return nofree * fat->csize;
}

uint32_t sdGetFreeKB()
{
  return sdGetFreeSectors() * (1024 / BLOCK_SIZE);
}

bool sdIsFull() { return sdGetFreeKB() < SDCARD_MIN_FREE_SPACE_MB * 1024; }

#else  // #if !defined(SIMU) || defined(SIMU_DISKIO)

uint32_t sdGetNoSectors()
{
  return 0;
}

uint32_t sdGetSize()
{
  return 0;
}

uint32_t sdGetFreeSectors()
{
  return ((SDCARD_MIN_FREE_SPACE_MB*1024*1024)/BLOCK_SIZE)+1;    // SIMU SD card is always above threshold
}

uint32_t sdGetFreeKB() { return SDCARD_MIN_FREE_SPACE_MB * 1024 + 1; }
bool sdIsFull() { return false; }

#endif  // #if !defined(SIMU) || defined(SIMU_DISKIO)


static bool _g_FATFS_init = false;
static FATFS g_FATFS_Obj __DMA; // this is in uninitialised section !!!

#if defined(LOG_TELEMETRY)
FIL g_telemetryFile = {};
#endif

#if defined(LOG_BLUETOOTH)
FIL g_bluetoothFile = {};
#endif

/*#include "audio.h"
#include "sdcard.h"
#include "disk_cache.h"
*/
/*-----------------------------------------------------------------------*/
/* Lock / unlock functions                                               */
/*-----------------------------------------------------------------------*/
/*
static RTOS_MUTEX_HANDLE ioMutex;
uint32_t ioMutexReq = 0, ioMutexRel = 0;
int ff_cre_syncobj (BYTE vol, FF_SYNC_t * mutex)
{
  *mutex = ioMutex;
  return 1;
}

int ff_req_grant (FF_SYNC_t mutex)
{
  ioMutexReq += 1;
  RTOS_LOCK_MUTEX(mutex);
  return 1;
}

void ff_rel_grant (FF_SYNC_t mutex)
{
  ioMutexRel += 1;
  RTOS_UNLOCK_MUTEX(mutex);
}

int ff_del_syncobj (FF_SYNC_t mutex)
{
  return 1;
}

void sdInit()
{
  TRACE("sdInit");
  RTOS_CREATE_MUTEX(ioMutex);
  sdMount();
}

void sdMount()
{
  TRACE("sdMount");

#if defined(DISK_CACHE)
  diskCache.clear();
#endif
  
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

void sdDone()
{
  TRACE("sdDone");
  
  if (sdMounted()) {
    audioQueue.stopSD();

#if defined(LOG_TELEMETRY)
    f_close(&g_telemetryFile);
#endif

#if defined(LOG_BLUETOOTH)
    f_close(&g_bluetoothFile);
#endif

    f_mount(nullptr, "", 0); // unmount SD
  }
}

uint32_t sdMounted()
{
#if defined(SIMU)
  return true;
#else
  return _g_FATFS_init && (g_FATFS_Obj.fs_type != 0);
#endif
}


uint32_t sdIsHC()
{
  // defined in diskio
  #define CT_BLOCK 0x08
  extern uint32_t SD_GetCardType();

  return SD_GetCardType() & CT_BLOCK;
}

uint32_t sdGetSpeed()
{
  return 330000;
}
*/
