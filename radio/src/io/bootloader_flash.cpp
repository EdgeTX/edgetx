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
#include "edgetx.h"

#include "bootloader_flash.h"
#include "timers_driver.h"
#include "flash_driver.h"

#include "hal/watchdog_driver.h"
#include "os/sleep.h"

#if !defined(LIBOPENUI)
  #include "lib_file.h"
#endif

#ifndef BOOTLOADER_ADDRESS
#define BOOTLOADER_ADDRESS FIRMWARE_ADDRESS
#endif

bool isBootloader(const char * filename)
{
  FIL file;
  f_open(&file, filename, FA_READ);
  uint8_t buffer[1024];
  UINT count;

  if (f_read(&file, buffer, sizeof(buffer), &count) != FR_OK || count != sizeof(buffer)) {
    return false;
  }

  // Check firmware is for this radio
  for (int i = 0; i < 1024; i++) {
    if (memcmp(buffer + i, FLAVOUR, sizeof(FLAVOUR) - 1) == 0) {
      if (buffer[i + sizeof(FLAVOUR) - 1] == '-')
        return isBootloaderStart(buffer);;
      return false;
    }
  }
  return false;
}

void BootloaderFirmwareUpdate::flashFirmware(const char * filename, ProgressHandler progressHandler)
{
  FIL file;
  uint8_t buffer[1024];
  UINT count;

  pulsesStop();

  f_open(&file, filename, FA_READ);

  UINT flash_size = file.obj.objsize;
  if (flash_size > BOOTLOADER_SIZE) {
    flash_size = BOOTLOADER_SIZE;
  }
  
  for (int i = 0; i < BOOTLOADER_SIZE; i += 1024) {

    watchdogSuspend(1000/*10s*/);
    memset(buffer, 0xFF, sizeof(buffer));

    if (f_read(&file, buffer, sizeof(buffer), &count) != FR_OK) {
      POPUP_WARNING(STR_SDCARD_ERROR);
      break;
    }
    if (count != sizeof(buffer)
        && !f_eof(&file)) {
      POPUP_WARNING(STR_SDCARD_ERROR);
      break;
    }
    if (i == 0 && !isBootloaderStart(buffer)) {
      POPUP_WARNING(STR_INCOMPATIBLE);
      break;
    }
    for (UINT j = 0; j < count; j += FLASH_PAGESIZE) {
      WDG_ENABLE(3000);
      flashWrite(CONVERT_UINT_PTR(BOOTLOADER_ADDRESS + i + j), CONVERT_UINT_PTR(buffer + j));
      WDG_ENABLE(WDG_DURATION);
    }
    progressHandler("Bootloader", STR_WRITING, i, flash_size);

    // Reached end-of-file
    if (f_eof(&file)) break;

#if defined(SIMU)
    sleep_ms(30);
#endif
  }

  POPUP_INFORMATION(STR_FIRMWARE_UPDATE_SUCCESS);

  watchdogSuspend(0);
  WDG_RESET();

  f_close(&file);
  pulsesStart();
}
