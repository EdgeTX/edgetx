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

#include "ff.h"

extern FIL g_oLogFile;

#include "translations.h"

#define FILE_COPY_PREFIX "cp_"

#define PATH_SEPARATOR      "/"
#define ROOT_PATH           PATH_SEPARATOR
#define MODELS_PATH         ROOT_PATH "MODELS"      // no trailing slash = important
#define DELETED_MODELS_PATH MODELS_PATH PATH_SEPARATOR "DELETED"
#define UNUSED_MODELS_PATH  MODELS_PATH PATH_SEPARATOR "UNUSED"
#define RADIO_PATH          ROOT_PATH "RADIO"       // no trailing slash = important
#define TEMPLATES_PATH      ROOT_PATH "TEMPLATES"
#define PERS_TEMPL_PATH     TEMPLATES_PATH "/2.Personal"
#define PERS_TEMPL_PATH_OLD TEMPLATES_PATH "/PERSONAL"
#define LOGS_PATH           ROOT_PATH "LOGS"
#define SCREENSHOTS_PATH    ROOT_PATH "SCREENSHOTS"
#define SOUNDS_PATH         ROOT_PATH "SOUNDS/en"
#define SOUNDS_PATH_LNG_OFS (sizeof(SOUNDS_PATH)-3)
#define SYSTEM_SUBDIR       "SYSTEM"
#define BITMAPS_PATH        ROOT_PATH "IMAGES"
#define FIRMWARES_PATH      ROOT_PATH "FIRMWARE"
#define AUTOUPDATE_FILENAME FIRMWARES_PATH PATH_SEPARATOR "autoupdate.frsk"
#define BACKUP_PATH         ROOT_PATH "BACKUP"
#define SCRIPTS_PATH        ROOT_PATH "SCRIPTS"
#define WIZARD_PATH         SCRIPTS_PATH PATH_SEPARATOR "WIZARD"
#define THEMES_PATH         ROOT_PATH "THEMES"
#define LAYOUTS_PATH        ROOT_PATH "LAYOUTS"
#define WIDGETS_PATH        ROOT_PATH "WIDGETS"
#define WIZARD_NAME         "wizard.lua"
#define SCRIPTS_MIXES_PATH  SCRIPTS_PATH PATH_SEPARATOR "MIXES"
#define SCRIPTS_FUNCS_PATH  SCRIPTS_PATH PATH_SEPARATOR "FUNCTIONS"
#define SCRIPTS_TELEM_PATH  SCRIPTS_PATH PATH_SEPARATOR "TELEMETRY"
#define SCRIPTS_TOOLS_PATH  SCRIPTS_PATH PATH_SEPARATOR "TOOLS"
#define SCRIPTS_RGB_PATH    SCRIPTS_PATH PATH_SEPARATOR "RGBLED"

#define LEN_FILE_PATH_MAX   (sizeof(SCRIPTS_TELEM_PATH)+1)  // longest + "/"

#define RADIO_FILENAME      "radio.bin"
const char RADIO_SETTINGS_PATH[] = RADIO_PATH PATH_SEPARATOR RADIO_FILENAME;
#define LABELS_FILENAME     "labels.yml"
#define MODELS_FILENAME     "models.yml"
const char MODELSLIST_YAML_PATH[] = MODELS_PATH PATH_SEPARATOR MODELS_FILENAME;
const char FALLBACK_MODELSLIST_YAML_PATH[] = RADIO_PATH PATH_SEPARATOR MODELS_FILENAME;
const char LABELSLIST_YAML_PATH[] = MODELS_PATH PATH_SEPARATOR LABELS_FILENAME;
const char RADIO_SETTINGS_YAML_PATH[] = RADIO_PATH PATH_SEPARATOR "radio.yml";
const char RADIO_SETTINGS_TMPFILE_YAML_PATH[] = RADIO_PATH PATH_SEPARATOR "radio_new.yml";
const char RADIO_SETTINGS_ERRORFILE_YAML_PATH[] = RADIO_PATH PATH_SEPARATOR "radio_error.yml";

const char YAMLFILE_CHECKSUM_TAG_NAME[] = "checksum";
#define    SPLASH_FILE             "splash.png"
#define    SHUTDOWN_SPLASH_FILE    "shutdown.png"

#define MODELS_EXT          ".bin"
#define LOGS_EXT            ".csv"
#define SOUNDS_EXT          ".wav"
#define BMP_EXT             ".bmp"
#define PNG_EXT             ".png"
#define JPG_EXT             ".jpg"
#define SCRIPT_EXT          ".lua"
#define SCRIPT_BIN_EXT      ".luac"
#define TEXT_EXT            ".txt"
#define FIRMWARE_EXT        ".bin"
#define SPORT_FIRMWARE_EXT  ".frk"
#define FRSKY_FIRMWARE_EXT  ".frsk"
#define MULTI_FIRMWARE_EXT  ".bin"
#define ELRS_FIRMWARE_EXT   ".elrs"
#define YAML_EXT            ".yml"

#if defined(COLORLCD)
#define BITMAPS_EXT         BMP_EXT JPG_EXT PNG_EXT
#define LEN_BITMAPS_EXT     4
#else
#define BITMAPS_EXT         BMP_EXT
#endif

#ifdef LUA_COMPILER
  #define SCRIPTS_EXT         SCRIPT_BIN_EXT SCRIPT_EXT
#else
  #define SCRIPTS_EXT         SCRIPT_EXT
#endif

#define GET_FILENAME(filename, path, var, ext) \
  char filename[sizeof(path) + sizeof(var) + sizeof(ext)]; \
  memcpy(filename, path, sizeof(path) - 1); \
  filename[sizeof(path) - 1] = '/'; \
  memcpy(&filename[sizeof(path)], var, sizeof(var)); \
  filename[sizeof(path)+sizeof(var)] = '\0'; \
  strcat(&filename[sizeof(path)], ext)

extern uint8_t logDelay100ms;
void logsInit();
void logsClose();
void logsWrite();

void sdInit();
void sdMount();
void sdDone();
uint32_t sdMounted();

uint32_t sdGetNoSectors();
uint32_t sdGetSize();
uint32_t sdGetFreeSectors();
uint32_t sdGetFreeKB();
bool sdIsFull();

const char * sdCheckAndCreateDirectory(const char * path);

#if !defined(BOOT)
inline const char * SDCARD_ERROR(FRESULT result)
{
  if (result == FR_NOT_READY)
    return STR_NO_SDCARD;
  else
    return STR_SDCARD_ERROR;
}
#endif

const char * getBasename(const char * path);

bool isFileAvailable(const char * filename, bool exclDir = false);
unsigned int findNextFileIndex(char * filename, uint8_t size, const char * directory);

const char * sdCopyFile(const char * src, const char * dest);
const char * sdCopyFile(const char * srcFilename, const char * srcDir, const char * destFilename, const char * destDir);
const char * sdMoveFile(const char * src, const char * dest);
const char * sdMoveFile(const char * srcFilename, const char * srcDir, const char * destFilename, const char * destDir);

#define LIST_NONE_SD_FILE   1
#define LIST_SD_FILE_EXT    2
bool sdListFiles(const char * path, const char * extension, const uint8_t maxlen, const char * selection, uint8_t flags=0);
