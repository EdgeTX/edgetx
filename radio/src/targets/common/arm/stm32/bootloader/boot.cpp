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

#include "hal/usb_driver.h"
#include "hal/abnormal_reboot.h"
#include "hal/rotary_encoder.h"

#include "board.h"
#include "boot.h"
#include "bin_files.h"
#include "lcd.h"
#include "debug.h"

#include "timers_driver.h"
#include "flash_driver.h"

#if defined(BLUETOOTH)
  #include "bluetooth_driver.h"
  #include "stm32_serial_driver.h"
#endif

#if defined(DEBUG_SEGGER_RTT)
  #include "thirdparty/Segger_RTT/RTT/SEGGER_RTT.h"
#endif

#if defined(SPI_FLASH)
  #define MAIN_MENU_LEN 3
#else
  #define MAIN_MENU_LEN 2
#endif

#if defined(SPI_FLASH)
  #include "diskio_spi_flash.h"
  #define SEL_CLEAR_FLASH_STORAGE_MENU_LEN 2
#endif

#define REBOOT_CMD_DFU 0x55464442

#if !defined(SIMU)
// Bootloader marker:
// -> used to detect valid bootloader files
const uint8_t bootloaderVersion[] __attribute__ ((section(".version"), used)) =
  {'B', 'O', 'O', 'T', '1', '0'};
#endif

#if defined(SIMU)
  #define __weak
#elif !defined(__weak)
  #define __weak __attribute__((weak))
#endif

volatile tmr10ms_t g_tmr10ms;
volatile uint8_t tenms = 1;

uint32_t firmwareSize;
uint32_t firmwareAddress = FIRMWARE_ADDRESS;
uint32_t firmwareWritten = 0;

FlashCheckRes valid;
MemoryType memoryType;
uint32_t unlocked = 0;

void per5ms() {} // make linker happy

void per10ms()
{
  tenms |= 1u; // 10 mS has passed
  g_tmr10ms++;

  keysPollingCycle();

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

uint32_t isValidBufferStart(const uint8_t * buffer)
{
#if !defined(SIMU)
  return isFirmwareStart(buffer);
#else
  return 1;
#endif
}

FlashCheckRes checkFlashFile(unsigned int index, FlashCheckRes res)
{
  if (res != FC_UNCHECKED)
    return res;

  if (openBinFile(memoryType, index) != FR_OK)
    return FC_ERROR;

  if (closeBinFile() != FR_OK)
    return FC_ERROR;

  if (!isValidBufferStart(Block_buffer))
    return FC_ERROR;

  return FC_OK;
}

int menuFlashFile(uint32_t index, event_t event)
{
  valid = checkFlashFile(index, valid);

  if (valid == FC_ERROR) {
    if (event == EVT_KEY_BREAK(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_ENTER))
      return 0;

    return -1;
  }

  if (event == EVT_KEY_LONG(KEY_ENTER)) {

    return (openBinFile(memoryType, index) == FR_OK) && isValidBufferStart(Block_buffer);
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT))
    return 0;

  return -1;
}

void flashWriteBlock()
{
  // TODO: use some board provided driver instead
  uint32_t blockOffset = 0;
#if !defined(SIMU)
  while (BlockCount) {
    flashWrite((uint32_t *)firmwareAddress, (uint32_t *)&Block_buffer[blockOffset]);
    blockOffset += FLASH_PAGESIZE;
    firmwareAddress += FLASH_PAGESIZE;
    if (BlockCount > FLASH_PAGESIZE) {
      BlockCount -= FLASH_PAGESIZE;
    }
    else {
      BlockCount = 0;
    }
  }
#endif // SIMU
}

#if !defined(SIMU)

typedef void (*fctptr_t)(void);

static __attribute__((noreturn)) void jumpTo(uint32_t addr)
{
  __disable_irq();
  __set_MSP(*(uint32_t*)addr);
  fctptr_t reset_handler = (fctptr_t)*(uint32_t*)(addr + 4);
  reset_handler();
  while(1){}    
}

// Optional board hook
__weak void boardBLEarlyInit() {}
__weak bool boardBLStartCondition() { return false; }
__weak void boardBLPreJump() {}
__weak void boardBLInit() {}

void bootloaderInitApp()
{
  boardBLEarlyInit();

#if defined(DEBUG_SEGGER_RTT)
  SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
#endif

  pwrInit();
  keysInit();
  delaysInit();

  bool boot_dfu = abnormalRebootGetCmd() == REBOOT_CMD_DFU; 
  if (!boot_dfu) {
    bool start_firmware = true;

    // wait a bit for the inputs to stabilize...
    if (!WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) {
      delay_us(200);
    }

    start_firmware = !boardBLStartCondition(); 
    if (start_firmware) {
      // Start main application
      boardBLPreJump();
      jumpTo(APP_START_ADDRESS);
    }
  } else {
     setSelectedUsbMode(USB_DFU_MODE);
  }

  // TODO: move all this into board specifics
  pwrOn();

#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
  rotaryEncoderInit();
#endif

#if defined(DEBUG)
  initSerialPorts();
#endif

  __enable_irq();

  TRACE("\nBootloader started :)");

  // TODO: move BT into board specifics
#if defined(BLUETOOTH)
  // we shutdown the bluetooth module now to be sure it will be detected on
  // firmware start
  bluetoothInit(BLUETOOTH_DEFAULT_BAUDRATE, false);
#endif

  timersInit();

  // SD card detect pin
  sdInit();
  usbInit();
  boardBLInit();
}

