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

#include "stm32_hal.h"

#include "boot.h"
#include "hal/usb_driver.h"
#include "hal/watchdog_driver.h"
#include "os/time.h"

#include "lcd.h"

#define FRAME_INTERVAL_MS 20

#if defined(BOOTLOADER_DFU)
#include "hal/abnormal_reboot.h"
#include "board.h"

volatile bool bootloaderDFULeaveRequested = false;

// board hook from boot.cpp (sets up what the firmware expects)
void boardBLPreJump();
#endif

// DFU is entered from the firmware, whose watchdog keeps running (it cannot
// be stopped): re-arm it longer than a 128 KB F4 sector erase (up to 2 s
// with interrupts off) and feed it from the main loop.
#define DFU_WATCHDOG_MS 4000

void bootloaderDFU()
{
  WDG_ENABLE(DFU_WATCHDOG_MS);

  usbPlugged();
  usbStart();

  bootloaderDrawDFUScreen();

  uint32_t next_frame = time_get_ms();
  for (;;) {
    if (time_get_ms() - next_frame >= FRAME_INTERVAL_MS) {
      next_frame += FRAME_INTERVAL_MS;
      WDG_RESET();

      if (!usbPlugged()) break;
#if defined(BOOTLOADER_DFU)
      if (bootloaderDFULeaveRequested) {
        // a reset would start the ST ROM bootloader with USB attached
        blExit();
        abnormalRebootRequestPowerOn();
        abnormalRebootQuiesce();
        boardBLPreJump();
        abnormalRebootJump(APP_START_ADDRESS);
      }
#endif
      bootloaderDrawDFUScreen();
      lcdRefresh();
    }
  }

  blExit();
  NVIC_SystemReset();
}
