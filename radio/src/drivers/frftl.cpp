/*
 * Copyright (C) EdgeTX
 *
 * Authors:
 *   Dr. Richard Li <richard.li@ces.hk>
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

/*  
 * FrFTL - Flash Resident Flash Translation Layer
 *
 * This is a FTL designed for NOR flash, logical to physical mapping uses 2 layers
 * of translation tables all resident in flash.  It comes with mechanisms to ensure
 * the integrity of the data in previous state when power out occurs in the middle
 * of flash programming.
 *
 * It can be used to back the FatFS library by ChaN and included the support of
 * CTRL_SYNC and CTRL_TRIM functions for best performance.
 * 
 * 
 * How it works:
 * The FTL organize the storage as 4096 byte pages.  The translations from a logical page
 * to a physical page are recorded in the translation table.  The translation table is
 * organized into TT pages in which each TT page can store the mapping of 1024 pages.
 * the TT pages are organized into 2 layers: master TT (MTT) and secondary TT (STT).
 * the logical page no. of MTT is 0, and followed by STT from 1 onwards, therefore
 * all the translation for TT pages are resided in the MTT page.
 * 
 * For logical to physical lookup, there are 2 cases, depends on what the logical page no. is:
 * 1. MTT page -> data page
 * 2. MTT page -> STT page -> data page
 *
 * The page buffer is designed to delay the changes and write back to the flash in larger
 * translation context to minimize the amount of TT page updates, so that the write
 * amplification effect and wearing is minimized and improving the overall read/write
 * performance.
 * 
 */

#include "frftl.h"

#include "crc.h"
#include "definitions.h"

#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE                    512
#define PAGE_SIZE                      4096
#define BLOCK_SIZE                     32768
#define BLOCK_MASK                     (BLOCK_SIZE - 1)
#define PAGES_PER_BLOCK                (BLOCK_SIZE / PAGE_SIZE)
#define USE_BLOCK_ERASE_THRESHOLD      4
#define SECTORS_PER_PAGE               (PAGE_SIZE / SECTOR_SIZE)
#define TT_PAGE_MAGIC                  0xEF87364A
#define TT_RECORDS_PER_PAGE            1024

// The multiplier for cache buffers, min recommendation is 2
#define BUFFER_SIZE_MULTIPLIER         4

// Will cap the buffer size to this when exceeded
#define MAX_BUFFER_SIZE               64

// Reserve pages to minimize the erase cycles when the FS is full,
// should be at least 2 times of BUFFER_SIZE_MULTIPLIER
#define RESERVED_PAGES_MULTIPLIER      16

#define LOCKED   1
#define UNLOCKED 0

typedef enum {
  UNKNOWN,
  USED,
  ERASE_REQUIRED,
  ERASED
} PhysicalPageState;

typedef enum {
  NONE,
  PROGRAM,
  ERASE_PROGRAM,
  RELOCATE_ERASE_PROGRAM
} ProgramMode;

//
// Structures in FLASH
//
typedef struct {
  uint32_t magicStart;
  uint32_t logicalPageNo;
  uint32_t serial;
  uint16_t padding;
  uint16_t crc16;
} TransTableHeader;

typedef struct {
  union {
    TransTableHeader header; // 16 bytes
    uint8_t __padding[1024];                   // 1KB
  };
  uint8_t sectStatus[TT_RECORDS_PER_PAGE];     // 1KB
  uint16_t physicalPageNo[TT_RECORDS_PER_PAGE]; // 2KB
} TransTable;

typedef union {
  TransTable tt;           // 3KB + 16B
  uint8_t data[PAGE_SIZE]; // 4KB
} Page;

//
// Structures in RAM only
//
typedef struct {
  uint16_t physicalPageNo;
  uint8_t sectStatus;
} PageInfo;

typedef struct PageBuffer PageBuffer;

struct PageBuffer {
  Page page;
  PageBuffer* lruPrev;           // Double linked-list for LRU implementation
  PageBuffer* lruNext;           // Double linked-list for LRU implementation
  PageBuffer* hashNext;          // Linked-list for hash table implementation
  uint16_t logicalPageNo;        // Required for first program or reprogram
  uint16_t physicalPageNo;
  uint8_t lock;                  // Page locked for delayed update
  uint8_t sectorProgramRequired; // Record which sector need to be programmed
  uint8_t pMode;
};

static const uint16_t supportedFlashSizes[] = { 4, 8, 16, 32, 64, 128, 256 };

static PhysicalPageState getPhysicalPageState(FrFTL* ftl,
                                              uint16_t physicalPageNo)
{
  uint32_t idx = physicalPageNo >> 4;
  uint32_t result =
      (ftl->physicalPageState[idx] >> ((physicalPageNo & 0xf) * 2));
  return (PhysicalPageState)(result & 0x3);
}

static void setPhysicalPageState(FrFTL* ftl, uint16_t physicalPageNo,
                                 PhysicalPageState state)
{
  uint32_t idx = physicalPageNo >> 4;
  uint32_t mask = 0x3 << ((physicalPageNo & 0xf) * 2);
  ftl->physicalPageState[idx] &= ~mask;
  ftl->physicalPageState[idx] |=
      ((state & 0x3) << ((physicalPageNo & 0xf) * 2));
}


static const uint16_t crc16_ccitt_start = 0xFFFF;

