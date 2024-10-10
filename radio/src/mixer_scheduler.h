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

#define MIXER_SCHEDULER_DEFAULT_PERIOD_US  4000u // 4ms
#define MIXER_SCHEDULER_JOYSTICK_PERIOD_US 1000u // 1ms

#define MIN_REFRESH_RATE       850 /* us */
#define MAX_REFRESH_RATE     50000 /* us */

#if !defined(SIMU)

// Call once to initialize the mixer scheduler
void mixerSchedulerInit();

// Configure and start the scheduler timer
void mixerSchedulerStart();

// Stop the scheduler timer
void mixerSchedulerStop();

// Set the scheduling period for a given module
void mixerSchedulerSetPeriod(uint8_t moduleIdx, uint16_t periodUs);

// Get the scheduling period for a given module
uint16_t mixerSchedulerGetPeriod(uint8_t moduleIdx);

// Enable the timer trigger
void mixerSchedulerEnableTrigger();

// Disable the timer trigger
void mixerSchedulerDisableTrigger();

// Trigger mixer from heartbeat interrupt 
void mixerSchedulerSoftTrigger();

// Fetch the current scheduling period
uint16_t getMixerSchedulerPeriod();

// Trigger mixer from an ISR
void mixerSchedulerISRTrigger();

#else

#define mixerSchedulerInit()
#define mixerSchedulerStart()
#define mixerSchedulerStop()
#define mixerSchedulerSetPeriod(m,p) ((void)(p))
#define mixerSchedulerGetPeriod(m) ((uint16_t)MIXER_SCHEDULER_DEFAULT_PERIOD_US)

#define mixerSchedulerEnableTrigger()
#define mixerSchedulerDisableTrigger()

#define mixerSchedulerSoftTrigger()

#define getMixerSchedulerPeriod() (MIXER_SCHEDULER_DEFAULT_PERIOD_US)
#define mixerSchedulerISRTrigger()

#endif

// Wait for the scheduler timer to trigger
// returns true if timeout, false otherwise
bool mixerSchedulerWaitForTrigger(uint8_t timeoutMs);
