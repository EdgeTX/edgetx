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
#include <stdint.h>
#include <bitset>
#include "uf2/uf2.h"

#include "boards/FirmwareVersionTag.h"

#define UF2_FLAG_FILE_CONTAINER       0x1000
#define UF2_FLAG_FAMILYID_PRESENT     0x2000
#define UF2_FLAG_MD5_CHECKSUM_PRESENT 0x4000
#define UF2_FLAG_EXTENSION_PRESENT    0x8000

#define UF2_VERSION_TAG 0x9FC7BC
#define UF2_DEVICE_TAG  0x650D9D

bool isUF2FirmwareImage(const uint8_t* buffer, size_t length);
bool isUF2FirmwareFile(const char * filename);
bool extractFirmwareVersion(const UF2_Block* block, VersionTag* tag);
static const size_t eraseSectorSize = 4096;
bool uf2WriteBuffer(
    UF2_Block* uf2Data,
    size_t count,
    std::bitset<FIRMWARE_MAX_LEN/eraseSectorSize>& erasedSectors,
    size_t* blockCount,
    size_t* totalBlocks);


#ifndef BOOT
class UF2FirmwareUpdate
{
  public:
    void flashFirmware(const char * filename, ProgressHandler progressHandler);
};
#endif
