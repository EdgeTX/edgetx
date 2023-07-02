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

#include "stm32_hal_ll.h"

#if defined(BLUETOOTH)
  #include "bluetooth_driver.h"
  #include "stm32_serial_driver.h"
#endif

#include "board.h"
#include "boot.h"
#include "bin_files.h"
#include "dataconstants.h"
#include "lcd.h"
// #include "keys.h"
#include "debug.h"

#include "watchdog_driver.h"

#include "hal/rotary_encoder.h"

#if defined(DEBUG_SEGGER_RTT)
  #include "thirdparty/Segger_RTT/RTT/SEGGER_RTT.h"
#endif

#if defined(PCBXLITE)
  #define BOOTLOADER_KEYS                 0x0F
#else
  #define BOOTLOADER_KEYS                 0x42
#endif

#define APP_START_ADDRESS               (uint32_t)(FIRMWARE_ADDRESS + BOOTLOADER_SIZE)

#if defined(EEPROM)
  #define MAIN_MENU_LEN 3
#else
  #define MAIN_MENU_LEN 2
#endif

typedef void (*voidFunction)(void);

#define jumpTo(addr) do {                                       \
        SCB->VTOR = addr;                                       \
        __set_MSP(*(__IO uint32_t*)addr);                       \
        uint32_t     jumpAddress = *(uint32_t*)(addr + 4);      \
        voidFunction jumpFn = (voidFunction)jumpAddress;        \
        jumpFn();                                               \
    } while(0)

#if !defined(SIMU)
// Bootloader marker:
// -> used to detect valid bootloader files
const uint8_t bootloaderVersion[] __attribute__ ((section(".version"), used)) =
  {'B', 'O', 'O', 'T', '1', '0'};
#endif

#define SOFTRESET_REQUEST 0xCAFEDEAD
  
volatile tmr10ms_t g_tmr10ms;
volatile uint8_t tenms = 1;

uint32_t firmwareSize;
uint32_t firmwareAddress = FIRMWARE_ADDRESS;
uint32_t firmwareWritten = 0;

#if defined(EEPROM)
uint32_t eepromAddress = 0;
uint32_t eepromWritten = 0;
#endif

FlashCheckRes valid;
MemoryType memoryType;
uint32_t unlocked = 0;

void interrupt10ms()
{
  tenms |= 1u; // 10 mS has passed
  g_tmr10ms++;

  keysPollingCycle();

#if defined(ROTARY_ENCODER_NAVIGATION)
  static rotenc_t rePreviousValue;

  rotenc_t reNewValue = rotaryEncoderGetValue();
  int8_t scrollRE = reNewValue - rePreviousValue;
  if (scrollRE) {
    rePreviousValue = reNewValue;
    pushEvent(scrollRE < 0 ? EVT_ROTARY_LEFT : EVT_ROTARY_RIGHT);
  }
#endif
}

void init10msTimer()
{
  INTERRUPT_xMS_TIMER->ARR = 9999;  // 10mS in uS
  INTERRUPT_xMS_TIMER->PSC = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 1000000 - 1; // 1uS
  INTERRUPT_xMS_TIMER->CCER = 0;
  INTERRUPT_xMS_TIMER->CCMR1 = 0;
  INTERRUPT_xMS_TIMER->EGR = 0;
  INTERRUPT_xMS_TIMER->CR1 = 5;
  INTERRUPT_xMS_TIMER->DIER |= 1;
  NVIC_EnableIRQ(INTERRUPT_xMS_IRQn);
}

#if !defined(SIMU)
extern "C" void INTERRUPT_xMS_IRQHandler()
{
  INTERRUPT_xMS_TIMER->SR &= ~TIM_SR_UIF;
  interrupt10ms();
}
#endif

