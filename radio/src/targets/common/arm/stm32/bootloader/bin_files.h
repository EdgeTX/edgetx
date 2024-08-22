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
#if defined(COLORLCD)
#include "bitmaps.h"
#endif

enum MemoryType {
  MEM_FLASH,
  MEM_EEPROM
};

#define getBinaryPath(mt)  (FIRMWARES_PATH)

#if LCD_H == 480
#define MAX_NAMES_ON_SCREEN   13
#else
#define MAX_NAMES_ON_SCREEN   6
#endif

#define MAX_BIN_FILES         (MAX_NAMES_ON_SCREEN+1)

// Size of the block read when checking / writing BIN files
#define BLOCK_LEN 4096

// File info struct while browsing files on SD card
struct BinFileInfo {
    TCHAR        name[FF_MAX_LFN + 1];
    unsigned int size;
};

// File info storage while browsing files on SD card
extern BinFileInfo binFiles[MAX_BIN_FILES];

// Block buffer used when checking / writing BIN files
extern uint8_t Block_buffer[BLOCK_LEN];

// Bytes read into the Block_buffer
extern UINT    BlockCount;

// Open directory for EEPROM / firmware files
FRESULT openBinDir(MemoryType mt);

// Fetch file names and sizes into binFiles,
// starting at the provided index.
// Only files ending with ".bin" (case-insensitive)
// will be considered.
unsigned int fetchBinFiles(unsigned int index);

// Open file indexed in binFiles and read the first BLOCK_LEN bytes
// Bootloader is skipped in firmware files
FRESULT openBinFile(MemoryType mt, unsigned int index);

struct VersionTag
{
    char        flavour[11];
    const char* version;
    const char* fork;
};
// Ensure flavour can hold FLAVOUR defined in target cmakefile
static_assert(sizeof(((VersionTag){}).flavour) >= sizeof(FLAVOUR), "VersionTag flavour size too small");


// Can be called right after openBinFile() to extract the version information
// from a firmware file
void extractFirmwareVersion(VersionTag* tag);

// Read the next BLOCK_LEN bytes into 'Block_buffer'
// Check 'BlockCount' for # of bytes read
FRESULT readBinFile();

// Close the previously opened file
FRESULT closeBinFile();
