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

#include <stddef.h>
#include "edgetx.h"

template <int SIZE_SLOT, int NUM_BINS>
class BinAllocator
{
 private:
  union Bin {
    char data[SIZE_SLOT];
    union Bin* next;
  };
  union Bin bins[NUM_BINS];
  union Bin* freeList;
  int freeCount;

 public:
  BinAllocator()
  {
    freeCount = NUM_BINS;
    memclear(bins, sizeof(bins));
    freeList = &bins[0];
    for (int i = 0; i < NUM_BINS - 1; i += 1) {
      bins[i].next = &bins[i+1];
    }
  }

  bool is_member(void * ptr) {
    return (ptr >= bins[0].data && ptr <= bins[NUM_BINS-1].data);
  }

  bool free(void * ptr)
  {
    if (is_member(ptr)) {
      union Bin* b = (union Bin*)ptr;
      b->next = freeList;
      freeList = b;
      freeCount += 1;
      // TRACE("\tBinAllocator<%d> free %p", SIZE_SLOT, ptr);
      return true;
    }
    // TRACE("\tBinAllocator<%d> free - %p NOT FOUND", SIZE_SLOT, ptr);
    return false;
  }

  void * malloc(size_t size)
  {
    if (size > SIZE_SLOT) {
      // TRACE("BinAllocator<%d> malloc [%lu] size > SIZE_SLOT", SIZE_SLOT, size);
      return nullptr;
    }
    if (freeList) {
      union Bin* b = freeList;
      freeList = freeList->next;
      freeCount -= 1;
      // TRACE("\tBinAllocator<%d> malloc [%lu] - %p", SIZE_SLOT, size, b);
      return b->data;
    }
    // TRACE("BinAllocator<%d> malloc [%lu] no free slots", SIZE_SLOT , size);
    return nullptr;
  }

  size_t size(void * ptr) { return is_member(ptr) ? SIZE_SLOT : 0; }

  bool can_fit(void * ptr, size_t size)
  {
    return is_member(ptr) && size <= SIZE_SLOT;
  }

  int freeSlots() { return freeCount; }
  int avail() { return freeCount * SIZE_SLOT; }
};

#if defined(SIMU)
  typedef BinAllocator<40,300> BinAllocator_slots1;
  typedef BinAllocator<80,100> BinAllocator_slots2;
#else
  typedef BinAllocator<28,200> BinAllocator_slots1;
  typedef BinAllocator<92,50> BinAllocator_slots2;
#endif

BinAllocator_slots1 slots1;
BinAllocator_slots2 slots2;

#if defined(DEBUG)
int SimulateMallocFailure = 0;    //set this to simulate allocation failure
int totalAllocated = 0;
int totalFreed = 0;
int missedAlloc = 0;
int missedFree = 0;
#endif 

int custom_avail()
{
  return slots1.avail() + slots2.avail();
}

static bool bin_free(void * ptr)
{
  // return TRUE if ours
  bool res = slots1.free(ptr);
  if (!res) res = slots2.free(ptr);
#if defined(DEBUG)
  if (res) totalFreed += 1;
  else missedFree += 1;
#endif
  return res;
}

static void * bin_malloc(size_t size) {
  // try to allocate from our space
  void* res = slots1.malloc(size);
  if (!res) res = slots2.malloc(size);
#if defined(DEBUG)
  if (res) totalAllocated += 1;
  else missedAlloc += 1;
#endif
  return res;
}

static void * bin_realloc(void * ptr, size_t size)
{
  if (ptr == nullptr) {
    // no previous data, try our malloc
    return bin_malloc(size);
  }
  else {
    if (! (slots1.is_member(ptr) || slots2.is_member(ptr)) ) {
      // not our data, leave it to libc realloc
      return nullptr;
    }

    // we have existing data
    // if it fits in current slot, return it
    // TODO if new size is smaller, try to relocate in smaller slot
    if ( slots1.can_fit(ptr, size) ) {
      // TRACE("OUR realloc %p[%lu] fits in slot1", ptr, size);
      return ptr;
    }
    if ( slots2.can_fit(ptr, size) ) {
      // TRACE("OUR realloc %p[%lu] fits in slot2", ptr, size);
      return ptr;
    }

    // we need a bigger slot
    void * res = bin_malloc(size);
    if (res == nullptr) {
      // we don't have the space, use libc malloc
      // TRACE("bin_malloc [%lu] FAILURE", size);
      res = malloc(size);
      if (res == nullptr) {
        // TRACE("libc malloc [%lu] FAILURE", size);  
        return nullptr;
      }
    }
    // copy data
    memcpy(res, ptr, slots1.size(ptr) + slots2.size(ptr));
    bin_free(ptr);
    return res;
  }
}

void *custom_l_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
  // (void)ud; (void)osize;  /* not used */

  void* res = nullptr;

  if (nsize == 0) {
    if (ptr) {   // avoid a bunch of NULL pointer free calls
      if (!bin_free(ptr)) {
        // not our range, use libc allocator
        // TRACE("libc free %p", ptr);
        free(ptr);
      }
    }
  } else {
#if defined(DEBUG)
    if (SimulateMallocFailure < 0) {
      // delayed failure
      if (++SimulateMallocFailure == 0)
        SimulateMallocFailure = 1;
    }
    // normal alloc if <= 0, otherwise will return nullptr
    if ( SimulateMallocFailure <= 0) {
#endif
    // try our allocator, if it fails use libc allocator
    res = bin_realloc(ptr, nsize);
    if (res && ptr) {
      // TRACE("OUR realloc %p[%lu] -> %p[%lu]", ptr, osize, res, nsize); 
    }
    if (res == nullptr) {
      res = realloc(ptr, nsize);
      // TRACE("libc realloc %p[%lu] -> %p[%lu]", ptr, osize, res, nsize);
      // if (res == 0 ){
      //   TRACE("realloc FAILURE %lu", nsize);
      //   dumpFreeMemory();
      // }
    }
#if defined(DEBUG)
    }
#endif
  }

#if defined(DEBUG)
  TRACE("bin_l_alloc(%p,%p,%lu,%lu) - S1F %d, S2F %d, TA %d, TF %d, MA %d, MF %d",ud,ptr,osize,nsize,slots1.freeSlots(),slots2.freeSlots(),totalAllocated,totalFreed,missedAlloc,missedFree);
#endif

  return res;
}
