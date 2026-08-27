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

#include <stdlib.h>
#include "edgetx.h"

/*
  Custom memory allocator for Lua that adds all available CCM RAM to the memory pool
  - maintains a free list of available CCM RAM, order by address
  - allocates from the smallest available free block
  - coalesces adjacent blocks on free
*/

typedef struct _memblk {
  int size;             // actual size of block. Available / allocated size = 'size' - sizeof(int)
  struct _memblk* next;
} memblk;

#define MIN_SIZE  12    // Minimum size for free block (4 for 'size' field, 8 for data)

// CCM RAM available region
extern memblk _ccm_heap_start;
extern int _ccm_heap_end;

// Init flag
static bool started = false;

// Free list
static memblk* ccm_list = nullptr;

// CCM region for checking if blocks belong here
static unsigned char* ccm_start;
static unsigned char* ccm_end;

// Initialise free list
static void init()
{
  if (!started) {
    started = true;

    ccm_start = (unsigned char*)&_ccm_heap_start;
    ccm_end = (unsigned char*)&_ccm_heap_end;

    ccm_list = &_ccm_heap_start;
    ccm_list->next = nullptr;
    ccm_list->size = ccm_end - ccm_start;
  }
}

// Return free memory available
uint32_t custom_avail()
{
  uint32_t free = 0;
  for (memblk* b = ccm_list; b; b = b->next) {
    free += (b->size - sizeof(int));
  }
  return free;
}

// Free memory block
static void ccm_free(void* ptr)
{
  // Check that block belongs in CCM RAM
  if (ptr >= ccm_start && ptr < ccm_end) {
    memblk* prev = nullptr;
    memblk* next = ccm_list;
    memblk* p = (memblk*)(((unsigned char*)ptr) - sizeof(int));
    // Find place in list
    while (next && (p > next)) {
      prev = next;
      next = next->next;
    }
    // Link into list
    p->next = next;
    if (prev == nullptr) {
      ccm_list = p;
    } else {
      prev->next = p;
      if ((((unsigned char*)prev) + prev->size) == (unsigned char*)p) {
        // Merge adjacent
        prev->next = p->next;
        prev->size += p->size;
        p = prev;
      }
    }
    if ((((unsigned char*)p) + p->size) == (unsigned char*)next) {
      // Merge adjacent
      p->next = next->next;
      p->size += next->size;
    }
  } else {
    // Not CCM RAM - let system handle it.
    free(ptr);
  }
}

// Allocate new block
static void* ccm_malloc(size_t nsize)
{
  // Add space for 'size' field and align to 32 bit size
  int asize = (nsize + sizeof(int) + 3) & 0xFFFFFFFC;

  memblk* prev = nullptr;
  memblk* next = ccm_list;
  memblk* p = nullptr;
  memblk* pprev = nullptr;
  // Find smallest block that will fit
  while (next) {
    if (next->size >= asize) {
      if (!p || (p && (p->size < next->size))) {
        pprev = prev;
        p = next;
        if (p->size == asize) break;  // exact fit
      }
    }
    prev = next;
    next = next->next;
  }

  void* res;

  if (p) {
    if (p->size <= (asize + MIN_SIZE)) {
      // Using entire block - unlink it
      if (pprev)
        pprev->next = p->next;
      else
        ccm_list = p->next;
    } else {
      // reduce size of block and return end part
      p->size -= asize;
      p = (memblk*)(((unsigned char*)p) + p->size);
      p->size = asize;
    }
    res = ((unsigned char*)p) + sizeof(int);
  } else {
    // No space - let system handle it
    res = malloc(nsize);
  }

  return res;
}

// Re-allocate existing block
static void* ccm_realloc(void* ptr, size_t osize, size_t nsize)
{
  // Add space for 'size' field and align to 32 bit size
  int asize = (nsize + sizeof(int) + 3) & 0xFFFFFFFC;

  // Check that block belongs in CCM RAM
  if (ptr >= ccm_start && ptr < ccm_end) {
    memblk* p = (memblk*)(((unsigned char*)ptr) - sizeof(int));
    if (asize <= p->size)
      return ptr; // fits in already allocated space

    void* np = ccm_malloc(nsize);
    memcpy(np, ptr, osize);
    ccm_free(ptr);
    return np;
  } else {
    // In theory nsize might fit in available CCM RAM; but since block is
    // already in normal RAM we just let system handle it.
    return realloc(ptr, nsize);
  }
}

void* custom_l_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
{
  (void)ud; /* not used */

  init();

  if (nsize == 0) {
    if (ptr) {   // avoid a bunch of NULL pointer free calls
      ccm_free(ptr);
    }
    return nullptr;
  }
  else if (ptr) {
    return ccm_realloc(ptr, osize, nsize);
  } else {
    return ccm_malloc(nsize);
  }
}