static inline uint16_t crc16_x25_ccitt(const void* buf, uint32_t len) {
  return crc16(CRC_1021, (const uint8_t*)buf, len, crc16_ccitt_start);
}

static inline uint16_t calcCRC(TransTableHeader* header)
{
  header->padding = 0xffff;
  return crc16_x25_ccitt(header, sizeof(TransTableHeader) - 2);
}

static void resolveUnknownState(FrFTL* ftl, uint16_t count)
{
  if (ftl->physicalPageStateResolved) {
    return;
  }

  uint16_t idx = ftl->writeFrontier;
  bool earlyEnd = false;

  const FrFTLOps* cb = ftl->callbacks;
  for (uint16_t i = 0; i < ftl->physicalPageCount; i++) {
    if (getPhysicalPageState(ftl, idx) == UNKNOWN) {
      PhysicalPageState state =
          cb->isFlashErased(idx * PAGE_SIZE) ? ERASED : ERASE_REQUIRED;
      setPhysicalPageState(ftl, idx, state);
      count--;
      if (count == 0) {
        earlyEnd = true;
        break;
      }
    }
    idx++;
    if (idx >= ftl->physicalPageCount) {
      idx = 0;
    }
  }
  if (!earlyEnd) {
    ftl->physicalPageStateResolved = true;
  }
}

static PageBuffer* findPageFromHashTable(FrFTL* ftl, uint16_t physicalPageNo)
{
  PageBuffer** hashTable = (PageBuffer**) ftl->hashTable;
  uint16_t hash = physicalPageNo % ftl->pageBufferSize;
  PageBuffer* lastBuffer = nullptr;
  PageBuffer* buffer = hashTable[hash];
  while (buffer) {
    if (buffer->physicalPageNo == physicalPageNo) {
      // Update search order with LRU
      if (lastBuffer) {
        lastBuffer->hashNext = buffer->hashNext;
        buffer->hashNext = hashTable[hash];
        hashTable[hash] = buffer;
      }
      return buffer;
    }
    lastBuffer = buffer;
    buffer = buffer->hashNext;
  }

  return nullptr;
}

static void removePageFromHashTable(FrFTL* ftl, uint16_t physicalPageNo)
{
  PageBuffer** hashTable = (PageBuffer**) ftl->hashTable;
  uint16_t hash = physicalPageNo % ftl->pageBufferSize;
  PageBuffer* lastBuffer = nullptr;
  PageBuffer* buffer = hashTable[hash];
  while (buffer) {
    if (buffer->physicalPageNo == physicalPageNo) {
      if (lastBuffer == nullptr) {
        hashTable[hash] = buffer->hashNext;
      } else {
        lastBuffer->hashNext = buffer->hashNext;
      }
      buffer->hashNext = nullptr;
      return;
    }
    lastBuffer = buffer;
    buffer = buffer->hashNext;
  }
}

static void addPageToHashTable(FrFTL* ftl, PageBuffer* buffer)
{
  PageBuffer** hashTable = (PageBuffer**) ftl->hashTable;
  uint16_t hash = buffer->physicalPageNo % ftl->pageBufferSize;
  buffer->hashNext = hashTable[hash];
  hashTable[hash] = buffer;
}

static void movePageBufferToLRUHead(FrFTL* ftl, PageBuffer* buffer)
{
  // Take buffer out from the linked list
  if (buffer->lruPrev) {
    buffer->lruPrev->lruNext = buffer->lruNext;
  } else {
    // Already is head, done
    return;
  }

  if (buffer->lruNext) {
    buffer->lruNext->lruPrev = buffer->lruPrev;
  } else {
    // Tail detected, need to update tail pointer
    ftl->bufferTail = buffer->lruPrev;
  }

  // Put buffer to buffer head
  ((PageBuffer*) ftl->bufferHead)->lruPrev = buffer;
  buffer->lruNext = (PageBuffer*) ftl->bufferHead;
  buffer->lruPrev = nullptr;
  ftl->bufferHead = buffer;
}

static PageBuffer* rawFindReplacableBuffer(FrFTL* ftl)
{
  PageBuffer* buffer = (PageBuffer*) ftl->bufferTail;
  while (buffer && buffer->lock == LOCKED) {
    buffer = buffer->lruPrev;
  }

  return buffer;
}

static void movePageBufferToLRUTail(FrFTL* ftl, PageBuffer* buffer)
{
  // Take buffer out from the linked list
  if (buffer->lruNext) {
    buffer->lruNext->lruPrev = buffer->lruPrev;
  } else {
    // Already is tail, done
    return;
  }

  if (buffer->lruPrev) {
    buffer->lruPrev->lruNext = buffer->lruNext;
  } else {
    // Head detected, need to update head pointer
    ftl->bufferHead = buffer->lruNext;
  }

  // Put buffer to buffer tail
  ((PageBuffer*) ftl->bufferTail)->lruNext = buffer;
  buffer->lruPrev = (PageBuffer*) ftl->bufferTail;
  buffer->lruNext = nullptr;
  ftl->bufferTail = buffer;
}

