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

#if defined(SIMU)
  #define WRITE_DELAY_10MS 100
#elif defined(RTC_BACKUP_RAM)
  #define WRITE_DELAY_10MS 1500 /* 15s */
#elif defined(PCBTARANIS)
  #define WRITE_DELAY_10MS 500
#else
  #define WRITE_DELAY_10MS 200
#endif

extern uint8_t   storageDirtyMsk;
extern tmr10ms_t storageDirtyTime10ms;
#define TIME_TO_WRITE()                (storageDirtyMsk && (tmr10ms_t)(get_tmr10ms() - storageDirtyTime10ms) >= (tmr10ms_t)WRITE_DELAY_10MS)

#if defined(RTC_BACKUP_RAM)
#include "storage/rtc_backup.h"
extern uint8_t   rambackupDirtyMsk;
extern tmr10ms_t rambackupDirtyTime10ms;
#define TIME_TO_BACKUP_RAM()            (rambackupDirtyMsk && (tmr10ms_t)(get_tmr10ms() - rambackupDirtyTime10ms) >= (tmr10ms_t)100)
#endif

//
// Generic storage interface
//
void storageEraseAll(bool warn);
void storageFormat();
void storageReadAll();
void storageCheck(bool immediately);

//
// Generic storage functions (implemented in storage_common.cpp)
//
void storageDirty(uint8_t msk);
void storageFlushCurrentModel();
void postRadioSettingsLoad();
void preModelLoad();
void postModelLoad(bool alarms);

#if !defined(STORAGE_MODELSLIST)
extern ModelHeader modelHeaders[MAX_MODELS];

void loadModelHeader(uint8_t id, ModelHeader *header);
void loadModelHeaders();

uint8_t findNextUnusedModelId(uint8_t index, uint8_t module);
#endif

#include "sdcard_common.h"

#if defined(RTC_BACKUP_RAM)
#include "rtc_backup.h"
#endif
