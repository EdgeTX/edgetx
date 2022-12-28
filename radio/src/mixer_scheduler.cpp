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

#include "opentx.h"
#include "mixer_scheduler.h"

bool mixerSchedulerWaitForTrigger(uint8_t timeoutMs)
{
#if !defined(SIMU)
  uint32_t ulNotificationValue;
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS( timeoutMs );

  /* Wait to be notified that the transmission is complete.  Note
     the first parameter is pdTRUE, which has the effect of clearing
     the task's notification value back to 0, making the notification
     value act like a binary (rather than a counting) semaphore.  */
  ulNotificationValue = ulTaskNotifyTake( pdTRUE, xMaxBlockTime );

  if( ulNotificationValue == 1 ) {
    /* The transmission ended as expected. */
    return false;

  } else {
    /* The call to ulTaskNotifyTake() timed out. */
    return true;
  }
#else
  simuSleep(timeoutMs);
  return true;
#endif
}

// Global trigger flag

// Mixer schedule
struct MixerSchedule {

  // period in us
  volatile uint16_t period = MIXER_SCHEDULER_DEFAULT_PERIOD_US;
  volatile uint16_t divider = DOUBLE;
};

static MixerSchedule mixerSchedules[NUM_MODULES]; 
static volatile uint8_t _syncedModule;

// Called from ISR
uint16_t getMixerSchedulerPeriod()
{
  uint8_t synced_module = 0;
  uint8_t synced_modules = 0;
  uint16_t sync_period = mixerSchedules[synced_module].period;

  // Compute minimum period & synced modules
  for(uint8_t module = 0; module < NUM_MODULES; module++) {
    auto& sched = mixerSchedules[module];
    if (sched.period > 0) {
      synced_modules++;
      if (sched.period < sync_period) {
        synced_module = module;
        sync_period = sched.period;
      }
    }
  }

  // adjust sync_period due to mixer task running at doubled frequency
  sync_period = sync_period / DOUBLE;

  // Compute dividers
  for(uint8_t module = 0; module < NUM_MODULES; module++) {
    auto& sched = mixerSchedules[module];
    if(module == synced_module)
      sched.divider = DOUBLE;
    else {
      if(sched.period == 0)
        sched.divider = DOUBLE;
      else {
        sched.divider = sched.period / sync_period;

        //round up if period is not a multiple of sync_period
        if(sched.period % sync_period)    
          sched.divider++;
      }
    }
  }

  _syncedModule = synced_module;
  
  // No active module
  if (synced_modules == 0) {

#if defined(STM32) && !defined(SIMU)
    // no internal/external module and Joystick conntected
    if(getSelectedUsbMode() == USB_JOYSTICK_MODE) {
      return MIXER_SCHEDULER_JOYSTICK_PERIOD_US / DOUBLE;
    }
#endif

    return MIXER_SCHEDULER_DEFAULT_PERIOD_US / DOUBLE;
  }

  // Some module(s) active
  return sync_period; 
}

uint16_t getMixerSchedulerRealPeriod(uint8_t moduleIdx)
{
  // unknown moduleIDX
  if (moduleIdx >= NUM_MODULES) {
    return MIXER_SCHEDULER_DEFAULT_PERIOD_US;
  }

  return (mixerSchedules[_syncedModule].period / DOUBLE)
    * mixerSchedules[moduleIdx].divider;
}

uint16_t getMixerSchedulerDivider(uint8_t moduleIdx) {
  return mixerSchedules[moduleIdx].divider;
}

uint8_t getMixerSchedulerSyncedModule() {
  return _syncedModule;
}

void mixerSchedulerInit()
{
  _syncedModule = 0;
  
  // set default divider and period (for simu as sync not active)
  for(uint8_t i = 0; i < NUM_MODULES; i++) {
    mixerSchedules[i].period = MIXER_SCHEDULER_DEFAULT_PERIOD_US;
    mixerSchedules[i].divider = DOUBLE;
  }
}

void mixerSchedulerSetPeriod(uint8_t moduleIdx, uint16_t periodUs)
{
  if (periodUs > 0 && periodUs < MIN_REFRESH_RATE) {
    periodUs = MIN_REFRESH_RATE;
  }
  else if (periodUs > 0 && periodUs > MAX_REFRESH_RATE) {
    periodUs = MAX_REFRESH_RATE;
  }

  mixerSchedules[moduleIdx].period = periodUs;
}

#if !defined(SIMU)

void mixerSchedulerISRTrigger()
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  /* At this point xTaskToNotify should not be NULL as
     a transmission was in progress. */
  configASSERT( mixerTaskId.rtos_handle != NULL );

  /* Notify the task that the transmission is complete. */
  vTaskNotifyGiveFromISR( mixerTaskId.rtos_handle,
                          &xHigherPriorityTaskWoken );

  /* If xHigherPriorityTaskWoken is now set to pdTRUE then a
     context switch should be performed to ensure the interrupt
     returns directly to the highest priority task.  The macro used
     for this purpose is dependent on the port in use and may be
     called portEND_SWITCHING_ISR(). */
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

#endif
