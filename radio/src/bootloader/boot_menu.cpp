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
#include "firmware_files.h"

#include "hal/usb_driver.h"
#include "hal/rotary_encoder.h"
#include "os/time.h"

#include "lcd.h"

#define FRAME_INTERVAL_MS 20

#if defined(SPI_FLASH)
  #define MAIN_MENU_LEN 3
#else
  #define MAIN_MENU_LEN 2
#endif

#if defined(SPI_FLASH)
  #include "diskio_spi_flash.h"
  #define SEL_CLEAR_FLASH_STORAGE_MENU_LEN 2
#endif

void pollInputs()
{
  keysPollingCycle();

  // TODO: use rotaryEncoderPollingCycle() instead
#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
  static rotenc_t rePreviousValue;

  rotenc_t reNewValue = rotaryEncoderGetValue();
  int8_t scrollRE = reNewValue - rePreviousValue;
  if (scrollRE) {
    rePreviousValue = reNewValue;
    pushEvent(scrollRE < 0 ? EVT_ROTARY_LEFT : EVT_ROTARY_RIGHT);
  }
#endif
}

// poll inputs every 10ms
void per5ms()
{
  static uint32_t cnt = 0;
  if (++cnt & 1) { pollInputs(); }
}

FlashCheckRes valid;

int menuFlashFile(uint32_t index, event_t event)
{
  valid = checkFirmwareFile(index, valid);

  if (valid == FC_ERROR) {
    if (event == EVT_KEY_BREAK(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_ENTER))
      return 0;

    return -1;
  }

  if (event == EVT_KEY_LONG(KEY_ENTER)) {
    return openFirmwareFile(index) == FR_OK ? 1 : -1;
  } else if (event == EVT_KEY_BREAK(KEY_EXIT))
    return 0;

  return -1;
}

