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
extern FIL g_oLogFile;

#include "translations.h"

#define FILE_COPY_PREFIX "cp_"

#define PATH_SEPARATOR      "/"
#define ROOT_PATH           PATH_SEPARATOR
#define MODELS_PATH         ROOT_PATH "MODELS"      // no trailing slash = important
#define RADIO_PATH          ROOT_PATH "RADIO"       // no trailing slash = important
#define TEMPLATES_PATH      ROOT_PATH "TEMPLATES"
#define PERS_TEMPL_PATH     TEMPLATES_PATH "/PERSONAL"
#define LOGS_PATH           ROOT_PATH "LOGS"
#define SCREENSHOTS_PATH    ROOT_PATH "SCREENSHOTS"
#define SOUNDS_PATH         ROOT_PATH "SOUNDS/en"
#define SOUNDS_PATH_LNG_OFS (sizeof(SOUNDS_PATH)-3)
#define SYSTEM_SUBDIR       "SYSTEM"
#define BITMAPS_PATH        ROOT_PATH "IMAGES"
#define FIRMWARES_PATH      ROOT_PATH "FIRMWARE"
#define AUTOUPDATE_FILENAME FIRMWARES_PATH PATH_SEPARATOR "autoupdate.frsk"
#define EEPROMS_PATH        ROOT_PATH "EEPROM"
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

#define LEN_FILE_PATH_MAX   (sizeof(SCRIPTS_TELEM_PATH)+1)  // longest + "/"

#if defined(SDCARD_YAML) || defined(SDCARD_RAW)
#define RADIO_FILENAME      "radio.bin"
const char RADIO_MODELSLIST_PATH[] = RADIO_PATH PATH_SEPARATOR "models.txt";
const char RADIO_SETTINGS_PATH[] = RADIO_PATH PATH_SEPARATOR RADIO_FILENAME;
#if defined(SDCARD_YAML)
const char MODELSLIST_YAML_PATH[] = MODELS_PATH PATH_SEPARATOR "models.yml";
const char FALLBACK_MODELSLIST_YAML_PATH[] = RADIO_PATH PATH_SEPARATOR "models.yml";
const char RADIO_SETTINGS_YAML_PATH[] = RADIO_PATH PATH_SEPARATOR "radio.yml";
#endif
#define    SPLASH_FILE             "splash.png"
#endif

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
#define EEPROM_EXT          ".bin"
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

bool sdCardFormat();
uint32_t sdGetNoSectors();
uint32_t sdGetSize();
uint32_t sdGetFreeSectors();
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

// NOTE: 'size' must = 0 or be a valid character position within 'filename' array -- it is NOT validated
const char * getBasename(const char * path);

#if defined(PCBX12S)
  #define ETX_FOURCC 0x3478746F // etx for X12S
#elif defined(RADIO_T16)
  #define ETX_FOURCC 0x3F78746F // etx for Jumper T16
#elif defined(RADIO_T18)
  #define ETX_FOURCC 0x4078746F // etx for Jumper T18
#elif defined(RADIO_TX16S)
  #define ETX_FOURCC 0x3878746F // etx for Radiomaster TX16S
#elif defined(PCBX10)
  #define ETX_FOURCC 0x3778746F // etx for X10
#elif defined(PCBX9E)
  #define ETX_FOURCC 0x3578746F // etx for Taranis X9E
#elif defined(PCBXLITES)
  #define ETX_FOURCC 0x3B78746F // etx for Taranis X-Lite S
#elif defined(PCBXLITE)
  #define ETX_FOURCC 0x3978746F // etx for Taranis X-Lite
#elif defined(RADIO_T12)
  #define ETX_FOURCC 0x3D78746F // etx for Jumper T12
#elif defined(RADIO_TLITE)
  #define ETX_FOURCC 0x4278746F // etx for Jumper TLite
#elif defined(RADIO_TPRO)
  #define ETX_FOURCC 0x4678746F // etx for Jumper TPro
#elif defined(RADIO_TX12)
  #define ETX_FOURCC 0x4178746F // etx for Radiomaster TX12
#elif defined(RADIO_TX12MK2)
  #define ETX_FOURCC 0x4878746F // etx for Radiomaster TX12MK2
#elif defined(RADIO_ZORRO)
  #define ETX_FOURCC 0x4778746F // otx for Radiomaster Zorro
#elif defined(RADIO_T8)
  #define ETX_FOURCC 0x4378746F // etx for Radiomaster T8
#elif defined(PCBX7)
  #define ETX_FOURCC 0x3678746F // etx for Taranis X7 / X7S / X7 Express / X7S Express
#elif defined(PCBX9LITES)
  #define ETX_FOURCC 0x3E78746F // etx for Taranis X9-Lite S
#elif defined(PCBX9LITE)
  #define ETX_FOURCC 0x3C78746F // etx for Taranis X9-Lite
#elif defined(PCBX9D) || defined(PCBX9DP)
  #define ETX_FOURCC 0x3378746F // etx for Taranis X9D
#elif defined(PCBNV14)
  #define ETX_FOURCC 0x3A78746F // etx for NV14
#endif

bool isFileAvailable(const char * filename, bool exclDir = false);
unsigned int findNextFileIndex(char * filename, uint8_t size, const char * directory);

const char * sdCopyFile(const char * src, const char * dest);
const char * sdCopyFile(const char * srcFilename, const char * srcDir, const char * destFilename, const char * destDir);

#define LIST_NONE_SD_FILE   1
#define LIST_SD_FILE_EXT    2
bool sdListFiles(const char * path, const char * extension, const uint8_t maxlen, const char * selection, uint8_t flags=0);

#endif // _SDCARD_H_
