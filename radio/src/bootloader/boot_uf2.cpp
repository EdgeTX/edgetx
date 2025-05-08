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

#include "boot.h"

#include "hal/usb_driver.h"
#include "hal/storage.h"
#include "hal/fatfs_diskio.h"
#include "os/time.h"

#include "thirdparty/FatFs/diskio.h"
#include "drivers/uf2_ghostfat.h"

#include "board.h"
#include "lcd.h"

#define FRAME_INTERVAL_MS 20

extern uf2_fat_write_state_t _uf2_write_state;

void bootloaderUF2()
{
  BootloaderState state = ST_START;

  // register SD storage
  storageInit();
  disk_initialize(0);

  uint32_t next_frame = time_get_ms();

  for (;;) {

    if (time_get_ms() - next_frame >= FRAME_INTERVAL_MS) {
      next_frame += FRAME_INTERVAL_MS;

      keysPollingCycle();
      event_t event = getEvent();

      if (state != ST_FLASHING && event == EVT_KEY_LONG(KEY_EXIT)) {
        // Start the main application
        state = ST_REBOOT;
      }

      if (state == ST_REBOOT) {
        storageDeInit();
#if !defined(SIMU)
        blExit();
        NVIC_SystemReset();
#else
        exit(1);
#endif
      }

      if (state != ST_USB && state != ST_FLASHING && state != ST_FLASH_DONE) {
        if (usbPlugged()) {
          state = ST_USB;
#if !defined(SIMU)
          usbStart();
#endif
        } else if (pwrOffPressed()) {
          storageDeInit();
          boardOff();
        }
      }

      if (state == ST_USB || state == ST_FLASH_DONE) {
        if (usbPlugged() == 0) {
#if !defined(SIMU)
          usbStop();
#endif
          state = (state == ST_FLASH_DONE) ? ST_REBOOT : ST_START;
        } else {
          auto wr_st = uf2_fat_get_state();
          if (wr_st->num_blocks != 0 && wr_st->num_blocks <= UF2_MAX_BLOCKS) {
            state = ST_FLASHING;
          }
        }
        bootloaderDrawScreen(state, 0);
      }

      if (state == ST_START) {
        bootloaderDrawScreen(state, 0);
      } else if (state == ST_FLASHING) {
        auto wr_st = uf2_fat_get_state();
        if (wr_st->num_blocks == 0 || wr_st->num_blocks > UF2_MAX_BLOCKS) {
          state = ST_USB;
        } else {
          uint32_t progress = (wr_st->num_written * 100) / wr_st->num_blocks;
          if (wr_st->num_written == wr_st->num_blocks) {
            state = ST_FLASH_DONE;
            uf2_fat_reset_state();
          }
          bootloaderDrawScreen(state, progress);
        }
      } else if (state == ST_FLASH_DONE) {
        bootloaderDrawScreen(state, 100);
      }

      lcdRefresh();
    }
  }  
}