void bootloaderMenu()
{
  BootloaderState state = ST_START;
  uint32_t vpos = 0;
  uint32_t radioMenuItem = 0;
  uint8_t index = 0;
  FRESULT fr;
  uint32_t nameCount = 0;
  
  sdInit();

  uint32_t next_frame = time_get_ms();

  for (;;) {

    if (time_get_ms() - next_frame >= FRAME_INTERVAL_MS) {
      next_frame += FRAME_INTERVAL_MS;

      if (state != ST_USB && state != ST_FLASHING
          && state != ST_FLASH_DONE && state != ST_RADIO_MENU) {
        if (usbPlugged()) {
          state = ST_USB;
#if !defined(SIMU)
          usbStart();
#endif
        }
      }

      if (state == ST_USB) {
        if (usbPlugged() == 0) {
          vpos = 0;
#if !defined(SIMU)
          usbStop();
#endif
          state = ST_START;
        }
        bootloaderDrawScreen(state, 0);
      }

      lcdRefreshWait();
      event_t event = getEvent();

      if (state == ST_START) {

        bootloaderDrawScreen(state, vpos);

        if (IS_NEXT_EVENT(event)) {
          if (vpos < bootloaderGetMenuItemCount(MAIN_MENU_LEN) - 1) { vpos++; }
          continue;
        }
        else if (IS_PREVIOUS_EVENT(event)) {
          if (vpos > 0) { vpos--; }
          continue;
        }
        else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
          switch (vpos) {
            case 0:
              state = ST_DIR_CHECK;
              break;
#if defined(SPI_FLASH)
            case 1:
              state = ST_CLEAR_FLASH_CHECK;
              vpos = 0;
              break;
#endif
            default:
              if(vpos < bootloaderGetMenuItemCount(MAIN_MENU_LEN-1))
              {
                state = ST_RADIO_MENU;
                radioMenuItem = vpos - MAIN_MENU_LEN - 1;
              } else {
                state = ST_REBOOT;
              }
              break;
          }

          // next loop
          continue;
        }
      }
      else if (state == ST_DIR_CHECK) {
        nameCount = 0;
        fr = openFirmwareDir();
        if (fr == FR_OK)
          nameCount = fetchFirmwareFiles(index);

        if (nameCount > 0) {
          index = vpos = 0;
          state = ST_FILE_LIST;
          continue;
        } else {
          bootloaderDrawScreen(state, fr);

          if (event == EVT_KEY_BREAK(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_ENTER)) {
            vpos = 0;
            state = ST_START;
            continue;
          }
        }
      }

      if (state == ST_FILE_LIST) {
        uint32_t limit = MAX_NAMES_ON_SCREEN;
        if (nameCount < limit) {
          limit = nameCount;
        }

        if (IS_NEXT_EVENT(event)) {
          if (vpos < limit - 1) {
            vpos += 1;
          } else {
            if (nameCount > limit) {
              index += 1;
              nameCount = fetchFirmwareFiles(index);
            }
          }
        } else if (IS_PREVIOUS_EVENT(event)) {
          if (vpos > 0) {
            vpos -= 1;
          }
          else {
            if (index) {
              index -= 1;
              nameCount = fetchFirmwareFiles(index);
            }
          }
        }

        bootloaderDrawScreen(state, 0);

        for (uint32_t i = 0; i < limit; i++) {
          bootloaderDrawFilename(getFirmwareFileNameByIndex(i), i, (vpos == i));
        }

        if (event == EVT_KEY_BREAK(KEY_ENTER)) {
          // Select file to flash
          state = ST_FLASH_CHECK;
          valid = FC_UNCHECKED;
          continue;
        } else if (event == EVT_KEY_BREAK(KEY_EXIT)) {
          state = ST_START;
          vpos = 0;
          continue;
        }
      } else if (state == ST_FLASH_CHECK) {
        bootloaderDrawScreen(state, valid, getFirmwareFileNameByIndex(vpos));

        int result = menuFlashFile(vpos, event);
        if (result == 0) {
          // canceled
          state = ST_FILE_LIST;
        } else if (result == 1) {
          // confirmed
          firmwareInitWrite(vpos);
          state = ST_FLASHING;
        }
      } else if (state == ST_FLASHING) {
        uint32_t progress = 0;
        bool done = firmwareWriteBlock(&progress);
        bootloaderDrawScreen(state, progress);
        if(done) {
          state = ST_FLASH_DONE;
        }
#if defined(SPI_FLASH)
      } else if (state == ST_CLEAR_FLASH_CHECK) {
        bootloaderDrawScreen(state, vpos);
        if (event == EVT_KEY_LONG(KEY_ENTER))
        {
          state = ST_CLEAR_FLASH;
        } else if (event == EVT_KEY_BREAK(KEY_EXIT)) {
          vpos = 0;
          state = ST_START;
          continue;
        }
      } else if (state == ST_CLEAR_FLASH) {
        bootloaderDrawScreen(state, 0);
        lcdRefresh();
        sdDone();
        spiFlashDiskEraseAll();
        sdInit();
        vpos = 0;
        state = ST_START;
#endif
      } else if (state == ST_RADIO_MENU) {
        if(bootloaderRadioMenu(radioMenuItem, event))
        {
          state = ST_START;
          vpos = 0;
          bootloaderDrawScreen(state, vpos);
        }
      }

      if (state == ST_FLASH_DONE) {
        if (event == EVT_KEY_BREAK(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_ENTER)) {
          state = ST_START;
          vpos = 0;
        }

        bootloaderDrawScreen(state, 100);
      }

      if (event == EVT_KEY_LONG(KEY_EXIT)) {
        // Start the main application
        state = ST_REBOOT;
      }

      lcdRefresh();
    }

    if (state != ST_FLASHING && state != ST_USB) {
      if (pwrOffPressed()) {
        sdDone();
        boardOff();
      }
    }

    if (state == ST_REBOOT) {
      sdDone();
#if !defined(SIMU)
      blExit();
      NVIC_SystemReset();
#else
      exit(1);
#endif
    }
  }
}
