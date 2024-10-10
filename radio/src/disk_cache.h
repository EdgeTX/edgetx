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

#include "hal/fatfs_diskio.h"

// tunable parameters
#define DISK_CACHE_BLOCKS_NUM      32   // no cache blocks
#define DISK_CACHE_BLOCK_SECTORS   16   // no sectors

struct DiskCacheStats
{
  uint32_t noHits;
  uint32_t noMisses;
  uint32_t noWrites;
};

class DiskCacheBlock;

class DiskCache
{
 public:
  DiskCache();

  void initialize(const diskio_driver_t* drv);
  void clear();

  DRESULT read(BYTE drv, BYTE* buff, DWORD sector, UINT count);
  DRESULT write(BYTE drv, const BYTE* buff, DWORD sector, UINT count);

  const DiskCacheStats& getStats() const;
  int getHitRate() const;

 private:
  DiskCacheStats stats;
  uint32_t lastBlock;
  DiskCacheBlock* blocks;
  const diskio_driver_t* diskDrv;
  uint32_t sectors;

  uint32_t getSectors(uint8_t lun);
};

extern DiskCache diskCache;

DRESULT disk_cache_read(BYTE drv, BYTE* buff, DWORD sector, UINT count);
DRESULT disk_cache_write(BYTE drv, const BYTE* buff, DWORD sector, UINT count);


