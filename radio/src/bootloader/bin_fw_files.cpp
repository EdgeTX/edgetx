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
#include "firmware_files.h"
#include "fw_version.h"
#include "strhelpers.h"
#include "hal/storage.h"
#include "flash_driver.h"

// Size of the block read when checking / writing BIN files
#define BLOCK_LEN 4096

// 'private'
static DIR  dir;
static FIL FlashFile;

static uint32_t firmwareSize;
static uint32_t firmwareAddress = FIRMWARE_ADDRESS;
static uint32_t firmwareWritten = 0;
uint32_t unlocked = 0;

struct FWFileInfo {
    TCHAR        name[FF_MAX_LFN + 1];
    unsigned int size;
};

// 'public' variables
static FWFileInfo  fwFiles[MAX_FW_FILES];
static uint8_t     Block_buffer[BLOCK_LEN];
static UINT        BlockCount;

static void flashWriteBlock()
{
  // TODO: use some board provided driver instead
  uint32_t blockOffset = 0;
#if !defined(SIMU)
  while (BlockCount) {
    flashWrite((uint32_t *)firmwareAddress, (uint32_t *)&Block_buffer[blockOffset]);
    blockOffset += FLASH_PAGESIZE;
    firmwareAddress += FLASH_PAGESIZE;
    if (BlockCount > FLASH_PAGESIZE) {
      BlockCount -= FLASH_PAGESIZE;
    }
    else {
      BlockCount = 0;
    }
  }
#endif // SIMU
}

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

FRESULT openFirmwareDir()
{
    FRESULT fr = f_chdir(getFirmwarePath());
    if (fr != FR_OK) return fr;

    return f_opendir(&dir, ".");
}

static uint32_t isValidBufferStart(const uint8_t * buffer)
{
#if !defined(SIMU)
  return isFirmwareStart(buffer);
#else
  return 1;
#endif
}

FlashCheckRes checkFirmwareFile(unsigned int index, FlashCheckRes res)
{
  if (res != FC_UNCHECKED)
    return res;

  if (openFirmwareFile(index) != FR_OK)
    return FC_ERROR;

  if (closeFirmwareFile() != FR_OK)
    return FC_ERROR;

  if (!isValidBufferStart(Block_buffer))
    return FC_ERROR;

  return FC_OK;
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

unsigned int fetchFirmwareFiles(unsigned int index)
{
  FILINFO file_info;

  // rewind
  if (f_readdir(&dir, NULL) != FR_OK)
      return 0;

  // skip 'index' .bin files
  for (unsigned int i = 0; i < index; i++) {
      if (findNextBinFile(&file_info) != FR_OK || file_info.fname[0] == 0)
          return 0;
  }

  unsigned int i = 0;
  for (; i < MAX_NAMES_ON_SCREEN+1; i++) {
      if (findNextBinFile(&file_info) != FR_OK || file_info.fname[0] == 0)
          return i;
  
      strAppend(fwFiles[i].name, file_info.fname);
      fwFiles[i].size = file_info.fsize;
  }

  return i;
}

const char* getFirmwareFileNameByIndex(unsigned int index)
{
  if(index >= MAX_FW_FILES)
    return "";

  return fwFiles[index].name;
}

FRESULT openFirmwareFile(unsigned int index)
{
  TCHAR full_path[FF_MAX_LFN+1];
  FRESULT res;

  // build full_path: [bin path]/[filename]
  char* s = strAppend(full_path, getFirmwarePath());
  s = strAppend(s, "/");
  strAppend(s, fwFiles[index].name);

  BlockCount = 0;

  // open the file
  res = f_open(&FlashFile, full_path, FA_READ);
  if (res != FR_OK) return res;

  // skip bootloader in firmware
  res = f_lseek(&FlashFile, BOOTLOADER_SIZE);
  if (res != FR_OK) return res;

  // ... and fetch BLOCK_LEN bytes
  res = f_read(&FlashFile, Block_buffer, BLOCK_LEN, &BlockCount);
  if (BlockCount == BLOCK_LEN && isValidBufferStart(Block_buffer)) return res;

  f_close(&FlashFile);
  return FR_INVALID_OBJECT;
}

void getFileFirmwareVersion(VersionTag* tag)
{
    memset(tag->flavour, 0, sizeof(tag->flavour));
    const char * vers = getFirmwareVersion((const uint8_t *)Block_buffer);
    if (!vers || (vers[0] == 'n' && vers[1] == 'o')) { // "no version found"
      memcpy(tag->flavour, "unknown", sizeof("unknown"));
      tag->version = "unknown";
      tag->flavour[sizeof(tag->flavour)-1] = 0;
      return;
    }

    tag->fork = vers;
    
    // skip 'edgetx-' / 'opentx-'
    vers += sizeof("edgetx-") - 1;

    char* fl = tag->flavour;
    while(*vers != '-')
        *(fl++) = *(vers++);
    tag->flavour[sizeof(tag->flavour)-1] = 0;

    // skip '-'
    tag->version = ++vers;
}

FRESULT readFirmwareFile()
{
    BlockCount = 0;
    return f_read(&FlashFile, Block_buffer, sizeof(Block_buffer), &BlockCount);
}

FRESULT closeFirmwareFile()
{
    return f_close(&FlashFile);
}

void firmwareInitWrite(uint32_t index)
{
  firmwareSize = fwFiles[index].size - BOOTLOADER_SIZE;
  firmwareAddress = FIRMWARE_ADDRESS + BOOTLOADER_SIZE;
  firmwareWritten = 0;
}

bool firmwareWriteBlock(uint32_t* progress)
{
  flashWriteBlock();
  firmwareWritten += sizeof(Block_buffer);
  *progress = (100 * firmwareWritten) / firmwareSize;

  readFirmwareFile();
  if (BlockCount == 0 || firmwareWritten >= FLASHSIZE - BOOTLOADER_SIZE) {
    return true;
  }

  return false;
}

