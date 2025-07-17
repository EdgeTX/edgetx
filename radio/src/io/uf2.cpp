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

#include "uf2.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "board.h"
#include "fw_version.h"

#include "thirdparty/FatFs/ff.h"
#include "thirdparty/uf2/uf2.h"

#define UF2_BLOCK_SIZE 512

bool isUF2Block(const void* block, uint32_t len)
{
  const UF2_Block* uf2 = (UF2_Block*)block;
  return len >= UF2_BLOCK_SIZE
      && uf2->magicStart0 == UF2_MAGIC_START0
      && uf2->magicStart1 == UF2_MAGIC_START1
      && uf2->magicEnd == UF2_MAGIC_END;
}

static size_t getUF2Extension(const UF2_Block* block, uint32_t extension,
                              const char** data)
{
  if ((block->flags & UF2_FLAG_EXTENSION_TAGS) == 0) return 0;

  uint32_t* ptr = (uint32_t*)(block->data + block->payloadSize);
  if (*ptr == 0) return 0;

  while ((size_t)((uint8_t*)ptr - (uint8_t*)block) < sizeof(UF2_Block)) {
    uint32_t tag = ((*ptr) & 0xFFFFFF00) >> 8;
    uint32_t len = (*ptr) & 0x000000FF;
    if (*ptr == 0) break;
    if (tag != extension) {
      ptr += (len + 3) / 4;
      continue;
    }
    *data = (char*)++ptr;
    return len > 4 ? len - 4 : 0;
  }

  return 0;
}

bool extractUF2FirmwareVersion(const void* block, VersionTag* tag)
{

  memset(tag, 0, sizeof(VersionTag));

  const char* extension;
  size_t len = getUF2Extension((UF2_Block*)block, UF2_DEVICE_TAG, &extension);
  if (len == 0) return false;
  memcpy(tag->flavour, extension, std::min(len, sizeof(tag->flavour) - 1));

  len = getUF2Extension((UF2_Block*)block, UF2_VERSION_TAG, &extension);
  if (len == 0) return false;
  memcpy(tag->full_version, extension, std::min(len, sizeof(tag->full_version) - 1));

  tag->fork = tag->full_version;
  tag->full_version[sizeof("edgetx") - 1] = '\0';
  tag->version = tag->fork + sizeof("edgetx");

  return true;
}

bool isUF2FirmwareImage(const void* buffer, uint32_t length)
{
  const uint8_t* bptr = (const uint8_t*)buffer;
  const uint8_t* end_ptr = bptr + length;

  while(bptr < end_ptr) {
    if (!isUF2Block(bptr, end_ptr - bptr)) return false;
    bptr += UF2_BLOCK_SIZE;
  }

  // check compat: should this really be part of checking
  //               the file type?
  // VersionTag tag;
  // if(!extractFirmwareVersion(buffer, &tag))
  //   return false;

  // if(strnlen(tag.flavour, sizeof(tag.flavour)) != strlen(FLAVOUR))
  //   return false;

  // if(strncmp(tag.flavour, FLAVOUR, sizeof(FLAVOUR)) != 0)
  //   return false;

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

static uint8_t* skipUF2Extensions(uint8_t* data) {

  uint32_t* ptr = (uint32_t*)data;
  while(true) {
    uint32_t tag = ((*ptr) & 0xFFFFFF00) >> 8;
    uint32_t len = (*ptr) & 0x000000FF;
    if (*ptr == 0) break;
    if (tag) {
      ptr += (len + 3) / 4;
      continue;
    }
  }

  return (uint8_t*)ptr;
}

#if defined(STM32) && !defined(SIMU)
void writeUF2FirmwareVersion(void* block)
{
  UF2_Block* uf2 = (UF2_Block*)block;
  uint8_t* end_data = skipUF2Extensions(uf2->data + uf2->payloadSize);

  const char* version = getFirmwareVersion((const uint8_t*)FIRMWARE_ADDRESS);
  if (version) {
    uf2->flags |= UF2_FLAG_EXTENSION_TAGS;

    size_t version_len = strlen(version);
    const char* flavour_start = version + sizeof("edgetx-") - 1;
    const char* flavour_end = strchr(flavour_start, '-');

    uint32_t ext_tag = UF2_DEVICE_TAG << 8;
    if (flavour_end < version + version_len) {
      size_t flavour_len = flavour_end - flavour_start;
      ext_tag |= (flavour_len + 4) & 0xFF;
      memcpy(end_data, &ext_tag, sizeof(ext_tag));
      end_data += 4;
      memcpy(end_data, flavour_start, flavour_len);
      end_data += ((flavour_len) + 3) & (~3);
      version_len -= flavour_len + 1;
    } else {
      flavour_end = nullptr;
    }

    ext_tag = UF2_VERSION_TAG << 8;
    ext_tag |= (version_len + 4) & 0xFF;

    memcpy(end_data, &ext_tag, sizeof(ext_tag));
    end_data +=4;

    if (flavour_end) {
      memcpy(end_data, version, sizeof("edgetx-") - 1);
      end_data += sizeof("edgetx-") - 1;
      version_len -= sizeof("edgetx-") - 1;
      memcpy(end_data, flavour_end + 1, version_len);
    } else {
      memcpy(end_data, version, version_len);
    }
  }
}

void writeUF2RebootBlock(void* block)
{
  extern uint32_t _reboot_cmd;
  
  UF2_Block* uf2 = (UF2_Block*)block;

  uf2->targetAddr = (uintptr_t)&_reboot_cmd;
  uf2->flags |= UF2_FLAG_EXTENSION_TAGS | UF2_FLAG_NOFLASH;
  uf2->payloadSize = 4;
  memcpy(uf2->data, "BDFU", 4);
  
  uint8_t* end_data = skipUF2Extensions(uf2->data + uf2->payloadSize);
  uint32_t ext_tag = (UF2_REBOOT_TAG << 8) | 8;
  memcpy(end_data, &ext_tag, sizeof(ext_tag));
  end_data += 4;

  uint32_t reboot_addr = BOOTLOADER_ADDRESS;
  memcpy(end_data, &reboot_addr, sizeof(reboot_addr));
}
#endif