static PageBuffer* findReplacableBuffer(FrFTL* ftl)
{
  PageBuffer* buffer = rawFindReplacableBuffer(ftl);
  if (!buffer)
  {
    // All buffers are locked, flush lock pages
    if (!ftlSync(ftl)) {
      return nullptr;
    }

    // Try again
    buffer = rawFindReplacableBuffer(ftl);
  }

  if (buffer) {
    // Invalidate buffer from hash table
    if (buffer->physicalPageNo != 0xffff) {
      removePageFromHashTable(ftl, buffer->physicalPageNo);
      buffer->physicalPageNo = 0xffff;
    }
    
    // Update LRU
    movePageBufferToLRUHead(ftl, buffer);
  }

  return buffer;
}

static PageBuffer* findPhysicalPageInBuffer(FrFTL* ftl, uint16_t physicalPageNo)
{
  PageBuffer* buffer = findPageFromHashTable(ftl, physicalPageNo);
  if (buffer) {
    // Update LRU
    movePageBufferToLRUHead(ftl, buffer);
  }

  return buffer;
}

static PageBuffer* loadPhysicalPageInBuffer(FrFTL* ftl, uint16_t logicalPageNo,
                                            uint16_t physicalPageNo)
{
  // Find page in buffer
  PageBuffer* currentBuffer = findPhysicalPageInBuffer(ftl, physicalPageNo);
  if (currentBuffer == nullptr) {
    // Page not in buffer, load page in buffer
    if ((currentBuffer = findReplacableBuffer(ftl)) == nullptr) {
      return nullptr;  // BUG
    };

    const FrFTLOps* cb = ftl->callbacks;
    if (!cb->flashRead(physicalPageNo * PAGE_SIZE, currentBuffer->page.data,
                        PAGE_SIZE)) {
      return nullptr;
    }
    currentBuffer->logicalPageNo = logicalPageNo;
    currentBuffer->physicalPageNo = physicalPageNo;
    currentBuffer->lock = UNLOCKED;
    currentBuffer->sectorProgramRequired = 0;
    currentBuffer->pMode = NONE;

    // Add new page to hash table
    addPageToHashTable(ftl, currentBuffer);
  }

  return currentBuffer;
}

static PageBuffer* initPhysicalPageInBuffer(FrFTL* ftl, uint16_t logicalPageNo,
                                            uint16_t physicalPageNo)
{
  // Find page in buffer
  PageBuffer* currentBuffer = findPhysicalPageInBuffer(ftl, physicalPageNo);
  if (currentBuffer == nullptr) {
    // Page not in buffer, init page in buffer
    if ((currentBuffer = findReplacableBuffer(ftl)) == nullptr) {
      return nullptr;  // BUG
    };
    currentBuffer->logicalPageNo = logicalPageNo;
    currentBuffer->physicalPageNo = physicalPageNo;
    currentBuffer->lock = LOCKED;
    currentBuffer->sectorProgramRequired = 0;
    currentBuffer->pMode = ERASE_PROGRAM;
    memset(currentBuffer->page.data, 0xff, PAGE_SIZE);

    // Add new page to hash table
    addPageToHashTable(ftl, currentBuffer);
  }

  return currentBuffer;
}

static bool hasFreeBuffers(FrFTL* ftl, uint16_t bufferCount)
{
  uint16_t freeBufferFound = 0;
  PageBuffer* pageBuffer = (PageBuffer*) ftl->bufferHead;
  while (pageBuffer != nullptr)
  {
    if (pageBuffer->lock == UNLOCKED) {
      freeBufferFound++;
      if (freeBufferFound >= bufferCount) {
        return true;
      }
    }
    pageBuffer = pageBuffer->lruNext;
  }

  return false;
}

static bool readPhysicalSector(FrFTL* ftl, uint8_t* buffer,
                               uint16_t logicalPageNo, uint16_t physicalPageNo,
                               uint8_t pageSectorNo)
{
  PageBuffer* pageBuffer = loadPhysicalPageInBuffer(ftl, logicalPageNo, physicalPageNo);
  if (!pageBuffer) return false;

  memcpy(buffer, pageBuffer->page.data + pageSectorNo * SECTOR_SIZE, SECTOR_SIZE);
  return true;
}

static bool readPhysicalPageInfo(FrFTL* ftl, PageInfo* pageInfo,
                                 uint16_t logicalPageNo,
                                 uint16_t physicalPageNo, uint16_t recordNo)
{
  PageBuffer* pageBuffer = loadPhysicalPageInBuffer(ftl, logicalPageNo, physicalPageNo);
  if (!pageBuffer) return false;
  
  // Page found in buffer
  const TransTable* tt = &pageBuffer->page.tt;
  pageInfo->physicalPageNo = tt->physicalPageNo[recordNo];
  pageInfo->sectStatus = tt->sectStatus[recordNo];
  return true;
}

static bool readPageInfo(FrFTL* ftl, PageInfo* pageInfo, uint16_t logicalPageNo)
{
  if (logicalPageNo < TT_RECORDS_PER_PAGE) {
    // Read from master TT
    return readPhysicalPageInfo(ftl, pageInfo, 0, ftl->mttPhysicalPageNo,
                                logicalPageNo);
  }
  
  // Lookup from secondary TT from master TT
  PageInfo secondaryTTInfo;
  uint16_t sttLogicalPageNo = logicalPageNo / TT_RECORDS_PER_PAGE;
  if (!readPhysicalPageInfo(ftl, &secondaryTTInfo, 0, ftl->mttPhysicalPageNo,
			    sttLogicalPageNo)) {
    return false;
  }

  // Read from secondary TT
  return readPhysicalPageInfo(ftl, pageInfo, sttLogicalPageNo,
			      secondaryTTInfo.physicalPageNo,
			      logicalPageNo % TT_RECORDS_PER_PAGE);
}

