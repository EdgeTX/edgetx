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
#include <bitset>
#include <string.h>
#include "thirdparty/FatFs/ff.h"
#ifndef BOOT
#include "edgetx.h"
#else
#include "board.h"
#endif
#include "uf2/uf2.h"
#include "stm32_qspi.h"

#include "uf2_flash.h"
#include "timers_driver.h"
#include "flash_driver.h"

#include "hal/watchdog_driver.h"

#if defined(LIBOPENUI)
  #include "libopenui.h"
#else
  #include "lib_file.h"
#endif

#ifndef BOOTLOADER_ADDRESS
#define BOOTLOADER_ADDRESS FIRMWARE_ADDRESS
#endif

static bool checkUF2Block(UF2_Block* block)
{
  return block->magicStart0 == UF2_MAGIC_START0
      && block->magicStart1 == UF2_MAGIC_START1
      && block->magicEnd == UF2_MAGIC_END;
}

size_t getUF2Extension(const UF2_Block* block, uint32_t extension, char** data)
{
  if((block->flags & UF2_FLAG_EXTENSION_PRESENT) == 0)
    return 0;

  uint32_t* ptr = (uint32_t*)(block->data + block->payloadSize);

  if(*ptr == 0)
    return 0;

  while((size_t)((uint8_t*)ptr - (uint8_t*)block) < sizeof(UF2_Block))
  {
    uint32_t tag = ((*ptr) & 0xFFFFFF00) >> 8;
    uint32_t len = (*ptr) & 0x000000FF;
    if(ptr == 0)
      break;
    if(tag != extension)
    {
      ptr += (len+3)/4;
      continue;
    }
    ptr++;
    *data = (char*)ptr;
    return len-4;
  }
  return 0;
}

static char versionBuf[256];
bool extractFirmwareVersion(const UF2_Block* block, VersionTag* tag)
{
  char* extension;
  char* vers;

  memset(tag->flavour, 0, sizeof(tag->flavour));

  size_t len = getUF2Extension(block, UF2_DEVICE_TAG, &extension);
  if(len>0)
  {
    memcpy(tag->flavour, extension, std::min(len, sizeof(tag->flavour)));
  } else {
    return false;
  }

  len = getUF2Extension(block, UF2_VERSION_TAG, &extension);
  if(len>0)
  {
    memcpy(versionBuf, extension, std::min(len, sizeof(versionBuf)));
  } else {
    return false;
  }
  tag->fork = versionBuf;
  vers = versionBuf;
  // skip 'edgetx-' / 'opentx-'
  vers += sizeof("edgetx-") - 1;

  // skip '-'
  tag->version = vers;

  return true;
}

bool isUF2FirmwareImage(const uint8_t* buffer, size_t length)
{
  if(length < 2048)
    return false;

  UF2_Block* b1 = (UF2_Block*)buffer;
  UF2_Block* b2 = (UF2_Block*)&(buffer[sizeof(*b1)]);
  if(!checkUF2Block(b1) || !checkUF2Block(b2))
  {
    return false;
  }

  VersionTag tag;
  if(!extractFirmwareVersion(b1, &tag))
    return false;

  if(strnlen(tag.flavour, sizeof(tag.flavour)) != strlen(FLAVOUR))
    return false;

  if(strncmp(tag.flavour, FLAVOUR, sizeof(FLAVOUR)) != 0)
    return false;

  return true;
}

bool isUF2FirmwareFile(const char * filename)
{
  FIL file;
  uint8_t buffer[2048];
  UINT count;

  if(f_open(&file, filename, FA_READ) != FR_OK)
    return false;

  if (f_read(&file, buffer, sizeof(buffer), &count) != FR_OK || count != sizeof(buffer)) {
    f_close(&file);
    return false;
  }

  f_close(&file);

  bool ret = isUF2FirmwareImage(buffer, sizeof(buffer));
  return ret;
}
//static const size_t eraseSectorSize = 4096;

