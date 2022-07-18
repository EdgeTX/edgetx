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

#if !defined(BOOT)
inline const char * SDCARD_ERROR(FRESULT result)
{
  if (result == FR_NOT_READY)
    return STR_NO_SDCARD;
  else
    return STR_SDCARD_ERROR;
}
#endif

bool sdCardFormat()
{
  BYTE work[FF_MAX_SS];
  FRESULT res = f_mkfs("", FM_FAT32, 0, work, sizeof(work));
  switch(res) {
    case FR_OK :
      return true;
    case FR_DISK_ERR:
      POPUP_WARNING("Format error");
      return false;
    case FR_NOT_READY:
      POPUP_WARNING("SDCard not ready");
      return false;
    case FR_WRITE_PROTECTED:
      POPUP_WARNING("SDCard write protected");
      return false;
    case FR_INVALID_PARAMETER:
      POPUP_WARNING("Format param invalid");
      return false;
    case FR_INVALID_DRIVE:
      POPUP_WARNING("Invalid drive");
      return false;
    case FR_MKFS_ABORTED:
      POPUP_WARNING("Format aborted");
      return false;
    default:
      POPUP_WARNING(STR_SDCARD_ERROR);
      return false;
  }
}

#if !defined(SIMU) || defined(SIMU_DISKIO)
uint32_t sdGetNoSectors()
{
  static DWORD noSectors = 0;
  if (noSectors == 0 ) {
    disk_ioctl(0, GET_SECTOR_COUNT, &noSectors);
  }
  return noSectors;
}

uint32_t sdGetSize()
{
  return (sdGetNoSectors() / 1000000) * BLOCK_SIZE;
}

uint32_t sdGetFreeSectors()
{
  DWORD nofree = 0;
  FATFS * fat = nullptr;
  if (f_getfree("", &nofree, &fat) != FR_OK) {
    return 0;
  }
  return nofree * fat->csize;
}

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
  return 10;
}

#endif  // #if !defined(SIMU) || defined(SIMU_DISKIO)
