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
#include "hal/abnormal_reboot.h"
#include "hal/rotary_encoder.h"

#include "board.h"
#include "debug.h"

#include "timers_driver.h"

#if defined(BLUETOOTH)
  #include "bluetooth_driver.h"
  #include "stm32_serial_driver.h"
#endif

#if defined(DEBUG_SEGGER_RTT)
  #include "thirdparty/Segger/SEGGER/SEGGER_RTT.h"
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
      delay_ms(10);
    }

    start_firmware = !boardBLStartCondition(); 
    if (start_firmware) {
      // Start main application
      boardBLPreJump();
      jumpTo(APP_START_ADDRESS);
    }
  } else {
#if defined(FIRMWARE_QSPI)
    abnormalRebootResetCmd();
    setSelectedUsbMode(USB_DFU_MODE);
#endif
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

  usbInit();
  boardBLInit();
}

#if defined(FIRMWARE_FORMAT_UF2)
// make linker happy
void per5ms() {}
#endif

int main()
#else // SIMU
void bootloaderInitApp() {}
int  bootloaderMain()
#endif
{
  // init hardware (may jump to app)
  bootloaderInitApp();

  // init screen
  bootloaderInitScreen();

#if defined(FIRMWARE_FORMAT_UF2)
  if (getSelectedUsbMode() == USB_DFU_MODE) {
    bootloaderDFU();
  }
#endif

#if defined(PWR_BUTTON_PRESS)
  // wait until power button is released
  while (pwrPressed()) {}
#endif

#if !defined(FIRMWARE_FORMAT_UF2)
  bootloaderMenu();
#else
  bootloaderUF2();
#endif

  return 0;
}
