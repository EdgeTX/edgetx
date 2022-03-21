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

#ifndef _DISK_CACHE_H_
#define _DISK_CACHE_H_

#include "diskio.h"
#include "sdio_sd.h"

// tunable parameters
#define DISK_CACHE_BLOCKS_NUM      32   // no cache blocks
#define DISK_CACHE_BLOCK_SECTORS   16   // no sectors

#define DISK_CACHE_BLOCK_SIZE   (DISK_CACHE_BLOCK_SECTORS * BLOCK_SIZE)

class DiskCacheBlock
{
public:
  DiskCacheBlock();
  bool read(BYTE* buff, DWORD sector, UINT count);
  DRESULT fill(BYTE drv, BYTE* buff, DWORD sector, UINT count);
  void free(DWORD sector, UINT count);
  void free();
  bool empty() const;

private:
  uint8_t data[DISK_CACHE_BLOCK_SIZE];
  DWORD startSector;
  DWORD endSector;
};

struct DiskCacheStats
{
  uint32_t noHits;
  uint32_t noMisses;
  uint32_t noWrites;
};

class DiskCache
{
  public:
    DiskCache();
    DRESULT read(BYTE drv, BYTE* buff, DWORD sector, UINT count);
    DRESULT write(BYTE drv, const BYTE* buff, DWORD sector, UINT count);
    const DiskCacheStats & getStats() const;
    int getHitRate() const;
    void clear();

  private:
    DiskCacheStats stats;
    uint32_t lastBlock;
    DiskCacheBlock * blocks;
};

extern DiskCache diskCache[2];

#endif // _DISK_CACHE_H_