static bool updatePhysicalPageInfo(FrFTL* ftl, PageInfo* pageInfo,
                                   uint16_t logicalPageNo,
                                   uint16_t physicalPageNo, uint16_t recordNo)
{
  PageBuffer* pageBuffer =
      loadPhysicalPageInBuffer(ftl, logicalPageNo, physicalPageNo);
  if (!pageBuffer) {
    return false;
  }

  // Update info, need to lock and ensure update
  pageBuffer->lock = LOCKED;
  if (pageBuffer->pMode == NONE) {
    pageBuffer->pMode = PROGRAM;
  }

  TransTable* tt = &pageBuffer->page.tt;
  tt->physicalPageNo[recordNo] = pageInfo->physicalPageNo;
  tt->sectStatus[recordNo] = pageInfo->sectStatus;

  return true;
}

static bool updatePageInfo(FrFTL* ftl, PageInfo* pageInfo,
                           uint16_t logicalPageNo)
{
  if (logicalPageNo < TT_RECORDS_PER_PAGE) {
    // Update to master TT
    return updatePhysicalPageInfo(ftl, pageInfo, 0, ftl->mttPhysicalPageNo,
                                  logicalPageNo);
  } else {
    // Lookup from secondary TT from master TT
    PageInfo secondaryTTInfo;
    uint16_t sttLogicalPageNo = logicalPageNo / TT_RECORDS_PER_PAGE;
    if (!readPhysicalPageInfo(ftl, &secondaryTTInfo, 0, ftl->mttPhysicalPageNo,
                              sttLogicalPageNo)) {
      return false;
    }

    // Program to secondary TT
    return updatePhysicalPageInfo(ftl, pageInfo, sttLogicalPageNo,
                                  secondaryTTInfo.physicalPageNo,
                                  logicalPageNo % TT_RECORDS_PER_PAGE);
  }
}

static uint16_t allocatePhysicalPage(FrFTL* ftl)
{
  uint16_t lookupCount = 0;
  while (getPhysicalPageState(ftl, ftl->writeFrontier) == USED) {
    ftl->writeFrontier++;
    if (ftl->writeFrontier >= ftl->physicalPageCount) {
      ftl->writeFrontier = 0;
    }
    lookupCount++;
    if (lookupCount > ftl->physicalPageCount) {
      return 0xffff;  // BUG
    }
  }

  uint16_t physicalPageNo = ftl->writeFrontier++;
  if (ftl->writeFrontier >= ftl->physicalPageCount) {
    ftl->writeFrontier = 0;
  }

  return physicalPageNo;
}

static bool quickErase(FrFTL* ftl, uint32_t addr)
{

  const FrFTLOps* cb = ftl->callbacks;
  if ((addr & BLOCK_MASK) == 0) {
    // Block aligned

    uint16_t ppn = addr / PAGE_SIZE;
    uint8_t count = 0;
    bool hasUsed = false;

    // Check state of the whole jumbo page
    for (uint8_t i = 0; i < PAGES_PER_BLOCK; i++) {
      PhysicalPageState state = getPhysicalPageState(ftl, ppn + i);
      if (state == UNKNOWN || state == ERASE_REQUIRED) {
        count++;
      }
      if (state == USED) {
        hasUsed = true;
        break;  // Cannot use block erase
      }
    }

    if (!hasUsed && count >= USE_BLOCK_ERASE_THRESHOLD) {
      bool ret = cb->flashBlockErase(addr);
      if (ret) {
        for (uint8_t i = 0; i < PAGES_PER_BLOCK; i++) {
          setPhysicalPageState(ftl, ppn + i, ERASED);
        }
      }
      return ret;
    }
  }
  return cb->flashErase(addr);
}

static bool programPage(FrFTL* ftl, PageBuffer* buffer, bool doErase)
{
  const FrFTLOps* cb = ftl->callbacks;
  uint32_t pageAddr = buffer->physicalPageNo * PAGE_SIZE;
        
  if (doErase && getPhysicalPageState(ftl, buffer->physicalPageNo) != ERASED) {
    // Do erase on the fly
    if (!quickErase(ftl, pageAddr)) {
      return false;
    }
  }

  if (buffer->logicalPageNo < ftl->ttPageCount) {
    buffer->sectorProgramRequired = 0xff;
  }

  // Sector by sector programming:
  // As flash requires 256 bytes per program command, it will be more efficient to program by sector
  uint8_t* dataPtr = buffer->page.data;
  for (uint8_t i = 0; i < SECTORS_PER_PAGE; i++)
  {
    uint8_t sectMask = 1 << i;
    if ((buffer->sectorProgramRequired & sectMask) != 0) {
      if (!cb->flashProgram(pageAddr, dataPtr, SECTOR_SIZE))
      {
        return false;
      }
    }
    pageAddr += SECTOR_SIZE;
    dataPtr += SECTOR_SIZE;
  }

  return true;
}

