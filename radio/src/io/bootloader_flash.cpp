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

#include <stdio.h>
#include "opentx.h"
#include "bootloader_flash.h"

#if defined(LIBOPENUI)
  #include "libopenui.h"
#else
  #include "libopenui/src/libopenui_file.h"
#endif

bool isBootloader(const char * filename)
{
  VfsFile file;
  VirtualFS::instance().openFile(file, filename, VfsOpenFlags::READ);
  uint8_t buffer[1024];
  size_t count;

  if (file.read(buffer, sizeof(buffer), count) != VfsError::OK || count != sizeof(buffer)) {
    return false;
  }

  return isBootloaderStart(buffer);
}

void BootloaderFirmwareUpdate::flashFirmware(const char * filename, ProgressHandler progressHandler)
{
  VfsFile file;
  uint8_t buffer[1024];
  size_t count;

  pausePulses();

  VirtualFS::instance().openFile(file, filename, VfsOpenFlags::READ);

  static uint8_t unlocked = 0;
  if (!unlocked) {
    unlocked = 1;
    unlockFlash();
  }

  size_t flash_size = file.size();
  if (flash_size > BOOTLOADER_SIZE) {
    flash_size = BOOTLOADER_SIZE;
  }
  
  for (int i = 0; i < BOOTLOADER_SIZE; i += 1024) {

    watchdogSuspend(1000/*10s*/);
    memset(buffer, 0xFF, sizeof(buffer));

    if (file.read(buffer, sizeof(buffer), count) != VfsError::OK) {
      POPUP_WARNING(STR_SDCARD_ERROR);
      break;
    }
    if (count != sizeof(buffer)
        && !file.eof()) {
      POPUP_WARNING(STR_SDCARD_ERROR);
      break;
    }
    if (i == 0 && !isBootloaderStart(buffer)) {
      POPUP_WARNING(STR_INCOMPATIBLE);
      break;
    }
    for (size_t j = 0; j < count; j += FLASH_PAGESIZE) {
      flashWrite(CONVERT_UINT_PTR(FIRMWARE_ADDRESS + i + j), CONVERT_UINT_PTR(buffer + j));
    }
    progressHandler("Bootloader", STR_WRITING, i, flash_size);

    // Reached end-of-file
    if (file.eof()) break;

#if defined(SIMU)
    // add an artificial delay and check for simu quit
    if (SIMU_SLEEP_OR_EXIT_MS(30))
      break;
#endif
  }

  POPUP_INFORMATION(STR_FIRMWARE_UPDATE_SUCCESS);

  watchdogSuspend(0);
  WDG_RESET();

  if (unlocked) {
    lockFlash();
    unlocked = 0;
  }

  file.close();

  resumePulses();
}
