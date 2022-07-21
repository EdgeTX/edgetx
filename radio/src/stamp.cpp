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

#include "definitions.h"
#include "board.h"
#include "fw_version.h"

#include <string.h>

#define STR2(s) #s
#define DEFNUMSTR(s)  STR2(s)

#define TAB "\037\033"

#if defined(FRSKY_RELEASE)
#define DISPLAY_VERSION "-frsky"
#elif defined(JUMPER_RELEASE)
#define DISPLAY_VERSION "-jumper"
#elif defined(RADIOMASTER_RELEASE)
#define DISPLAY_VERSION "-RM"
#elif defined(TBS_RELEASE)
#define DISPLAY_VERSION "-tbs"
#elif defined(IMRC_RELEASE)
#define DISPLAY_VERSION "-imrc"
#else
#define DISPLAY_VERSION
#endif

#if defined(COLORLCD)
  const char fw_stamp[]     = "FW" TAB ": edgetx-" FLAVOUR;
  #if defined(RADIOMASTER_RELEASE) || defined(JUMPER_RELEASE)
    const char vers_stamp[]   = "VERS" TAB ": Factory firmware (" GIT_STR ")";
  #else
    #if defined(VERSION_TAG)
      const char vers_stamp[] = "VERS" TAB ": " VERSION_TAG DISPLAY_VERSION "\"" CODENAME "\"";
    #else
      const char vers_stamp[] = "VERS" TAB ": " VERSION "-" VERSION_SUFFIX DISPLAY_VERSION " (" GIT_STR ")";
    #endif
  #endif
  const char date_stamp[]   = "DATE" TAB ": " DATE;
  const char time_stamp[]   = "TIME" TAB ": " TIME;
#elif defined(BOARD_NAME)
  const char vers_stamp[]   = "FW" TAB ": edgetx-" BOARD_NAME "\036VERS" TAB ": " VERSION DISPLAY_VERSION " (" GIT_STR ")" "\036DATE" TAB ": " DATE " " TIME;
#elif defined(RADIOMASTER_RELEASE)
  const char vers_stamp[]   = "FW" TAB ": edgetx-" FLAVOUR    "\036VERS" TAB ": RM Factory (" GIT_STR ")" "\036BUILT BY : EdgeTX" "\036DATE" TAB ": " DATE " " TIME;
#elif defined(JUMPER_RELEASE)
  const char vers_stamp[]   = "FW" TAB ": edgetx-" FLAVOUR    "\036VERS" TAB ": Factory (" GIT_STR ")" "\036BUILT BY : EdgeTX" "\036DATE" TAB ": " DATE " " TIME;
#else
  #if defined(VERSION_TAG)
    const char vers_stamp[]   = "FW" TAB ": edgetx-" FLAVOUR    "\036VERS" TAB ": " VERSION_TAG DISPLAY_VERSION "\036NAME" ": " CODENAME "\036DATE" TAB ": " DATE " " TIME;
  #else
    const char vers_stamp[]   = "FW" TAB ": edgetx-" FLAVOUR    "\036VERS" TAB ": " VERSION "-" VERSION_SUFFIX DISPLAY_VERSION "\036GIT#" TAB ": " GIT_STR "\036DATE" TAB ": " DATE " " TIME;
  #endif
#endif

/**
 * Retrieves the version of the bootloader or firmware
 * @return
 */
#if defined(STM32) && !defined(SIMU)
  #if defined(COLORLCD)
    #if defined(VERSION_TAG)
__SECTION_USED(".fwversiondata")   const char firmware_version[] = "edgetx-" FLAVOUR "-" VERSION_TAG DISPLAY_VERSION " (" GIT_STR ")";
__SECTION_USED(".bootversiondata") const char boot_version[] =     "edgetx-" FLAVOUR "-" VERSION_TAG DISPLAY_VERSION " (" GIT_STR ")";
    #else
__SECTION_USED(".fwversiondata")   const char firmware_version[] = "edgetx-" FLAVOUR "-" VERSION "-" VERSION_SUFFIX DISPLAY_VERSION " (" GIT_STR ")";
__SECTION_USED(".bootversiondata") const char boot_version[] =     "edgetx-" FLAVOUR "-" VERSION "-" VERSION_SUFFIX DISPLAY_VERSION " (" GIT_STR ")";
    #endif
  #else
  /* 128x64 does not have enough real estate to display more than basic VERSION */
__SECTION_USED(".fwversiondata")   const char firmware_version[] = "edgetx-" FLAVOUR "-" VERSION DISPLAY_VERSION " (" GIT_STR ")";
__SECTION_USED(".bootversiondata") const char boot_version[] =     "edgetx-" FLAVOUR "-" VERSION DISPLAY_VERSION " (" GIT_STR ")";
  #endif

/**
 * Tries to find EdgeTX or OpenTX version in the first 1024 byte of either firmware/bootloader (the one not running) or the buffer
 * @param buffer If non-null find the firmware version in the buffer instead
 */
const char * getFirmwareVersion(const char * buffer)
{
  if (buffer == nullptr) {
#if defined(BOOT)
    buffer = (const char *)(FIRMWARE_ADDRESS + BOOTLOADER_SIZE);
#else
    buffer = (const char *)FIRMWARE_ADDRESS;
#endif
  }

  for (int i = 0; i < 1024; i++) {
    if ((memcmp(buffer + i, "edgetx-", 7) == 0)
        || memcmp(buffer + i, "opentx-", 7) == 0) {
      return buffer + i;
    }
  }

  return "no version found";
}
#else
const char * getFirmwareVersion(const char * buffer)
{
  return "edgetx-" FLAVOUR "-" VERSION DISPLAY_VERSION " (" GIT_STR ")";
}
#endif