uint32_t isValidBufferStart(const uint8_t * buffer)
{
#if !defined(SIMU)
#if defined(EEPROM)
  if (memoryType == MEM_FLASH)
    return isFirmwareStart(buffer);
  else
    return isEepromStart(buffer);
#else
  return isFirmwareStart(buffer);
#endif
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

static uint32_t PowerUpDelay;

void flashWriteBlock()
{
  uint32_t blockOffset = 0;
  while (BlockCount) {
#if !defined(SIMU)
    flashWrite((uint32_t *)firmwareAddress, (uint32_t *)&Block_buffer[blockOffset]);
#endif
    blockOffset += FLASH_PAGESIZE;
    firmwareAddress += FLASH_PAGESIZE;
    if (BlockCount > FLASH_PAGESIZE) {
      BlockCount -= FLASH_PAGESIZE;
    }
    else {
      BlockCount = 0;
    }
  }
}

#if defined(EEPROM)
void writeEepromBlock()
{
  eepromWriteBlock(Block_buffer, eepromAddress, BlockCount);
  eepromAddress += BlockCount;
}
#endif

#if !defined(SIMU)
void bootloaderInitApp()
{
  RCC_AHB1PeriphClockCmd(PWR_RCC_AHB1Periph | KEYS_RCC_AHB1Periph |
                             LCD_RCC_AHB1Periph | BACKLIGHT_RCC_AHB1Periph |
                             AUX_SERIAL_RCC_AHB1Periph |
                             AUX2_SERIAL_RCC_AHB1Periph |
                             KEYS_BACKLIGHT_RCC_AHB1Periph | SD_RCC_AHB1Periph,
                         ENABLE);

  RCC_APB1PeriphClockCmd(ROTARY_ENCODER_RCC_APB1Periph | LCD_RCC_APB1Periph |
                             BACKLIGHT_RCC_APB1Periph |
                             INTERRUPT_xMS_RCC_APB1Periph | SD_RCC_APB1Periph,
                         ENABLE);

  RCC_APB2PeriphClockCmd(
      LCD_RCC_APB2Periph | BACKLIGHT_RCC_APB2Periph | RCC_APB2Periph_SYSCFG,
      ENABLE);

#if defined(HAVE_BOARD_BOOTLOADER_INIT)
  boardBootloaderInit();
#endif

#if defined(DEBUG_SEGGER_RTT)
  SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_SKIP);
#endif

  pwrInit();
  keysInit();

#if defined(SWSERIALPOWER)
  // TODO: replace with proper serial port query...
  // #if defined(AUX_SERIAL)
  //   void set_aux_pwr(uint8_t on);
  //   set_aux_pwr(0);
  // #endif
  // #if defined(AUX2_SERIAL)
  //   void set_aux2_pwr(uint8_t on);
  //   set_aux2_pwr(0);
  // #endif
#endif

  // wait a bit for the inputs to stabilize...
  if (!WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) {
    for (uint32_t i = 0; i < 150000; i++) {
      __ASM volatile ("nop");
    }
  }

#if (defined(RADIO_T8) || defined(RADIO_COMMANDO8)) && !defined(RADIOMASTER_RELEASE)
  // Bind button not pressed
  if ((~(KEYS_GPIO_REG_BIND->IDR) & KEYS_GPIO_PIN_BIND) == false) {
#else
  // LHR & RHL trims not pressed simultanously
  if (readTrims() != BOOTLOADER_KEYS) {
#endif
    // Start main application
    jumpTo(APP_START_ADDRESS);
  }

  pwrOn();

#if defined(ROTARY_ENCODER_NAVIGATION)
  rotaryEncoderInit();
#endif

  delaysInit(); // needed for lcdInit()

#if defined(DEBUG)
  initSerialPorts();
#endif

  __enable_irq();

  TRACE("\nBootloader started :)");

#if defined(BLUETOOTH)
  // we shutdown the bluetooth module now to be sure it will be detected on firmware start
  bluetoothInit(BLUETOOTH_DEFAULT_BAUDRATE, false);
#endif

#if defined(EEPROM) && defined(EEPROM_RLC)
  eepromInit();
#endif

  init10msTimer();

  // SD card detect pin
  sdInit();
  usbInit();
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
  while (pwrPressed()) {
    WDG_RESET();
  }
#endif

  for (;;) {
    WDG_RESET();

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
#if defined(EEPROM)
            case 1:
              memoryType = MEM_EEPROM;
              state = ST_DIR_CHECK;
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
            firmwareSize = binFiles[vpos].size - BOOTLOADER_SIZE;
            firmwareAddress = FIRMWARE_ADDRESS + BOOTLOADER_SIZE;
            firmwareWritten = 0;
          }
#if defined(EEPROM)
          else {
            eepromAddress = 0;
            eepromWritten = 0;
          }
#endif
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
#if defined(EEPROM)
        else {
          writeEepromBlock();
          eepromWritten += sizeof(Block_buffer);
          progress = (100 * eepromWritten) / EEPROM_SIZE;
        }
#endif

        bootloaderDrawScreen(state, progress);

        fr = readBinFile();
        if (BlockCount == 0) {
          state = ST_FLASH_DONE; // EOF
        }
        else if (memoryType == MEM_FLASH && firmwareWritten >= FLASHSIZE - BOOTLOADER_SIZE) {
          state = ST_FLASH_DONE; // Backstop
        }
#if defined(EEPROM)
        else if (memoryType == MEM_EEPROM && eepromWritten >= EEPROM_SIZE) {
          state = ST_FLASH_DONE; // Backstop
        }
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

      if (PowerUpDelay < 20) {  // 200 mS
        PowerUpDelay += 1;
      }
      else {
        sdPoll10ms();
      }
    }

    if (state != ST_FLASHING && state != ST_USB) {
      if (pwrOffPressed()) {
        boardOff();
      }
    }

    if (state == ST_REBOOT) {
#if !defined(SIMU)
#if defined(RTC_BACKUP_RAM)
      rtcInit();
      RTC->BKP0R = SOFTRESET_REQUEST;
#endif
      blExit();
      NVIC_SystemReset();
#else
      exit(1);
#endif
    }
  }

  return 0;
}

#if !defined(SIMU) && (defined(PCBHORUS) || defined(PCBNV14))
void *__dso_handle = nullptr;
#endif
