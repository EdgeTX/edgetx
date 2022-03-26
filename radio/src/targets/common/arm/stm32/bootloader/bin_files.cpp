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
#include "VirtualFS.h"
#include "bin_files.h"
#include "fw_version.h"
#include "strhelpers.h"

// 'private'
static VfsDir  dir;
static VfsFile FlashFile;

// 'public' variables
BinFileInfo binFiles[MAX_BIN_FILES];
uint8_t     Block_buffer[BLOCK_LEN];
size_t        BlockCount;

const char *getBinaryPath(MemoryType mt)
{
  switch(mt)
  {
  case MEM_EEPROM: return EEPROMS_PATH;
#if defined(SPI_FLASH) && defined(SDCARD)
  case MEM_INTERNAL: return  INTERNAL_ST_FIRMWARES_PATH;
  case MEM_SDCARD: return SDCARD_FIRMWARES_PATH;
#else
  case MEM_INTERNAL:
  case MEM_SDCARD: return FIRMWARES_PATH;
#endif
  }
  return "";
}


VfsError openBinDir(MemoryType mt)
{
    VirtualFS& vfs = VirtualFS::instance();
    VfsError fr = vfs.changeDirectory(getBinaryPath(mt));
    if (fr != VfsError::OK) return fr;
    
    return vfs.openDirectory(dir, ".");
}

static VfsError findNextBinFile(VfsFileInfo* fno)
{
  VfsError fr;

  do {
    fr = dir.read(*fno);

    if (fr != VfsError::OK || fno->getName().length() == 0)
      break;

    int32_t len = fno->getName().length() - 4;
    if (len < 0)
        continue;

    std::string fname = fno->getName();
    if (fname[len] != '.')
        continue;
    
    if ((fname[len + 1] != 'b') && (fname[len + 1] != 'B'))
        continue;

    if ((fname[len + 2] != 'i') && (fname[len + 2] != 'I'))
        continue;

    if ((fname[len + 3] != 'n') && (fname[len + 3] != 'N'))
        continue;

    // match!
    break;

  } while (1);

  return fr;
}

unsigned int fetchBinFiles(unsigned int index)
{
  VfsFileInfo file_info;

  // rewind
  if (dir.rewind() != VfsError::OK)
      return 0;

  // skip 'index' .bin files
  for (unsigned int i = 0; i <= index; i++) {
      
      if (findNextBinFile(&file_info) != VfsError::OK /*|| file_info.fname[0] == 0*/)
          return 0;
  }

  strAppend(binFiles[0].name, file_info.getName().c_str());
  binFiles[0].size = file_info.getSize();

  unsigned int i = 1;
  for (; i < MAX_NAMES_ON_SCREEN+1; i++) {

      if (findNextBinFile(&file_info) != VfsError::OK || file_info.getName().length() == 0)
          return i;
  
      strAppend(binFiles[i].name, file_info.getName().c_str());
      binFiles[i].size = file_info.getSize();
  }

  return i;
}

VfsError openBinFile(MemoryType mt, unsigned int index)
{
  TCHAR full_path[FF_MAX_LFN+1];
  VfsError fr;

  // build full_path: [bin path]/[filename]
  char* s = strAppend(full_path, getBinaryPath(mt));
  s = strAppend(s, "/");
  strAppend(s, binFiles[index].name);

  BlockCount = 0;
  
  // open the file
  if ((fr = VirtualFS::instance().openFile(FlashFile, full_path, VfsOpenFlags::READ)) != VfsError::OK)
    return fr;

  // skip bootloader in firmware
  if (mt != MEM_EEPROM &&
      ((fr = FlashFile.lseek(BOOTLOADER_SIZE)) != VfsError::OK))
      return fr;

  // ... and fetch BLOCK_LEN bytes
  fr = FlashFile.read(Block_buffer, BLOCK_LEN, BlockCount);

  if (BlockCount == BLOCK_LEN)
      return fr;

  return VfsError::INVAL;
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

VfsError readBinFile()
{
    BlockCount = 0;
    return FlashFile.read(Block_buffer, sizeof(Block_buffer), BlockCount);
}

VfsError closeBinFile()
{
    return FlashFile.close();
}