static bool programPageInBuffer(FrFTL* ftl, PageBuffer* buffer)
{
  uint16_t oldPhysicalPageNo;

  switch (buffer->pMode) {
    case PROGRAM:
      // Program only
      if (!programPage(ftl, buffer, false)) {
        return false;
      }
      setPhysicalPageState(ftl, buffer->physicalPageNo, USED);
      if (!findPageFromHashTable(ftl, buffer->physicalPageNo)) {
        addPageToHashTable(ftl, buffer);
      }
      break;
    case ERASE_PROGRAM:
      if (!programPage(ftl, buffer, true)) {
        return false;
      }
      setPhysicalPageState(ftl, buffer->physicalPageNo, USED);
      if (!findPageFromHashTable(ftl, buffer->physicalPageNo)) {
        addPageToHashTable(ftl, buffer);
      }
      break;
    case RELOCATE_ERASE_PROGRAM:
      // Reprogram
      oldPhysicalPageNo = buffer->physicalPageNo;
      setPhysicalPageState(ftl, oldPhysicalPageNo, ERASE_REQUIRED);
      removePageFromHashTable(ftl, oldPhysicalPageNo);
      buffer->physicalPageNo = allocatePhysicalPage(ftl);
      if (buffer->physicalPageNo == 0xffff) {
        return false;
      }

      if (buffer->logicalPageNo < ftl->ttPageCount) {
        if (buffer->logicalPageNo == 0) {
          // MTT need update physicalPageNo
          buffer->page.tt.physicalPageNo[0] = buffer->physicalPageNo;
        }

        // TT page, need update serial and CRC
        buffer->page.tt.header.serial++;
        buffer->page.tt.header.crc16 = calcCRC(&buffer->page.tt.header);
      }

      if (!programPage(ftl, buffer, true)) {
        return false;
      }

      setPhysicalPageState(ftl, buffer->physicalPageNo, USED);
      addPageToHashTable(ftl, buffer);
      if (buffer->logicalPageNo == 0) {
        // MTT page, need update mttPhysicalPageNo
        ftl->mttPhysicalPageNo = buffer->physicalPageNo;
      }
      break;

    default:
      break;
  }

  return true;
}

static bool lockTTPages(FrFTL* ftl, uint16_t logicalPageNo)
{
  // Read TT pages and lock it for later update
  PageBuffer* ttBuffer;
  PageInfo ttPageInfo;
  uint16_t ttPageNo = logicalPageNo / TT_RECORDS_PER_PAGE;
  if (!readPageInfo(ftl, &ttPageInfo, ttPageNo)) {
    return false;
  }
  ttBuffer =
      loadPhysicalPageInBuffer(ftl, ttPageNo, ttPageInfo.physicalPageNo);
  ttBuffer->lock = LOCKED;
  ttBuffer->pMode = RELOCATE_ERASE_PROGRAM;
  if (ttPageNo > 0) {
    // TT page not MTT page, need to lock MTT page as well
    ttBuffer = loadPhysicalPageInBuffer(ftl, 0, ftl->mttPhysicalPageNo);
    ttBuffer->lock = LOCKED;
    ttBuffer->pMode = RELOCATE_ERASE_PROGRAM;
  }
  return true;
}

bool ftlSync(FrFTL* ftl)
{
  PageBuffer* pageBuffer = ((PageBuffer*)(ftl->pageBuffer));

  // First program data pages
  for (uint16_t i = 0; i < ftl->pageBufferSize; i++) {
    PageBuffer* currentBuffer = pageBuffer + i;
    if (currentBuffer->lock == LOCKED) {
      if (currentBuffer->logicalPageNo >= ftl->ttPageCount) {
        if (!programPageInBuffer(ftl, currentBuffer)) {
          return false;
        }

        // Update PageInfo in TT pages
        PageInfo pageInfo;
        if (!readPageInfo(ftl, &pageInfo, currentBuffer->logicalPageNo)) {
          return false;
        }
        pageInfo.physicalPageNo = currentBuffer->physicalPageNo;
        if (!updatePageInfo(ftl, &pageInfo, currentBuffer->logicalPageNo)) {
          return false;
        }

        // Unlock buffer
        currentBuffer->lock = UNLOCKED;
        currentBuffer->sectorProgramRequired = 0;
        currentBuffer->pMode = NONE;
      }
    }
  }

  // Second program STT pages
  PageBuffer* mttBuffer =
      loadPhysicalPageInBuffer(ftl, 0, ftl->mttPhysicalPageNo);
  if (!mttBuffer) {
    return false;
  }
  for (uint16_t i = 0; i < ftl->pageBufferSize; i++) {
    PageBuffer* currentBuffer = pageBuffer + i;
    if (currentBuffer->lock == LOCKED) {
      int16_t logicalPageNo = currentBuffer->logicalPageNo;
      if (logicalPageNo > 0 &&
          logicalPageNo < ftl->ttPageCount) {
        if (!programPageInBuffer(ftl, currentBuffer)) {
          return false;
        }

        // Update PageInfo in MTT page
        TransTable* tt = &mttBuffer->page.tt;
        tt->physicalPageNo[logicalPageNo] = currentBuffer->physicalPageNo;

        // Unlock buffer
        currentBuffer->lock = UNLOCKED;
        currentBuffer->pMode = NONE;
      }
    }
  }

  // Finally program MTT page
  if (mttBuffer->lock == LOCKED) {
    if (!programPageInBuffer(ftl, mttBuffer)) {
      return false;
    }

    // Unlock buffer
    mttBuffer->lock = UNLOCKED;
    mttBuffer->pMode = NONE;
  }

  return true;
}

