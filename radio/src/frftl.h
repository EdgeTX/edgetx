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
 */

#ifndef _FRFTL_H_
#define _FRFTL_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef bool (*FlashReadCB)(uint32_t addr, uint8_t* buf, uint32_t len);
  typedef bool (*FlashProgramCB)(uint32_t addr, const uint8_t* buf, uint32_t len);
  typedef bool (*FlashEraseCB)(uint32_t addr);
  typedef bool (*IsFlashErasedCB)(uint32_t addr);
  
  typedef struct
  {
    FlashReadCB flashRead;
    FlashProgramCB flashProgram;
    FlashEraseCB flashErase;
    IsFlashErasedCB isFlashErased;
    uint32_t mttPhysicalPageNo;
    uint16_t physicalPageCount;
    uint8_t ttPageCount;
    uint32_t usableSectorCount;
    uint16_t writeFrontier;
    uint32_t* physicalPageState;
    bool physicalPageStateResolved;
    uint16_t pageBufferSize;
    void *pageBuffer;
    size_t memoryUsed;
  } FrFTL;

  FrFTL* ftlInit(FlashReadCB rf, FlashProgramCB pf, FlashEraseCB ef, IsFlashErasedCB ief, uint8_t flashSizeInMB);
  void ftlDeInit(FrFTL *ftl);
  bool ftlWrite(FrFTL* ftl, uint32_t startSectorNo, uint32_t noOfSectors, const uint8_t* buf);
  bool ftlRead(FrFTL* ftl, uint32_t sectorNo, uint8_t* buffer);
  bool ftlTrim(FrFTL* ftl, uint32_t startSectorNo, uint32_t noOfSectors);
  bool ftlSync(FrFTL* ftl);

#ifdef __cplusplus
}
#endif

#endif // _FRFTL_H_
