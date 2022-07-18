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

#ifndef _SDCARD_RAW_H_
#define _SDCARD_RAW_H_

#include "FatFs/ff.h"

#if defined(SDCARD_RAW)

#define writeModel           writeModelBin
#define loadRadioSettings    loadRadioSettingsBin
#define writeGeneralSettings writeGeneralSettingsBin

const char* readModel(const char* filename, uint8_t* buffer, uint32_t size);

#endif

const char* loadRadioSettingsBin();
const char * writeGeneralSettingsBin();

const char* openFileBin(const char* fullpath, VfsFile& file, uint16_t& size,
                        uint8_t* version);

const char* readModelBin(const char* filename, uint8_t* buffer, uint32_t size,
                         uint8_t* version);

const char* writeModelBin();

#endif