bool ftlWrite(FrFTL* ftl, uint32_t startSectorNo, uint32_t noOfSectors,
              const uint8_t* buf)
{
  resolveUnknownState(ftl, ftl->ttPageCount);
  if (startSectorNo + noOfSectors > ftl->usableSectorCount) {
    return false;
  }

  uint32_t sectorNo = startSectorNo;
  while (noOfSectors > 0) {
    // Max no. of sectors need to be rewritten is 3,
    // need to ensure has enough free buffers
    if (!hasFreeBuffers(ftl, 3)) {
      // Flush the buffers first if free space is not found
      if (!ftlSync(ftl)) {
        return false;
      }
    }

    uint16_t logicalPageNo = sectorNo / SECTORS_PER_PAGE + ftl->ttPageCount;
    uint8_t pageSectorNo = sectorNo % SECTORS_PER_PAGE;

    // Read page info
    PageInfo pageInfo;
    readPageInfo(ftl, &pageInfo, logicalPageNo);
    PageBuffer* dataBuffer;

    // Allocate new physical page for uninitialized logical page
    if (pageInfo.physicalPageNo == 0xffff) {
      // Need allocate physical page
      if ((pageInfo.physicalPageNo = allocatePhysicalPage(ftl)) == 0xffff) {
        return false;
      }

      // Init page in buffer, locked for delayed program
      dataBuffer = initPhysicalPageInBuffer(ftl, logicalPageNo, pageInfo.physicalPageNo);
      if (!dataBuffer) {
        return false;
      }

      pageInfo.sectStatus = 0xff;

      if (!updatePageInfo(ftl, &pageInfo, logicalPageNo)) {
        return false;
      }
    } else {
      dataBuffer =
          loadPhysicalPageInBuffer(ftl, logicalPageNo, pageInfo.physicalPageNo);
      if (!dataBuffer) {
        return false;
      }
    }

    uint8_t sectMask = 1 << pageSectorNo;
    if ((pageInfo.sectStatus & sectMask) != 0) {
      // Sector never write, append information
      pageInfo.sectStatus &= ~sectMask;
      if (!updatePageInfo(ftl, &pageInfo, logicalPageNo)) {
        return false;
      }

      // Update sector, locked for delayed program
      dataBuffer->lock = LOCKED;
      if (dataBuffer->pMode == NONE) {
        dataBuffer->pMode = PROGRAM;
      }
      memcpy(dataBuffer->page.data + pageSectorNo * SECTOR_SIZE, buf,
             SECTOR_SIZE);
      dataBuffer->sectorProgramRequired |= sectMask;
    } else {
      // Sector already written, use replace write
      // Lock data page for delayed update with reprogram
      dataBuffer->lock = LOCKED;
      dataBuffer->pMode = RELOCATE_ERASE_PROGRAM;
      memcpy(dataBuffer->page.data + pageSectorNo * SECTOR_SIZE, buf,
             SECTOR_SIZE);
      dataBuffer->sectorProgramRequired = ~pageInfo.sectStatus;

      if (!lockTTPages(ftl, logicalPageNo)) {
        return false;
      }
    }

    noOfSectors--;
    sectorNo++;
    buf += SECTOR_SIZE;
  }

  return true;
}

bool ftlRead(FrFTL* ftl, uint32_t sectorNo, uint8_t* buffer)
{
  //  doGC(ftl, ftl->ttPageCount, 1);
  if (sectorNo >= ftl->usableSectorCount) {
    return false;
  }

  uint16_t logicalPageNo = sectorNo / SECTORS_PER_PAGE + ftl->ttPageCount;
  uint8_t pageSectorNo = sectorNo % SECTORS_PER_PAGE;

  // Read page info
  PageInfo pageInfo;
  readPageInfo(ftl, &pageInfo, logicalPageNo);

  // Check if sector written before
  uint8_t sectMask = 1 << pageSectorNo;
  if ((pageInfo.sectStatus & sectMask) != 0) {
    // Sector never write, return init content
    memset(buffer, 0xff, SECTOR_SIZE);
    return true;
  }

  return readPhysicalSector(ftl, buffer, logicalPageNo,
			    pageInfo.physicalPageNo, pageSectorNo);
}

