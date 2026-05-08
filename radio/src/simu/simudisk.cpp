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
#include "hal/fatfs_diskio.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

extern const diskio_driver_t simu_diskio_driver;

static FILE* disk_image = nullptr;

// create FS with: mkdosfs -n SDCARD -S 512 -C sdcard.img 524288
static const char* image_path = "sdcard.img";

static int _seek(DWORD sector)
{
  if (!disk_image) {
    fprintf(stderr, "disk image is NULL\n");
    return -1;
  }

  if (fseek(disk_image, sector * 512, SEEK_SET) != 0) {
    fprintf(stderr, "fseek failed: %s\n", strerror(errno));
    return -1;
  }

  return 0;
}

static DSTATUS simu_disk_initialize(BYTE lun)
{
  if (!image_path) return STA_NODISK;
  disk_image = fopen(image_path, "rb+");
  if (!disk_image) {
    fprintf(stderr, "could not open disk image: %s\n", strerror(errno));
    return STA_NODISK;
  }

  return RES_OK;
}

static DSTATUS simu_disk_status(BYTE lun) { return RES_OK; }

static DRESULT simu_disk_read(BYTE lun, BYTE* buff, DWORD sector, UINT count)
{
  if (_seek(sector) != 0) return RES_ERROR;
  fprintf(stderr, "# R %d/%d\n", sector, count);
  fread(buff, 512, count, disk_image);
  return RES_OK;
}

static DRESULT simu_disk_write(BYTE lun, const BYTE* buff, DWORD sector, UINT count)
{
  if (_seek(sector) != 0) return RES_ERROR;
  fprintf(stderr, "# W %d/%d\n", sector, count);
  fwrite(buff, 512, count, disk_image);
  return RES_OK;
}

static DRESULT simu_disk_ioctl(BYTE lun, BYTE cmd, void* buff)
{
  if (!disk_image) return RES_NOTRDY;

  DRESULT res = RES_OK;
  switch(cmd) {
    case GET_SECTOR_COUNT:
      if (fseek(disk_image, 0, SEEK_END) == 0) {
        long sectors = ftell(disk_image) / 512L;
        fprintf(stderr, "# S %ld\n", sectors);
        *((DWORD*)buff) = (DWORD)(sectors);
      } else {
        fprintf(stderr, "fseek failed: %s\n", strerror(errno));
        res = RES_ERROR;
      }
      break;

    case GET_SECTOR_SIZE:
      *(WORD*)buff = 512;
      break;

    case GET_BLOCK_SIZE:
      *(WORD*)buff = 512 * 4;
      break;

    default:
      break;
  }

  return res;
}

const diskio_driver_t simu_diskio_driver = {
  .initialize = simu_disk_initialize,
  .status = simu_disk_status,
  .read = simu_disk_read,
  .write = simu_disk_write,
  .ioctl = simu_disk_ioctl,
};

#endif // #if defined(SIMU_DISKIO)
