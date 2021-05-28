/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _BIN_ALLOCATOR_H_
#define _BIN_ALLOCATOR_H_

#include "debug.h"

template <int SIZE_SLOT, int NUM_BINS> class BinAllocator {
private:
  PACK(struct Bin {
    char data[SIZE_SLOT];
    bool Used;
  });
  struct Bin Bins[NUM_BINS];
  int NoUsedBins;
public:
  BinAllocator() : NoUsedBins(0) {
    memclear(Bins, sizeof(Bins));
  }
  bool free(void * ptr) {
    for (size_t n = 0; n < NUM_BINS; ++n) {
      if (ptr == Bins[n].data) {
        Bins[n].Used = false;
        --NoUsedBins;
        // TRACE("\tBinAllocator<%d> free %lu ------", SIZE_SLOT, n);
        return true;
      }
    }
    return false;
  }
  bool is_member(void * ptr) {
    return (ptr >= Bins[0].data && ptr <= Bins[NUM_BINS-1].data);
  }
  void * malloc(size_t size) {
    if (size > SIZE_SLOT) {
      // TRACE("BinAllocator<%d> malloc [%lu] size > SIZE_SLOT", SIZE_SLOT, size);
      return 0;
    }
    if (NoUsedBins >= NUM_BINS) {
      // TRACE("BinAllocator<%d> malloc [%lu] no free slots", SIZE_SLOT, size);
      return 0;
    }
    for (size_t n = 0; n < NUM_BINS; ++n) {
      if (!Bins[n].Used) {
        Bins[n].Used = true;
        ++NoUsedBins;
        // TRACE("\tBinAllocator<%d> malloc %lu[%lu]", SIZE_SLOT, n, size);
        return Bins[n].data;
      }
    }
    // TRACE("BinAllocator<%d> malloc [%lu] no free slots", SIZE_SLOT , size);
    return 0;
  }
  size_t size(void * ptr) {
    return is_member(ptr) ? SIZE_SLOT : 0;
  }
  bool can_fit(void * ptr, size_t size) {
    return is_member(ptr) && size <= SIZE_SLOT;  //todo is_member check is redundant
  }
  unsigned int capacity() { return NUM_BINS; }
  unsigned int size() { return NoUsedBins; }
};

#if defined(SIMU)
typedef BinAllocator<39,300> BinAllocator_slots1;
typedef BinAllocator<79,100> BinAllocator_slots2;
#else
typedef BinAllocator<27,200> BinAllocator_slots1;
typedef BinAllocator<91,50> BinAllocator_slots2;
#endif

#if defined(USE_BIN_ALLOCATOR)
extern BinAllocator_slots1 slots1;
extern BinAllocator_slots2 slots2;

// wrapper for our BinAllocator for Lua
void *bin_l_alloc (void *ud, void *ptr, size_t osize, size_t nsize);
#endif   //#if defined(USE_BIN_ALLOCATOR)

#endif // _BIN_ALLOCATOR_H_