bool ftlTrim(FrFTL* ftl, uint32_t startSectorNo, uint32_t noOfSectors)
{
  resolveUnknownState(ftl, ftl->ttPageCount);
  if (startSectorNo + noOfSectors > ftl->usableSectorCount) {
    return false;
  }

  uint32_t sectorNo = startSectorNo;
  while (noOfSectors > 0) {
    // Max no. of sectors need to be rewritten is 3,
    // need to ensure has enough free buffers
    if (!hasFreeBuffers(ftl, 3)) {
      // Flush the buffers first if free space is not found
      if (!ftlSync(ftl)) {
        return false;
      }
    }

    uint16_t logicalPageNo = sectorNo / SECTORS_PER_PAGE + ftl->ttPageCount;
    uint8_t pageSectorNo = sectorNo % SECTORS_PER_PAGE;

    // Read page info
    PageInfo pageInfo;
    readPageInfo(ftl, &pageInfo, logicalPageNo);

    // Check if physical page in used
    if (pageInfo.physicalPageNo != 0xffff) {
      uint8_t sectMask = 1 << pageSectorNo;
      if ((pageInfo.sectStatus & sectMask) == 0) {
        // Sector used, free it
        PageBuffer* dataBuffer = loadPhysicalPageInBuffer(
            ftl, logicalPageNo, pageInfo.physicalPageNo);
        if (!dataBuffer) {
          return false;
        }

        pageInfo.sectStatus |= sectMask;
        if (pageInfo.sectStatus == 0xff) {
          // Free whole page
          setPhysicalPageState(ftl, pageInfo.physicalPageNo, ERASE_REQUIRED);
          removePageFromHashTable(ftl, pageInfo.physicalPageNo);
          pageInfo.physicalPageNo = 0xffff;     // Invalidate page info
          dataBuffer->physicalPageNo = 0xffff;  // Invalidate buffer
          dataBuffer->lock = UNLOCKED;

          // Move freed page buffer to LRU tail
          movePageBufferToLRUTail(ftl, dataBuffer);

        } else {
          // Locked for delayed relocate, fill and program
          dataBuffer->lock = LOCKED;
          dataBuffer->pMode = RELOCATE_ERASE_PROGRAM;
          dataBuffer->sectorProgramRequired = ~pageInfo.sectStatus;
        }

        // Update page info
        if (!updatePageInfo(ftl, &pageInfo, logicalPageNo)) {
          return false;
        }

        if (!lockTTPages(ftl, logicalPageNo)) {
          return false;
        }
      }
    }
    noOfSectors--;
    sectorNo++;
  }

  return true;
}

static void initPageBuffer(FrFTL* ftl)
{
  // Init page buffer
  uint32_t bufferSize = sizeof(PageBuffer) * ftl->pageBufferSize;
  ftl->memoryUsed += bufferSize;
  ftl->pageBuffer = malloc(bufferSize);
  ftl->bufferHead = ftl->pageBuffer;

  for (uint16_t i = 0; i < ftl->pageBufferSize; i++) {
    PageBuffer* currentBuffer = ((PageBuffer*)ftl->pageBuffer) + i;
    if (i == 0) {
      // First buffer
      currentBuffer->lruPrev = nullptr;
    } else {
      currentBuffer->lruPrev = ((PageBuffer*)ftl->pageBuffer) + i - 1;
    }
    if (i == ftl->pageBufferSize - 1) {
      // Last buffer
      currentBuffer->lruNext = nullptr;
      ftl->bufferTail = currentBuffer;
    } else {
      currentBuffer->lruNext = ((PageBuffer*)ftl->pageBuffer) + i + 1;
    }
    currentBuffer->hashNext = nullptr;
    currentBuffer->logicalPageNo = 0xffff;
    currentBuffer->physicalPageNo = 0xffff;
    currentBuffer->lock = UNLOCKED;
    currentBuffer->pMode = NONE;
  }

  // Init hash table
  bufferSize = sizeof(PageBuffer*) * ftl->pageBufferSize;
  ftl->memoryUsed += bufferSize;
  ftl->hashTable = malloc(bufferSize);
  memset(ftl->hashTable, 0, bufferSize);
}

static void initTransTablePage(TransTable* tt, uint32_t logicalPageNo)
{
  memset(tt, 0xff, sizeof(TransTable));
  TransTableHeader* header = &tt->header;
  header->magicStart = TT_PAGE_MAGIC;
  header->logicalPageNo = logicalPageNo;
  header->serial = 1;
  header->crc16 = calcCRC(header);
}

static void updateTransTablePage(TransTable* tt, uint32_t logicalPageNo)
{
  TransTableHeader* header = &tt->header;
  header->logicalPageNo = logicalPageNo;
  header->crc16 = calcCRC(header);
}

void createFTL(FrFTL* ftl)
{
  // Resolve the first few blocks for proper startup
  ftl->writeFrontier = 0;
  resolveUnknownState(ftl, ftl->pageBufferSize);

  uint16_t i = 1;
  TransTable tt;
  initTransTablePage(&tt, i);

  uint32_t addr = PAGE_SIZE;
  const FrFTLOps* cb = ftl->callbacks;
  do {
    if (getPhysicalPageState(ftl, i) != ERASED) {
      quickErase(ftl, addr);
    }
    cb->flashProgram(addr, (const uint8_t*)&tt, PAGE_SIZE);
    setPhysicalPageState(ftl, i, USED);

    i += 1;
    addr += PAGE_SIZE;
    updateTransTablePage(&tt, i);
    
  } while(i < ftl->ttPageCount);

  i = 0;
  initTransTablePage(&tt, i);
  do {
    tt.physicalPageNo[i] = i;
    tt.sectStatus[i] = 0;
  } while(++i < ftl->ttPageCount);

  if (getPhysicalPageState(ftl, 0) != ERASED) {
    quickErase(ftl, 0);
  }

  cb->flashProgram(0, (const uint8_t*)&tt, PAGE_SIZE);
  setPhysicalPageState(ftl, 0, USED);

  ftl->writeFrontier = ftl->ttPageCount;
}

