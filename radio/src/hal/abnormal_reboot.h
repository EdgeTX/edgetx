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

#pragma once

#include <stdint.h>

enum AbnormalRebootCause {
  ARC_None = 0,
  ARC_Watchdog,
  ARC_Software,
};

// Enable detecting abnormal reboots
// This should be called after booting
void abnormalRebootEnableDetection();

// Disable detecting abnormal reboots
// This should be called on normal shutdowns / reboots
void abnormalRebootDisableDetection();

// Test for abnormal reboot conditions
// (see AbnormalRebootCause)
uint32_t abnormalRebootGetCause();

// Retrieve last reboot command
uint32_t abnormalRebootGetCmd();

// Reset reboot command
void abnormalRebootResetCmd();

// Reboot command: start the bootloader in DFU mode
#define REBOOT_CMD_DFU 0x55464442
// Set by the bootloader before starting a firmware the host asked for
// (DFU leave): the firmware powers on as after a software reset, without
// waiting for the power button.
#define REBOOT_CMD_POWER_ON 0x4E4F5750

// Set the command read by the bootloader after the next reset
void abnormalRebootSetCmd(uint32_t cmd);

#if defined(BOOTLOADER_DFU)
// On these boards a hardware reset with USB attached starts the ST ROM
// bootloader (BOOT0 follows USB VBUS while the main power is off), so the
// bootloader and the firmware hand over to each other without a reset.

// Tell the firmware about to be started that the host asked for it: it
// powers on as after a software reset. Clears the reset flags, which may
// still hold the cause of an older reset (e.g. the one that started the ST
// ROM bootloader).
void abnormalRebootRequestPowerOn();

// Interrupts off, every peripheral but the GPIO ports (power latch) reset
void abnormalRebootQuiesce();

// True when started by the ST ROM bootloader (e.g. its DFU leave) rather
// than from a reset: system memory is still mapped at address 0
bool abnormalRebootStartedFromSystemMemory();

// Clocks back to their reset state (HSI, PLLs off)
void abnormalRebootResetClocks();

// Start the image at 'address' (vector table) in privileged thread mode on
// the main stack
__attribute__((noreturn)) void abnormalRebootJump(uint32_t address);
#endif

#define UNEXPECTED_SHUTDOWN() \
  (abnormalRebootGetCause() == ARC_Watchdog)

#define WAS_RESET_BY_WATCHDOG_OR_SOFTWARE() \
  (abnormalRebootGetCause() != ARC_None)

#define WAS_RESET_BY_SOFTWARE() \
  (abnormalRebootGetCause() == ARC_Software)
