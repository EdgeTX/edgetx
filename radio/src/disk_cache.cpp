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

#include "disk_cache.h"
#include "sdcard.h"

#include <string.h>

#if 0  // set to 1 to enable traces
  #include "debug.h"
  #define TRACE_DISK_CACHE(...)   TRACE(__VA_ARGS__)
#else
  #define TRACE_DISK_CACHE(...)
#endif

#if FF_MAX_SS != FF_MIN_SS
#error "Variable sector size is not supported"
#endif

#define BLOCK_SIZE FF_MAX_SS
#define DISK_CACHE_BLOCK_SIZE (DISK_CACHE_BLOCK_SECTORS * BLOCK_SIZE)

DiskCache diskCache;

class DiskCacheBlock
{
 public:
  DiskCacheBlock();
  bool read(BYTE* buff, DWORD sector, UINT count);
  DRESULT fill(const diskio_driver_t* drv, BYTE lun, BYTE* buff, DWORD sector,
               UINT count);
  void free(DWORD sector, UINT count);
  void free();
  bool empty() const;

 private:
  uint8_t data[DISK_CACHE_BLOCK_SIZE];
  DWORD startSector;
  DWORD endSector;
};

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

DRESULT DiskCacheBlock::fill(const diskio_driver_t* drv, BYTE lun, BYTE * buff,
			     DWORD sector, UINT count)
{
  DRESULT res = drv->read(lun, data, sector, DISK_CACHE_BLOCK_SECTORS);
  if (res != RES_OK) {
    return res;
  }
  startSector = sector;
  endSector = sector + DISK_CACHE_BLOCK_SECTORS;
  memcpy(buff, data, count * BLOCK_SIZE);
  TRACE_DISK_CACHE("cache %p FILLED from read(%u, %u)", this, (uint32_t)sector, (uint32_t)count);
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

DiskCache::DiskCache() : lastBlock(0), blocks(nullptr), diskDrv(nullptr)
{
  stats.noHits = 0;
  stats.noMisses = 0;
  stats.noWrites = 0;
}

void DiskCache::initialize(const diskio_driver_t* drv)
{
  blocks = new DiskCacheBlock[DISK_CACHE_BLOCKS_NUM];
  diskDrv = drv;
}

void DiskCache::clear()
{
  lastBlock = 0;
  stats.noHits = 0;
  stats.noMisses = 0;
  stats.noWrites = 0;
  for (int n = 0; n < DISK_CACHE_BLOCKS_NUM; ++n) {
    blocks[n].free();
  }
}

uint32_t DiskCache::getSectors(uint8_t lun)
{
  if (sectors == 0) {
    diskDrv->ioctl(lun, GET_SECTOR_COUNT, &sectors);
  }
  return sectors;
}

DRESULT DiskCache::read(BYTE lun, BYTE * buff, DWORD sector, UINT count)
{
  // if read is bigger than cache block, then read it directly without using cache
  if (count > DISK_CACHE_BLOCK_SECTORS) {
    TRACE_DISK_CACHE("big read(%u, %u)",  (uint32_t)sector, (uint32_t)count);
    return diskDrv->read(lun, buff, sector, count);
  }
  
  // if block + cache block size is beyond the end of the disk,
  // then read it directly without using cache
  if (sector + DISK_CACHE_BLOCK_SECTORS >= getSectors(lun)) {
    TRACE_DISK_CACHE("cache would be beyond end of disk %u (%u)",
		     (uint32_t)sector, getSectors(lun));
    return diskDrv->read(lun, buff, sector, count);
  }

  for (int n = 0; n < DISK_CACHE_BLOCKS_NUM; ++n) {
    if (blocks[n].read(buff, sector, count)) {
      ++stats.noHits;
      return RES_OK;
    }
  }

  ++stats.noMisses;

  // find free block
  for (int n = 0; n < DISK_CACHE_BLOCKS_NUM; ++n) {
    if (blocks[n].empty()) {
      TRACE_DISK_CACHE("\t\t using free block");
      return blocks[n].fill(diskDrv, lun, buff, sector, count);
    }
  }

  // use next block (round robin)
  // TODO: use better strategy to select which used block gets used here
  if (++lastBlock >= DISK_CACHE_BLOCKS_NUM) {
    lastBlock = 0;
  }
  
  return blocks[lastBlock].fill(diskDrv, lun, buff, sector, count);
}

DRESULT DiskCache::write(BYTE lun, const BYTE* buff, DWORD sector, UINT count)
{
  ++stats.noWrites;
  for(int n = 0; n < DISK_CACHE_BLOCKS_NUM; ++n) {
    blocks[n].free(sector, count);
  }
  return diskDrv->write(lun, buff, sector, count);
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

DRESULT disk_cache_read(BYTE drv, BYTE * buff, DWORD sector, UINT count)
{
  return diskCache.read(drv, buff, sector, count);
}

DRESULT disk_cache_write(BYTE drv, const BYTE * buff, DWORD sector, UINT count)
{
  return diskCache.write(drv, buff, sector, count);
}