static bool loadFTL(FrFTL* ftl)
{
  // Scan for MTT
  uint32_t currentSerial = 0;
  uint16_t currentPhysicalMTTPageNo = 0xffff;

  const FrFTLOps* cb = ftl->callbacks;
  for (uint16_t i = 0; i < ftl->physicalPageCount; i++) {
    TransTableHeader header;
    if (!cb->flashRead(i * PAGE_SIZE, (uint8_t*)&header, sizeof(header))) {
      return false;
    }
    if (header.magicStart == TT_PAGE_MAGIC && header.logicalPageNo == 0 &&
        header.crc16 == calcCRC(&header)) {
      // MTT detected
      if (header.serial > currentSerial || currentSerial - header.serial > 0x7fffffff) {
        // Newer MTT found
        currentSerial = header.serial;
        currentPhysicalMTTPageNo = i;
      }
    }
  }

  if (currentPhysicalMTTPageNo != 0xffff) {
    // MTT found, load data
    ftl->mttPhysicalPageNo = currentPhysicalMTTPageNo;
    setPhysicalPageState(ftl, currentPhysicalMTTPageNo, USED);
    ftl->writeFrontier = currentPhysicalMTTPageNo + 1;
    if (ftl->writeFrontier >= ftl->physicalPageCount) {
      ftl->writeFrontier = 0;
    }

    PageBuffer* mtt =
        loadPhysicalPageInBuffer(ftl, 0, currentPhysicalMTTPageNo);

    // Check if MTT valid
    if (!mtt || mtt->page.tt.physicalPageNo[0] != currentPhysicalMTTPageNo) {
      return false;
    }

    PageBuffer* tt = mtt;

    for (uint16_t i = 0; i < ftl->ttPageCount; i++) {
      if (i > 0) {
        // Not MTT, read TT page and check integrity
        uint16_t ttPhysicalPageNo = mtt->page.tt.physicalPageNo[i];

        // Check wrong range of physical page no.
        if (ttPhysicalPageNo >= ftl->physicalPageCount) {
          return false;
        }

        // Load TT page
        tt = loadPhysicalPageInBuffer(ftl, i, ttPhysicalPageNo);
        if (!tt) {
          return false;
        }

        // Check if TT valid
        if (tt->page.tt.header.magicStart != TT_PAGE_MAGIC ||
            tt->page.tt.header.crc16 != calcCRC(&tt->page.tt.header)) {
          return false;
        }
      }

      // Mark used page
      for (uint16_t j = 0; j < TT_RECORDS_PER_PAGE; j++) {
        uint16_t currentPhysicalPageNo = tt->page.tt.physicalPageNo[j];
        if (currentPhysicalPageNo != 0xffff) {
          // Check wrong range of physical page no.
          if (currentPhysicalPageNo >= ftl->physicalPageCount) {
            return false;
          }
          // Used page
          setPhysicalPageState(ftl, currentPhysicalPageNo, USED);
        }
      }
    }

    // Walk forward to ensure some pages are resolved
    resolveUnknownState(ftl, ftl->pageBufferSize);
    return true;
  }

  return false;
}

bool ftlInit(FrFTL* ftl, const FrFTLOps* cb, uint16_t flashSizeInMB)
{
  // Check flash size
  bool found = false;
  for (uint8_t i = 0; i < sizeof(supportedFlashSizes); i++) {
    if (flashSizeInMB == supportedFlashSizes[i]) {
      found = true;
      break;
    }
  }
  if (!found) {
    return false;
  }

  memset(ftl, 0, sizeof(FrFTL));
  ftl->callbacks = cb;
  ftl->mttPhysicalPageNo = 0;
  ftl->physicalPageCount = flashSizeInMB > 128 ? 65535 : flashSizeInMB * 1024 * 1024 / PAGE_SIZE;
  ftl->ttPageCount = ftl->physicalPageCount / TT_RECORDS_PER_PAGE + (ftl->physicalPageCount % TT_RECORDS_PER_PAGE > 0 ? 1 : 0);
  ftl->usableSectorCount =
      (ftl->physicalPageCount - ftl->ttPageCount * RESERVED_PAGES_MULTIPLIER) *
      SECTORS_PER_PAGE;
  uint32_t stateSize =
      ftl->physicalPageCount / 16 + (ftl->physicalPageCount % 16 > 0 ? 1 : 0);
  ftl->physicalPageState = (uint32_t*)calloc(stateSize, sizeof(uint32_t));
  ftl->physicalPageStateResolved = false;
  ftl->memoryUsed += stateSize * sizeof(uint32_t);
  ftl->pageBufferSize = ftl->ttPageCount * BUFFER_SIZE_MULTIPLIER;
  if (ftl->pageBufferSize > MAX_BUFFER_SIZE) {
    ftl->pageBufferSize = MAX_BUFFER_SIZE;
  }
  initPageBuffer(ftl);

  if (!loadFTL(ftl)) {
    // Need reset physical page state before create
    memset(ftl->physicalPageState, 0, stateSize * sizeof(uint32_t));
    createFTL(ftl);
  }
  return true;
}

void ftlDeInit(FrFTL* ftl)
{
  free(ftl->pageBuffer);
  free(ftl->physicalPageState);
  free(ftl->hashTable);
}
