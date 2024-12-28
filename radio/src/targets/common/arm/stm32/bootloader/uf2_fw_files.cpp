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
#include "io/uf2_flash.h"

#include "flash_driver.h"

# if defined(FIRMWARE_QSPI)
#   include "stm32_qspi.h"
# endif

// Size of the block read when checking / writing BIN files
#define BLOCK_LEN 4096

// 'private'
static DIR  dir;
static FIL firmwareFile;

#ifndef FIRMWARE_QSPI
static uint32_t firmwareSize;
static uint32_t firmwareAddress = FIRMWARE_ADDRESS;
static uint32_t firmwareWritten = 0;
#endif

struct FWFileInfo {
    TCHAR        name[FF_MAX_LFN + 1];
    unsigned int size;
};

// 'public' variables
static FWFileInfo  fwFiles[MAX_FW_FILES];
static uint8_t     Block_buffer[BLOCK_LEN];
static UINT        BlockCount;

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

FRESULT openFirmwareDir(MemoryType mt)
{
    FRESULT fr = f_chdir(getFirmwarePath(mt));
    if (fr != FR_OK) return fr;
    
    return f_opendir(&dir, ".");
}

FlashCheckRes checkFirmwareFile(unsigned int index, MemoryType memoryType, FlashCheckRes res)
{
  if(memoryType != MEM_FLASH)
    return FC_ERROR;

  if (res != FC_UNCHECKED)
    return res;

  if(openFirmwareFile(memoryType, index) != FR_OK)
    return FC_ERROR;

  f_close(&firmwareFile);
  return FC_OK;
}

static FRESULT findNextFile(FILINFO* fno)
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
    
    if ((fno->fname[len + 1] != 'u') && (fno->fname[len + 1] != 'U'))
        continue;

    if ((fno->fname[len + 2] != 'f') && (fno->fname[len + 2] != 'F'))
        continue;

    if ((fno->fname[len + 3] != '2') && (fno->fname[len + 3] != '2'))
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
  for (unsigned int i = 0; i <= index; i++) {
      
      if (findNextFile(&file_info) != FR_OK /*|| file_info.fname[0] == 0*/)
          return 0;
  }

  strAppend(fwFiles[0].name, file_info.fname);
  fwFiles[0].size = file_info.fsize;

  unsigned int i = 1;
  for (; i < MAX_NAMES_ON_SCREEN+1; i++) {

      if (findNextFile(&file_info) != FR_OK || file_info.fname[0] == 0)
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

void getFileFirmwareVersion(VersionTag* tag)
{

  extractFirmwareVersion((UF2_Block*)Block_buffer, tag);
}

FRESULT openFirmwareFile(MemoryType mt, unsigned int index)
{
  TCHAR full_path[FF_MAX_LFN+1];
  FRESULT fr;

  // build full_path: [bin path]/[filename]
  char* s = strAppend(full_path, getFirmwarePath(mt));
  s = strAppend(s, "/");
  strAppend(s, fwFiles[index].name);

  BlockCount = 0;
  
  // open the file
  if ((fr = f_open(&firmwareFile, full_path, FA_READ)) != FR_OK)
    return fr;
#if !defined(STM32H7)
  // skip bootloader in firmware
  if (mt == MEM_FLASH &&
      ((fr = f_lseek(&firmwareFile, BOOTLOADER_SIZE)) != FR_OK))
      return fr;
#endif
  // ... and fetch BLOCK_LEN bytes
  fr = f_read(&firmwareFile, Block_buffer, sizeof(Block_buffer), &BlockCount);

  if (BlockCount == BLOCK_LEN && isUF2FirmwareImage(Block_buffer, sizeof(Block_buffer)))
  {
      return fr;
  }

  f_close(&firmwareFile);

  return FR_INVALID_OBJECT;
}




FRESULT readFirmwareFile()
{
    BlockCount = 0;
    return f_read(&firmwareFile, Block_buffer, sizeof(Block_buffer), &BlockCount);
}

FRESULT closeFirmwareFile()
{
    return f_close(&firmwareFile);
}


void firmwareInitWrite(uint32_t index)
{
#ifndef FIRMWARE_QSPI
  firmwareSize = fwFiles[vpos].size - BOOTLOADER_SIZE;
  firmwareAddress = FIRMWARE_ADDRESS + BOOTLOADER_SIZE;
  firmwareWritten = 0;
#endif
}

//static const size_t eraseSectorSize = 4096;
static std::bitset<FIRMWARE_MAX_LEN/eraseSectorSize> erasedSectors;
size_t blockCount = 0;
size_t totalBlocks = 0;

bool firmwareWriteBlock(uint32_t* progress)
{
#ifndef FIRMWARE_QSPI
  // commit to flashing
  if (!unlocked && (MEM_FLASH)) {
    unlocked = 1;
    unlockFlash();
  }
#else

  uf2WriteBuffer((UF2_Block*)Block_buffer, BlockCount/sizeof(UF2_Block), erasedSectors, &blockCount, &totalBlocks);
  *progress = (100*blockCount)/totalBlocks;
  if(blockCount==totalBlocks)
    return true;
  readFirmwareFile();

#endif


#ifndef FIRMWARE_QSPI
  flashWriteBlock();
  firmwareWritten += sizeof(Block_buffer);
  *progress = (100 * firmwareWritten) / firmwareSize;

  FRESULT fr = readFirmwareFile();
  if (BlockCount == 0) {
    return true;
  }
  else if (firmwareWritten >= FLASHSIZE - BOOTLOADER_SIZE) {
    return true;
  }
#endif
  return false;
}
