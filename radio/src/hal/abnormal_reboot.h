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

#define UNEXPECTED_SHUTDOWN() \
  (abnormalRebootGetCause() == ARC_Watchdog)

#define WAS_RESET_BY_WATCHDOG_OR_SOFTWARE() \
  (abnormalRebootGetCause() != ARC_None)

#define WAS_RESET_BY_SOFTWARE() \
  (abnormalRebootGetCause() == ARC_Software)
