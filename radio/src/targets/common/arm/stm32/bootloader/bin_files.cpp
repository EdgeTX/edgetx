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

#include <strings.h>
#include <memory.h>

#include "boot.h"
#include "board.h"
#include "sdcard.h"
#include "bin_files.h"
#include "fw_version.h"
#include "strhelpers.h"
#include "hal/storage.h"

// 'private'
static DIR  dir;
static FIL FlashFile;

// 'public' variables
BinFileInfo binFiles[MAX_BIN_FILES];
uint8_t     Block_buffer[BLOCK_LEN];
UINT        BlockCount;

void sdInit(void)
{
  storageInit();

  static FATFS fatFS __DMA;
  if (f_mount(&fatFS, "", 1) == FR_OK) {
    f_chdir("/");
  }
}

void sdDone()
{
  // unmount
  f_mount(nullptr, "", 0);
  storageDeInit();
}

FRESULT openBinDir(MemoryType mt)
{
    FRESULT fr = f_chdir(getBinaryPath(mt));
    if (fr != FR_OK) return fr;
    
    return f_opendir(&dir, ".");
}

static FRESULT findNextBinFile(FILINFO* fno)
{
  FRESULT fr;

  do {
    fr = f_readdir(&dir, fno);

    if (fr != FR_OK || fno->fname[0] == 0)
      break;
    if (fno->fattrib & (AM_HID | AM_SYS | AM_DIR))
      continue;  // Skip folders and hidden files
    if (fno->fname[0] == '.' && fno->fname[1] != '.')
      continue;  // Ignore hidden files under UNIX, but not ..

    int32_t len = strlen(fno->fname) - 4;
    if (len < 0)
        continue;

    if (fno->fname[len] != '.')
        continue;
    
    if ((fno->fname[len + 1] != 'b') && (fno->fname[len + 1] != 'B'))
        continue;

    if ((fno->fname[len + 2] != 'i') && (fno->fname[len + 2] != 'I'))
        continue;

    if ((fno->fname[len + 3] != 'n') && (fno->fname[len + 3] != 'N'))
        continue;

    // match!
    break;

  } while (1);

  return fr;
}

unsigned int fetchBinFiles(unsigned int index)
{
  FILINFO file_info;

  // rewind
  if (f_readdir(&dir, NULL) != FR_OK)
      return 0;

  // skip 'index' .bin files
  for (unsigned int i = 0; i <= index; i++) {
      
      if (findNextBinFile(&file_info) != FR_OK /*|| file_info.fname[0] == 0*/)
          return 0;
  }

  strAppend(binFiles[0].name, file_info.fname);
  binFiles[0].size = file_info.fsize;

  unsigned int i = 1;
  for (; i < MAX_NAMES_ON_SCREEN+1; i++) {

      if (findNextBinFile(&file_info) != FR_OK || file_info.fname[0] == 0)
          return i;
  
      strAppend(binFiles[i].name, file_info.fname);
      binFiles[i].size = file_info.fsize;
  }

  return i;
}

FRESULT openBinFile(MemoryType mt, unsigned int index)
{
  TCHAR full_path[FF_MAX_LFN+1];
  FRESULT fr;

  // build full_path: [bin path]/[filename]
  char* s = strAppend(full_path, getBinaryPath(mt));
  s = strAppend(s, "/");
  strAppend(s, binFiles[index].name);

  BlockCount = 0;
  
  // open the file
  if ((fr = f_open(&FlashFile, full_path, FA_READ)) != FR_OK)
    return fr;

  // skip bootloader in firmware
  if (mt == MEM_FLASH &&
      ((fr = f_lseek(&FlashFile, BOOTLOADER_SIZE)) != FR_OK))
      return fr;

  // ... and fetch BLOCK_LEN bytes
  fr = f_read(&FlashFile, Block_buffer, BLOCK_LEN, &BlockCount);

  if (BlockCount == BLOCK_LEN)
      return fr;

  return FR_INVALID_OBJECT;
}

void extractFirmwareVersion(VersionTag* tag)
{
    const char * vers = getFirmwareVersion((const char *)Block_buffer);
    if (!vers || (vers[0] == 'n' && vers[1] == 'o')) { // "no version found"
      memcpy(tag->flavour, "unknown", sizeof("unknown"));
      tag->version = "unknown";
      return;
    }

    tag->fork = vers;
    
    // skip 'edgetx-' / 'opentx-'
    vers += sizeof("edgetx-") - 1;

    char* fl = tag->flavour;
    while(*vers != '-')
        *(fl++) = *(vers++);

    // skip '-'
    tag->version = ++vers;
}

FRESULT readBinFile()
{
    BlockCount = 0;
    return f_read(&FlashFile, Block_buffer, sizeof(Block_buffer), &BlockCount);
}

FRESULT closeBinFile()
{
    return f_close(&FlashFile);
}
