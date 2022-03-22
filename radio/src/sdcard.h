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

#ifndef _SDCARD_H_
#define _SDCARD_H_

#include "ff.h"

extern FATFS g_FATFS_Obj;

#include "translations.h"

#define DELETED_MODELS_PATH MODELS_PATH PATH_SEPARATOR "DELETED"
#define UNUSED_MODELS_PATH  MODELS_PATH PATH_SEPARATOR "UNUSED"
#define LABELS_FILENAME     "labels.yml"
#define MODELS_FILENAME     "models.yml"
const char MODELSLIST_YAML_PATH[] = MODELS_PATH PATH_SEPARATOR "models.yml";
const char FALLBACK_MODELSLIST_YAML_PATH[] = RADIO_PATH PATH_SEPARATOR "models.yml";
const char LABELSLIST_YAML_PATH[] = MODELS_PATH PATH_SEPARATOR LABELS_FILENAME;
const char RADIO_SETTINGS_TMPFILE_YAML_PATH[] = RADIO_PATH PATH_SEPARATOR "radio_new.yml";
const char RADIO_SETTINGS_ERRORFILE_YAML_PATH[] = RADIO_PATH PATH_SEPARATOR "radio_error.yml";

const char YAMLFILE_CHECKSUM_TAG_NAME[] = "checksum";
bool sdCardFormat();
uint32_t sdGetNoSectors();
uint32_t sdGetSize();
uint32_t sdGetFreeSectors();

#elif defined(RADIO_LR3PRO)
  #define ETX_FOURCC 0x4478746F // etx for BETAFPV LR3PRO
const char * sdMoveFile(const char * src, const char * dest);
const char * sdMoveFile(const char * srcFilename, const char * srcDir, const char * destFilename, const char * destDir);
#endif // _SDCARD_H_
