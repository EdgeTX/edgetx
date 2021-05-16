/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _MIXER_SCHEDULER_H_
#define _MIXER_SCHEDULER_H_

#define MIXER_SCHEDULER_DEFAULT_PERIOD_US 4000u // 4ms

#define MIN_REFRESH_RATE      1750 /* us */
#define MAX_REFRESH_RATE     25000 /* us */

#if !defined(SIMU)

// Call once to initialize the mixer scheduler
void mixerSchedulerInit();

// Configure and start the scheduler timer
void mixerSchedulerStart();

// Stop the scheduler timer
void mixerSchedulerStop();

// Set the timer counter to 0
void mixerSchedulerResetTimer();

// Set the scheduling period for a given module
void mixerSchedulerSetPeriod(uint8_t moduleIdx, uint16_t periodUs);

// Clear the flag before waiting
void mixerSchedulerClearTrigger();

// Wait for the scheduler timer to trigger
// returns true if timeout, false otherwise
bool mixerSchedulerWaitForTrigger(uint8_t timeoutMs);

// Enable the timer trigger
void mixerSchedulerEnableTrigger();

// Disable the timer trigger
void mixerSchedulerDisableTrigger();

// Fetch the current scheduling period
uint16_t getMixerSchedulerPeriod();

// Trigger mixer from an ISR
void mixerSchedulerISRTrigger();

#else

#define mixerSchedulerInit()
#define mixerSchedulerStart()
#define mixerSchedulerStop()
#define mixerSchedulerResetTimer()
#define mixerSchedulerSetPeriod(m,p)
#define mixerSchedulerClearTrigger()

static inline bool mixerSchedulerWaitForTrigger(uint8_t timeout)
{
  simuSleep(timeout);
  return false;
}

#define mixerSchedulerEnableTrigger()
#define mixerSchedulerDisableTrigger()

#define getMixerSchedulerPeriod() (MIXER_SCHEDULER_DEFAULT_PERIOD_US)
#define mixerSchedulerISRTrigger()

#endif

#endif
