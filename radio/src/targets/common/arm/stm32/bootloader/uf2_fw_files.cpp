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
#include "hal/flash_driver.h"
#include "sdcard.h"
#include "firmware_files.h"
#include "fw_version.h"
#include "strhelpers.h"
#include "hal/storage.h"

#include "io/uf2.h"
#include "uf2/uf2.h"

#include "flash_driver.h"

// Size of the block read when checking / writing files
#define BLOCK_LEN (16 * 1024)

// 'private'
static DIR  dir;
static FIL firmwareFile;

static uint32_t firmwareSize;
static uint32_t firmwareAddress;
static uint32_t firmwareWritten;
static uint32_t firmwareErased;
static uint32_t lastErased;

struct FWFileInfo {
    TCHAR        name[FF_MAX_LFN + 1];
    unsigned int size;
};

// 'public' variables
static FWFileInfo  fwFiles[MAX_FW_FILES];
static uint8_t     Block_buffer[BLOCK_LEN];

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

FlashCheckRes checkFirmwareFile(unsigned int index, FlashCheckRes res)
{
  if (res != FC_UNCHECKED)
    return res;

  if(openFirmwareFile(index) != FR_OK)
    return FC_ERROR;

  f_close(&firmwareFile);
  return FC_OK;
}

#define match_uf2_ext(ext)                 \
  (((ext)[0] == 'u' || (ext)[0] == 'U') && \
   ((ext)[1] == 'f' || (ext)[1] == 'F') && \
   ((ext)[2] == '2' || (ext)[2] == '2'))

#define match_bin_ext(ext)                 \
  (((ext)[0] == 'b' || (ext)[0] == 'B') && \
   ((ext)[1] == 'i' || (ext)[1] == 'I') && \
   ((ext)[2] == 'n' || (ext)[2] == 'N'))

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
    if (len < 0) continue;

    const char* ext = fno->fname + len;
    if (*ext != '.') continue;
    ext++;

    if (match_uf2_ext(ext) || match_bin_ext(ext)) break;

  } while (1);

  return fr;
}

unsigned int fetchFirmwareFiles(unsigned int index)
{
  FILINFO file_info;

  // rewind
  if (f_readdir(&dir, NULL) != FR_OK) return 0;

  // skip 'index' .bin files
  for (unsigned int i = 0; i <= index; i++) {
    if (findNextFile(&file_info) != FR_OK) return 0;
  }

  strAppend(fwFiles[0].name, file_info.fname);
  fwFiles[0].size = file_info.fsize;

  unsigned int i = 1;
  for (; i < MAX_NAMES_ON_SCREEN + 1; i++) {
    if (findNextFile(&file_info) != FR_OK || file_info.fname[0] == 0) return i;

    strAppend(fwFiles[i].name, file_info.fname);
    fwFiles[i].size = file_info.fsize;
  }

  return i;
}

const char* getFirmwareFileNameByIndex(unsigned int index)
{
  if (index >= MAX_FW_FILES) return "";
  return fwFiles[index].name;
}

void getFileFirmwareVersion(VersionTag* tag)
{
  extractUF2FirmwareVersion(Block_buffer, tag);
}

FRESULT openFirmwareFile(unsigned int index)
{
  TCHAR full_path[FF_MAX_LFN + 1];
  FRESULT res;

  // build full_path: [bin path]/[filename]
  char* s = strAppend(full_path, getFirmwarePath());
  s = strAppend(s, "/");
  strAppend(s, fwFiles[index].name);

  // open the file
  res = f_open(&firmwareFile, full_path, FA_READ);
  if (res != FR_OK) return res;

  // ... and fetch BLOCK_LEN bytes
  UINT bytes_read = 0;
  res = f_read(&firmwareFile, Block_buffer, sizeof(Block_buffer), &bytes_read);
  if (bytes_read == BLOCK_LEN &&
      isUF2FirmwareImage(Block_buffer, sizeof(Block_buffer))) {
      return res;
  }

  f_close(&firmwareFile);
  return FR_INVALID_OBJECT;
}

FRESULT readFirmwareFile()
{
  UINT bytes_read = 0;
  return f_read(&firmwareFile, Block_buffer, sizeof(Block_buffer), &bytes_read);
}

FRESULT closeFirmwareFile() { return f_close(&firmwareFile); }

void firmwareInitWrite(uint32_t index)
{
  firmwareSize = fwFiles[index].size;
  firmwareAddress = FIRMWARE_ADDRESS;
  firmwareWritten = 0;
  firmwareErased = 0;
  lastErased = 0;

  // assume file is open
  f_rewind(&firmwareFile);
}

bool firmwareEraseBlock(uint32_t* progress)
{
  FRESULT res;
  UINT bytes_read = 0;
  res = f_read(&firmwareFile, Block_buffer, sizeof(Block_buffer), &bytes_read);

  if (res != FR_OK || bytes_read == 0) {
    // TODO: some error?
    return true;
  }

  auto block = (const UF2_Block*)Block_buffer;
  while(bytes_read >= sizeof(UF2_Block)) {
    if ((block->flags & UF2_FLAG_NOFLASH) == 0) {
      uint32_t addr = block->targetAddr;
      if (lastErased <= addr) {
        auto drv = flashFindDriver(addr);
        if (drv) {
          uint32_t sector = drv->get_sector(addr);
          uint32_t sect_len = drv->get_sector_size(sector);
          if (drv->erase_sector(addr) < 0) {
            *progress = 100;
            return true;
          }
          lastErased = addr + sect_len - 1;
        }
      }
    }
    block++;
    bytes_read -= sizeof(UF2_Block);
    firmwareErased += sizeof(UF2_Block);
  }

  *progress = (100 * firmwareErased) / firmwareSize;
  return firmwareErased >= firmwareSize;
}

bool firmwareWriteBlock(uint32_t* progress)
{
  if (firmwareWritten == 0 && f_rewind(&firmwareFile) != FR_OK) {
    // TODO: some error?
    return true;
  }
  
  FRESULT res;
  UINT bytes_read = 0;
  res = f_read(&firmwareFile, Block_buffer, sizeof(Block_buffer), &bytes_read);

  if (res != FR_OK || bytes_read == 0) {
    // TODO: some error?
    return true;
  }

  auto block = (const UF2_Block*)Block_buffer;
  while(bytes_read >= sizeof(UF2_Block)) {
    if ((block->flags & UF2_FLAG_NOFLASH) == 0) {
      uint32_t addr = block->targetAddr;
      auto drv = flashFindDriver(addr);
      if (drv) {
        uint32_t len = block->payloadSize;
        uint8_t* data = (uint8_t*)block->data;
        drv->program(addr, data, len);
      }
    }
    block++;
    bytes_read -= sizeof(UF2_Block);
    firmwareWritten += sizeof(UF2_Block);
  }

  *progress = (100 * firmwareWritten) / firmwareSize;
  return firmwareWritten >= firmwareSize;
}