int main()
#else // SIMU
void bootloaderInitApp() {}
int  bootloaderMain()
#endif
{
  BootloaderState state = ST_START;
  uint32_t vpos = 0;
  uint32_t radioMenuItem = 0;
  uint8_t index = 0;
  FRESULT fr;
  uint32_t nameCount = 0;

  // init hardware (may jump to app)
  bootloaderInitApp();

  // init screen
  bootloaderInitScreen();

#if defined(PWR_BUTTON_PRESS)
  // wait until power button is released
  while (pwrPressed()) {}
#endif

  for (;;) {

    if (tenms) {
      tenms = 0;

      if (state != ST_USB && state != ST_FLASHING
          && state != ST_FLASH_DONE && state != ST_RADIO_MENU) {
        if (usbPlugged()) {
          state = ST_USB;
          if (!unlocked) {
            unlocked = 1;
            unlockFlash();
          }
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
          if (unlocked) {
            lockFlash();
            unlocked = 0;
          }
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
              memoryType = MEM_FLASH;
              state = ST_DIR_CHECK;
              break;
#if defined(SPI_FLASH)
            case 1:
              state = ST_CLEAR_FLASH_CHECK;
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
        fr = openBinDir(memoryType);

        if (fr == FR_OK) {
          index = vpos = 0;
          state = ST_FILE_LIST;
          nameCount = fetchBinFiles(index);
          continue;
        }
        else {
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
          }
          else {
            if (nameCount > limit) {
              index += 1;
              nameCount = fetchBinFiles(index);
            }
          }
        }
        else if (IS_PREVIOUS_EVENT(event)) {
          if (vpos > 0) {
            vpos -= 1;
          }
          else {
            if (index) {
              index -= 1;
              nameCount = fetchBinFiles(index);
            }
          }
        }

        bootloaderDrawScreen(state, 0);

        for (uint32_t i = 0; i < limit; i++) {
          bootloaderDrawFilename(binFiles[i].name, i, (vpos == i));
        }

        if (event == EVT_KEY_BREAK(KEY_ENTER)) {
          // Select file to flash
          state = ST_FLASH_CHECK;
          valid = FC_UNCHECKED;
          continue;
        }
        else if (event == EVT_KEY_BREAK(KEY_EXIT)) {
          state = ST_START;
          vpos = 0;
          continue;
        }
      }
      else if (state == ST_FLASH_CHECK) {
        bootloaderDrawScreen(state, valid, binFiles[vpos].name);

        int result = menuFlashFile(vpos, event);
        if (result == 0) {
          // canceled
          state = ST_FILE_LIST;
        }
        else if (result == 1) {
          // confirmed

          if (memoryType == MEM_FLASH) {
            firmwareSize = FIRMWARE_LEN(binFiles[vpos].size);
            firmwareAddress = APP_START_ADDRESS;
            firmwareWritten = 0;
          }
          state = ST_FLASHING;
        }
      }
      else if (state == ST_FLASHING) {
        // commit to flashing
        if (!unlocked && (memoryType == MEM_FLASH)) {
          unlocked = 1;
          unlockFlash();
        }

        int progress = 0;
        if (memoryType == MEM_FLASH) {
          flashWriteBlock();
          firmwareWritten += sizeof(Block_buffer);
          progress = (100 * firmwareWritten) / firmwareSize;
        }

        bootloaderDrawScreen(state, progress);

        fr = readBinFile();
        if (BlockCount == 0) {
          state = ST_FLASH_DONE; // EOF
        }
        else if (memoryType == MEM_FLASH && firmwareWritten >= FIRMWARE_MAX_LEN) {
          state = ST_FLASH_DONE; // Backstop
        }
#if defined(SPI_FLASH)
      } else if (state == ST_CLEAR_FLASH_CHECK) {
        bootloaderDrawScreen(state, vpos);
        if (IS_NEXT_EVENT(event)) {
          if (vpos < SEL_CLEAR_FLASH_STORAGE_MENU_LEN - 1) { vpos++; }
          continue;
        }
        if (IS_PREVIOUS_EVENT(event)) {
          if (vpos > 0) { vpos--; }
          continue;
        }
        if (event == EVT_KEY_LONG(KEY_ENTER) && vpos == 0)
        {
          state = ST_CLEAR_FLASH;
        } else if (event == EVT_KEY_BREAK(KEY_EXIT) ||
            (event == EVT_KEY_BREAK(KEY_ENTER) && vpos == 1) ) {
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
        if (unlocked) {
          lockFlash();
          unlocked = 0;
        }

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

  return 0;
}
