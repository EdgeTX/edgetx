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

#ifndef _SDCARD_H_
#define _SDCARD_H_

#include "ff.h"

#include "translations.h"

bool sdCardFormat();
uint32_t sdGetNoSectors();
uint32_t sdGetSize();
uint32_t sdGetFreeSectors();
uint32_t sdGetFreeKB();
bool sdIsFull();

#if defined(PCBTARANIS)
void sdPoll10ms();
#endif

#if !defined(SIMU) || defined(SIMU_DISKIO)
  uint32_t sdIsHC();
  uint32_t sdGetSpeed();
  #define SD_IS_HC()                    (sdIsHC())
  #define SD_GET_SPEED()                (sdGetSpeed())
  #define SD_GET_FREE_BLOCKNR()         (sdGetFreeSectors())
#else
  #define SD_IS_HC()                    (0)
  #define SD_GET_SPEED()                (0)
#endif

/*
const char * sdCheckAndCreateDirectory(const char * path);

#if !defined(BOOT)
inline const char * SDCARD_ERROR(FRESULT result)
{
  if (result == FR_NOT_READY)
    return STR_NO_SDCARD;
  else
    return STR_SDCARD_ERROR;
}
#endif

// NOTE: 'size' must = 0 or be a valid character position within 'filename' array -- it is NOT validated
const char * getBasename(const char * path);

bool isFileAvailable(const char * filename, bool exclDir = false);
unsigned int findNextFileIndex(char * filename, uint8_t size, const char * directory);

const char * sdCopyFile(const char * src, const char * dest);
const char * sdCopyFile(const char * srcFilename, const char * srcDir, const char * destFilename, const char * destDir);
const char * sdMoveFile(const char * src, const char * dest);
const char * sdMoveFile(const char * srcFilename, const char * srcDir, const char * destFilename, const char * destDir);

#define LIST_NONE_SD_FILE   1
#define LIST_SD_FILE_EXT    2
bool sdListFiles(const char * path, const char * extension, const uint8_t maxlen, const char * selection, uint8_t flags=0);
*/
#endif // _SDCARD_H_
