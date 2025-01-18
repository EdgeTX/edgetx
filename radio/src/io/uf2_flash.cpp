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

#include "uf2_flash.h"

#include <stdio.h>
#include <string.h>
#include <algorithm>

#include "thirdparty/FatFs/ff.h"
#include "thirdparty/uf2/uf2.h"

#include "hal/watchdog_driver.h"
#include "hal/flash_driver.h"

#include "pulses/pulses.h"

#define UF2_BLOCK_SIZE 512

void UF2FirmwareUpdate::flashFirmware(const char* filename,
                                      ProgressHandler progressHandler)
{
#if !defined(SIMU)
  FIL file;
  UF2_Block block;
  bool success = false;

  pulsesStop();
  if (f_open(&file, filename, FA_READ) != FR_OK) {
    progressHandler("Firmware", STR_FIRMWARE_UPDATE_ERROR, 1, 1);
    pulsesStart();
    return;
  }

  size_t total_len = f_size(&file);
  size_t erased_len = 0;
  uint32_t last_erased = 0;

  UINT bytes_read;
  while (f_read(&file, &block, sizeof(block), &bytes_read) == FR_OK &&
         bytes_read > 0) {

    uint32_t addr = block.targetAddr;
    if ((block.flags & UF2_FLAG_NOFLASH) == 0 && addr > last_erased) {
      auto drv = flashFindDriver(addr);
      if (drv) {
        uint32_t sector = drv->get_sector(addr);
        uint32_t sect_len = drv->get_sector_size(sector);
        if (drv->erase_sector(addr) < 0) break;
        last_erased = block.targetAddr + sect_len - 1;
      }
    }

    erased_len += bytes_read;
    progressHandler("Firmware", "Erasing...", erased_len, total_len);

    if (erased_len >= total_len) {
      success = true;
      break;
    }
  }

  if (success) {
    success = false;
    size_t written_len = 0;
    f_rewind(&file);

    while (f_read(&file, &block, sizeof(block), &bytes_read) == FR_OK &&
           bytes_read > 0) {

      if ((block.flags & UF2_FLAG_NOFLASH) == 0) {
        uint32_t addr = block.targetAddr;
        auto drv = flashFindDriver(addr);
        if (drv) {
          uint32_t len = block.payloadSize;
          uint8_t* data = (uint8_t*)block.data;
          drv->program(addr, data, len);
        }
      }

      written_len += bytes_read;
      progressHandler("Firmware", "Writing...", written_len, total_len);

      if (written_len >= total_len) {
        success = true;
        break;
      }
    }
  }

  if(success)
    POPUP_INFORMATION(STR_FIRMWARE_UPDATE_SUCCESS);
  else
    POPUP_INFORMATION(STR_FIRMWARE_UPDATE_ERROR);

  watchdogSuspend(0);
  WDG_RESET();

  f_close(&file);
  pulsesStart();
#endif
}
