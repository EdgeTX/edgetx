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

#pragma once

#include <FatFs/ff.h>
#include "sdcard.h"
#include "boards/FirmwareVersionTag.h"

#define getFirmwarePath()  (FIRMWARES_PATH)

#if LCD_W < 300         // B&W
  constexpr int MAX_NAMES_ON_SCREEN = 6;
#elif LCD_W < LCD_H     // Portrait
  constexpr int MAX_NAMES_ON_SCREEN = (LCD_H - 143) / 25;
#else                   // Landscape
  constexpr int MAX_NAMES_ON_SCREEN = (LCD_H - 119) / 25;
#endif

#define MAX_FW_FILES         (MAX_NAMES_ON_SCREEN+1)

// Open directory for firmware files
FRESULT openFirmwareDir();

// Fetch file names and sizes into binFiles,
// starting at the provided index.
// Only files ending with ".bin" (case-insensitive)
// will be considered.
unsigned int fetchFirmwareFiles(unsigned int index);
const char* getFirmwareFileNameByIndex(unsigned int index);

// Open file indexed in binFiles and read the first BLOCK_LEN bytes
// Bootloader is skipped in firmware files
FRESULT openFirmwareFile(unsigned int index);

// Can be called right after openBinFile() to extract the version information
// from a firmware file
void getFileFirmwareVersion(VersionTag* tag);

// Read the next BLOCK_LEN bytes into 'Block_buffer'
// Check 'BlockCount' for # of bytes read
FRESULT readFirmwareFile();

// Close the previously opened file
FRESULT closeFirmwareFile();

enum FlashCheckRes {
    FC_UNCHECKED=0,
    FC_OK,
    FC_ERROR
};
FlashCheckRes checkFirmwareFile(unsigned int index, FlashCheckRes res);

void firmwareInitWrite(uint32_t index);
bool firmwareEraseBlock(uint32_t* progress);
bool firmwareWriteBlock(uint32_t* progress);

