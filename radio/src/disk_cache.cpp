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

#include <string.h>
#include "opentx.h"

#if defined(SIMU) && !defined(SIMU_DISKIO)
  #define __disk_read(...)    (RES_OK)
  #define __disk_write(...)   (RES_OK)
#endif

#if 0     // set to 1 to enable traces
  #define TRACE_DISK_CACHE(...)   TRACE(__VA_ARGS__)
#else
  #define TRACE_DISK_CACHE(...)
#endif

DiskCache diskCache[2];

DiskCacheBlock::DiskCacheBlock():
  startSector(0),
  endSector(0) 
{
}

bool DiskCacheBlock::read(BYTE * buff, DWORD sector, UINT count)
{
  if (sector >= startSector && (sector+count) <= endSector) {
    TRACE_DISK_CACHE("\tcache read(%u, %u) from %p", (uint32_t)sector, (uint32_t)count, this);
    memcpy(buff, data + ((sector - startSector) * BLOCK_SIZE), count * BLOCK_SIZE);
    return true;
  }
  return false;
}

DRESULT DiskCacheBlock::fill(BYTE drv, BYTE * buff, DWORD sector, UINT count)
{
  DRESULT res = __disk_read(drv, data, sector, DISK_CACHE_BLOCK_SECTORS);
  if (res != RES_OK) {
    return res;
  }
  startSector = sector;
  endSector = sector + DISK_CACHE_BLOCK_SECTORS;
  memcpy(buff, data, count * BLOCK_SIZE);
  TRACE_DISK_CACHE("\tcache %p FILLED from read(%u, %u)", this, (uint32_t)sector, (uint32_t)count);
  return RES_OK;
}

void DiskCacheBlock::free(DWORD sector, UINT count) 
{
  if (sector < endSector && (sector+count) > startSector) {
    TRACE_DISK_CACHE("\tINVALIDATING disk cache block %p (%u)", this, startSector);
    endSector = 0;
  }
}

void DiskCacheBlock::free()
{
  endSector = 0;
}

bool DiskCacheBlock::empty() const
{
  return (endSector == 0);
}

DiskCache::DiskCache():
  lastBlock(0)
{
  stats.noHits = 0;
  stats.noMisses = 0;
  stats.noWrites = 0;
  blocks = new DiskCacheBlock[DISK_CACHE_BLOCKS_NUM];
}

void DiskCache::clear()
{
  lastBlock = 0;
  stats.noHits = 0;
  stats.noMisses = 0;
  stats.noWrites = 0;
  for (int n=0; n<DISK_CACHE_BLOCKS_NUM; ++n) {
    blocks[n].free();
  }
}

DRESULT DiskCache::read(BYTE drv, BYTE * buff, DWORD sector, UINT count)
{
  // TODO: check if not caching first sectors would improve anything
  // if (sector < 1000) {
  //   ++stats.noMisses;
  //   return __disk_read(drv, buff, sector, count);  
  // }

  // if read is bigger than cache block, then read it directly without using cache
  if (count > DISK_CACHE_BLOCK_SECTORS) {
    TRACE_DISK_CACHE("\t\t big read(%u, %u)",  (uint32_t)sector, (uint32_t)count);
    return __disk_read(drv, buff, sector, count);
  }
  
  // if block + cache block size is beyond the end of the disk, then read it directly without using cache
  size_t sectors = 0;
  DRESULT res = disk_ioctl(drv, GET_SECTOR_COUNT, &sectors);
  if (res != RES_OK || sector+DISK_CACHE_BLOCK_SECTORS >= sectors) {
    TRACE_DISK_CACHE("\t\t cache would be beyond end of disk %u (%u)", (uint32_t)sector, sdGetNoSectors());
    return __disk_read(drv, buff, sector, count);
  }

  for (int n=0; n<DISK_CACHE_BLOCKS_NUM; ++n) {
    if (blocks[n].read(buff, sector, count)) {
      ++stats.noHits;
      return RES_OK;
    }
  }

  ++stats.noMisses;

  // find free block
  for (int n=0; n<DISK_CACHE_BLOCKS_NUM; ++n) {
    if (blocks[n].empty()) {
      TRACE_DISK_CACHE("\t\t using free block");
      return blocks[n].fill(drv, buff, sector, count);
    }
  }

  // use next block (round robin)
  // TODO: use better strategy to select which used block gets used here
  if (++lastBlock >= DISK_CACHE_BLOCKS_NUM) {
    lastBlock = 0;
  }
  
  return blocks[lastBlock].fill(drv, buff, sector, count);
}

DRESULT DiskCache::write(BYTE drv, const BYTE* buff, DWORD sector, UINT count)
{
  ++stats.noWrites;
  for(int n=0; n < DISK_CACHE_BLOCKS_NUM; ++n) {
    blocks[n].free(sector, count);
  }
  return __disk_write(drv, buff, sector, count);  
}

const DiskCacheStats & DiskCache::getStats() const 
{ 
  return stats; 
}

int DiskCache::getHitRate() const
{
  uint32_t all = stats.noHits + stats.noMisses;
  if (all == 0) return 0;
  return (stats.noHits * 1000) / all;
}

DRESULT disk_read(BYTE drv, BYTE * buff, DWORD sector, UINT count)
{
  return diskCache[drv].read(drv, buff, sector, count);
}


DRESULT disk_write(BYTE drv, const BYTE * buff, DWORD sector, UINT count)
{
  return diskCache[drv].write(drv, buff, sector, count);
}
