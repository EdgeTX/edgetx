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

#include "fatfs_diskio.h"
#include "FatFs/diskio.h"

#if FF_FS_REENTRANT != 0
#include "rtos.h"
#endif

struct fatfs_drive_t {
  const diskio_driver_t* drv;
  uint8_t                lun;
  bool                   initialized;
#if FF_FS_REENTRANT != 0
  RTOS_MUTEX_HANDLE      mutex;
#endif
};

static fatfs_drive_t _fatfs_drives[FF_VOLUMES];
static uint8_t _fatfs_n_drives = 0;

int fatfsRegisterDriver(const diskio_driver_t* drv, uint8_t lun)
{
  if (_fatfs_n_drives >= FF_VOLUMES) {
    return 0;
  }

  uint8_t vol = _fatfs_n_drives++;
  auto& drive = _fatfs_drives[vol];
  drive.drv = drv;
  drive.lun = lun;
  drive.initialized = false;

#if FF_FS_REENTRANT != 0
  // init IO mutex only once
  RTOS_CREATE_MUTEX(drive.mutex);
#endif

  return 1;
}

void fatfsUnregisterDrivers()
{
  for (uint8_t i = 0; i < _fatfs_n_drives; i++) {
    auto& drive = _fatfs_drives[i];
    if (drive.initialized) {
      disk_ioctl(i, CTRL_SYNC, nullptr);
      if (drive.drv->deinit) {
        drive.drv->deinit(drive.lun);
      }
      drive.initialized = false;
    }
  }
  _fatfs_n_drives = 0;
}

const diskio_driver_t* fatfsGetDriver(uint8_t pdrv)
{
  if (pdrv >= _fatfs_n_drives) {
    return nullptr;
  }

  return _fatfs_drives[pdrv].drv;
}

uint8_t fatfsGetLun(uint8_t pdrv)
{
  if (pdrv >= _fatfs_n_drives) {
    return 0;
  }

  return _fatfs_drives[pdrv].lun;
}

#if FF_FS_REENTRANT != 0

int ff_mutex_create(int vol)
{
  return 1;
}

int ff_mutex_take(int vol)
{
  return RTOS_LOCK_MUTEX(_fatfs_drives[vol].mutex);
}

void ff_mutex_give(int vol) { RTOS_UNLOCK_MUTEX(_fatfs_drives[vol].mutex); }

void ff_mutex_delete(int vol) { }

#endif

DSTATUS disk_initialize(BYTE pdrv)
{
  if (pdrv >= _fatfs_n_drives) {
    return STA_NODISK;
  }

  DSTATUS stat = RES_OK;
  auto drive = &_fatfs_drives[pdrv];
  if (!drive->initialized) {
    stat = drive->drv->initialize(drive->lun);
    if (stat == RES_OK) {
      drive->initialized = true;
    }
  }

  return stat;
}

DSTATUS disk_status(BYTE pdrv)
{
  auto drv = _fatfs_drives[pdrv].drv;
  uint8_t lun = _fatfs_drives[pdrv].lun;
  return drv->status(lun);
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
  auto drv = _fatfs_drives[pdrv].drv;
  uint8_t lun = _fatfs_drives[pdrv].lun;
  return drv->read(lun, buff, sector, count);
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
  auto drv = _fatfs_drives[pdrv].drv;
  uint8_t lun = _fatfs_drives[pdrv].lun;
  return drv->write(lun, buff, sector, count);
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff)
{
  auto drv = _fatfs_drives[pdrv].drv;
  uint8_t lun = _fatfs_drives[pdrv].lun;
  return drv->ioctl(lun, cmd, buff);
}

#if FF_FS_NORTC == 0

#include "rtc.h"

DWORD get_fattime(void)
{
  struct gtm t;

  gettime(&t);

  /* Pack date and time into a DWORD variable */
  return ((DWORD)(t.tm_year - 80) << 25)
    | ((uint32_t)(t.tm_mon+1) << 21)
    | ((uint32_t)t.tm_mday << 16)
    | ((uint32_t)t.tm_hour << 11)
    | ((uint32_t)t.tm_min << 5)
    | ((uint32_t)t.tm_sec >> 1);
}
#endif
