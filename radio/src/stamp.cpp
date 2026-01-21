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
#define FACTORY_RELEASE "-frsky"
#elif defined(JUMPER_RELEASE)
#define FACTORY_RELEASE "-jumper"
#elif defined(RADIOMASTER_RELEASE)
#define FACTORY_RELEASE "-RM"
#elif defined(IFLIGHT_RELEASE)
#define FACTORY_RELEASE "-IF"
#elif defined(TBS_RELEASE)
#define FACTORY_RELEASE "-tbs"
#elif defined(IMRC_RELEASE)
#define FACTORY_RELEASE "-imrc"
#elif defined(BETAFPV_RELEASE)
#define FACTORY_RELEASE "-betafpv"
#endif

// If factory release, append mfg tag to version
#ifdef FACTORY_RELEASE
#define WITH_FACTORY_RELEASE(base) base FACTORY_RELEASE
#else
#define WITH_FACTORY_RELEASE(base) base
#endif

#if defined(COLORLCD)
  const char fw_stamp[]     = "FW" TAB ": edgetx-" FLAVOUR;
  #if defined(VERSION_TAG) // tagged release, possibly mfg build
    const char vers_stamp[] = "VERS" TAB ": " WITH_FACTORY_RELEASE(VERSION_TAG) " \"" CODENAME "\"";
  #elif defined(FACTORY_RELEASE) // mfg build of non-tagged release, show commit hash only
    const char vers_stamp[]   = "VERS" TAB ": Factory firmware (" GIT_STR ")";
  #else // any other build (e.g. self, cloud, PR, nightly) will have prefix/suffix
    const char vers_stamp[] = "VERS" TAB ": " VERSION_PREFIX VERSION VERSION_SUFFIX " (" GIT_STR ")";
  #endif
  const char date_stamp[]   = "DATE" TAB ": " DATE;
  const char time_stamp[]   = "TIME" TAB ": " TIME;
#else // B&W / !COLOR_LCD
  #if defined(VERSION_TAG)
    const char vers_stamp[] = "FW" TAB ": edgetx-" FLAVOUR "\036VERS" TAB ": " WITH_FACTORY_RELEASE(VERSION_TAG) "\036NAME" ": " CODENAME "\036DATE" TAB ": " DATE " " TIME;
  #elif defined(FACTORY_RELEASE)
    const char vers_stamp[] = "FW" TAB ": edgetx-" FLAVOUR "\036VERS" TAB ": Factory (" GIT_STR ")" "\036BUILT BY : EdgeTX" "\036DATE" TAB ": " DATE " " TIME;
  #else
    const char vers_stamp[] = "FW" TAB ": edgetx-" FLAVOUR "\036VERS" TAB ": " VERSION_PREFIX VERSION VERSION_SUFFIX "\036GIT#" TAB ": " GIT_STR "\036DATE" TAB ": " DATE " " TIME;
  #endif
#endif

/**
 * Retrieves the version of the bootloader or firmware
 * @return
 */
#if defined(STM32) && !defined(SIMU)
  #if defined(COLORLCD)
    #if defined(VERSION_TAG)
__SECTION_USED(".fwversiondata")   const char firmware_version[] = "edgetx-" FLAVOUR "-" WITH_FACTORY_RELEASE(VERSION_TAG) " (" GIT_STR ")";
__SECTION_USED(".bootversiondata") const char boot_version[] =     "edgetx-" FLAVOUR "-" WITH_FACTORY_RELEASE(VERSION_TAG) " (" GIT_STR ")";
    #else
__SECTION_USED(".fwversiondata")   const char firmware_version[] = "edgetx-" FLAVOUR "-" VERSION_PREFIX WITH_FACTORY_RELEASE(VERSION) VERSION_SUFFIX " (" GIT_STR ")";
__SECTION_USED(".bootversiondata") const char boot_version[] =     "edgetx-" FLAVOUR "-" VERSION_PREFIX WITH_FACTORY_RELEASE(VERSION) VERSION_SUFFIX " (" GIT_STR ")";
    #endif
  #else // B&W / !COLOR_LCD
  /* 128x64 does not have enough real estate to display more than basic VERSION */
__SECTION_USED(".fwversiondata")   const char firmware_version[] = "edgetx-" FLAVOUR "-" VERSION_PREFIX WITH_FACTORY_RELEASE(VERSION) " (" GIT_STR ")";
__SECTION_USED(".bootversiondata") const char boot_version[] =     "edgetx-" FLAVOUR "-" VERSION_PREFIX WITH_FACTORY_RELEASE(VERSION) " (" GIT_STR ")";
  #endif

/**
 * Tries to find EdgeTX or OpenTX version in the first 1024 byte of either firmware/bootloader (the one not running) or the buffer
 * @param buffer If non-null find the firmware version in the buffer instead
 */
const char * getFirmwareVersion(const uint8_t* buffer)
{
  if(buffer == nullptr) {
#if defined(BOOT) && !defined(FIRMWARE_QSPI)
    buffer = (const uint8_t *)(FIRMWARE_ADDRESS + BOOTLOADER_SIZE);
#else
    buffer = (const uint8_t *)FIRMWARE_ADDRESS;
#endif
  }

  for (int i = 0; i < 1024; i++) {
    if ((memcmp(buffer + i, "edgetx-", 7) == 0)
        || memcmp(buffer + i, "opentx-", 7) == 0) {
      return (const char*)buffer + i;
    }
  }
  return nullptr;
}
#endif
