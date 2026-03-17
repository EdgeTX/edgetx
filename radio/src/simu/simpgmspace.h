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

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>

#include <string>

#define __disable_irq()
#define __enable_irq()

extern char * main_thread_error;

uint64_t simuTimerMicros(void);

void simuSetKey(uint8_t key, bool state);
void simuSetTrim(uint8_t trim, bool state);
void simuSetSwitch(uint8_t swtch, int8_t state);

#if defined(__cplusplus)
void simuInit();
void simuStart(bool tests = true, const char * sdPath = nullptr, const char * settingsPath = nullptr);
void simuStop();
bool simuIsRunning();
void startEepromThread(const char * filename = "eeprom.bin");
void stopEepromThread();
#endif

void simuMain();

void simuFatfsSetPaths(const char * sdPath, const char * settingsPath);

std::string simuFatfsGetCurrentPath();
std::string simuFatfsGetRealPath(const std::string &p);

#if defined(TRACE_SIMPGMSPACE)
  #undef TRACE_SIMPGMSPACE
  #define TRACE_SIMPGMSPACE   TRACE
#else
  #define TRACE_SIMPGMSPACE(...)
#endif

#if defined(HARDWARE_TOUCH)
  extern struct TouchState simTouchState;
  extern bool simTouchOccured;
#endif