bool uf2WriteBuffer(
    UF2_Block* uf2Data,
    size_t count,
    std::bitset<FIRMWARE_MAX_LEN/eraseSectorSize>& erasedSectors,
    size_t* blockCount,
    size_t* totalBlocks)
{

  if(*totalBlocks == 0)
    (*totalBlocks) = uf2Data[0].numBlocks;

  for(uint32_t i = 0; i< count; i++)
  {
    if(!checkUF2Block(&uf2Data[i]))
      return false;

    if((uf2Data[i].flags & UF2_FLAG_NOFLASH) || uf2Data[i].targetAddr < FIRMWARE_ADDRESS || uf2Data[i].targetAddr > FIRMWARE_ADDRESS + FIRMWARE_MAX_LEN)
    {
      (*totalBlocks)--;
      continue;
    }
    (*blockCount)++;
    uint32_t address = uf2Data[i].targetAddr - FIRMWARE_ADDRESS;
    size_t bit = address/eraseSectorSize;
    if(erasedSectors[bit] == 0)
    {
      erasedSectors[bit] = 1;
      qspiEraseSector((intptr_t)address&0xFFFFF000);
    }
    qspiWritePage(address, (uint8_t*)uf2Data[i].data);
  }
  return true;
}


//  UF2_Block uf2Data[512];
#if !defined(BOOT)
void UF2FirmwareUpdate::flashFirmware(const char * filename, ProgressHandler progressHandler)
{
#if !defined(SIMU)
#warning get erase sector size from qspi implementation
  static const size_t eraseSectorSize = 4096;
  std::bitset<FIRMWARE_MAX_LEN/eraseSectorSize> erasedSectors;
  FIL file;
  UF2_Block uf2Data[16];
  UINT count;
  size_t blockCount = 0;
  size_t totalBlocks = 0;
  bool success = false;

  pulsesStop();

  f_open(&file, filename, FA_READ);

  while (f_read(&file, (UINT*)uf2Data, sizeof(uf2Data), &count) == FR_OK)
  {
    size_t blocks = count/sizeof(UF2_Block);

    if(!uf2WriteBuffer(uf2Data, blocks, erasedSectors, &blockCount, &totalBlocks))
    {
      progressHandler("Firmware", STR_FIRMWARE_UPDATE_ERROR, 1, 1);
      f_close(&file);
      return;
    }

    progressHandler("Firmware", STR_WRITING, blockCount, totalBlocks);
    delay_ms(10);
    if(blocks != sizeof(uf2Data)/sizeof(UF2_Block))
    {
      success = true;
      break;
    }
  }


  if(success)
    POPUP_INFORMATION(STR_FIRMWARE_UPDATE_SUCCESS);
  else
    POPUP_INFORMATION(STR_FIRMWARE_UPDATE_ERROR);

  watchdogSuspend(0);
  WDG_RESET();

  f_close(&file);

  pulsesStart();
#endif
}
#endif

#if 0
void BootloaderFirmwareUpdate::flashFirmware(const char * filename, ProgressHandler progressHandler)
{
  FIL file;
  uint8_t buffer[1024];
  UINT count;

  pulsesStop();

  f_open(&file, filename, FA_READ);

  static uint8_t unlocked = 0;
  if (!unlocked) {
    unlocked = 1;
    unlockFlash();
  }

  UINT flash_size = file.obj.objsize;
  if (flash_size > BOOTLOADER_SIZE) {
    flash_size = BOOTLOADER_SIZE;
  }

  for (int i = 0; i < BOOTLOADER_SIZE; i += 1024) {

    watchdogSuspend(1000/*10s*/);
    memset(buffer, 0xFF, sizeof(buffer));

    if (f_read(&file, buffer, sizeof(buffer), &count) != FR_OK) {
      POPUP_WARNING(STR_SDCARD_ERROR);
      break;
    }
    if (count != sizeof(buffer)
        && !f_eof(&file)) {
      POPUP_WARNING(STR_SDCARD_ERROR);
      break;
    }
    if (i == 0 && !isBootloaderStart(buffer)) {
      POPUP_WARNING(STR_INCOMPATIBLE);
      break;
    }
    for (UINT j = 0; j < count; j += FLASH_PAGESIZE) {
      WDG_ENABLE(3000);
      flashWrite(CONVERT_UINT_PTR(BOOTLOADER_ADDRESS + i + j), CONVERT_UINT_PTR(buffer + j));
      WDG_ENABLE(WDG_DURATION);
    }
    progressHandler("Bootloader", STR_WRITING, i, flash_size);

    // Reached end-of-file
    if (f_eof(&file)) break;

#if defined(SIMU)
    // add an artificial delay and check for simu quit
    if (SIMU_SLEEP_OR_EXIT_MS(30))
      break;
#endif
  }

  POPUP_INFORMATION(STR_FIRMWARE_UPDATE_SUCCESS);

  watchdogSuspend(0);
  WDG_RESET();

  if (unlocked) {
    lockFlash();
    unlocked = 0;
  }

  f_close(&file);

  pulsesStart();
}
#endif
