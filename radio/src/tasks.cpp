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

#include "edgetx.h"
#include "timers_driver.h"
#include "hal/abnormal_reboot.h"
#include "hal/watchdog_driver.h"

#include "tasks.h"
#include "tasks/mixer_task.h"


#if defined(LIBOPENUI)
#include "startup_shutdown.h"
#endif

RTOS_TASK_HANDLE menusTaskId;
RTOS_DEFINE_STACK(menusTaskId, menusStack, MENUS_STACK_SIZE);

#if defined(AUDIO)
RTOS_TASK_HANDLE audioTaskId;
RTOS_DEFINE_STACK(audioTaskId, audioStack, AUDIO_STACK_SIZE);
#endif

RTOS_MUTEX_HANDLE audioMutex;

#define MENU_TASK_PERIOD_TICKS         (50 / RTOS_MS_PER_TICK)    // 50ms

#if defined(COLORLCD) && defined(CLI)
bool perMainEnabled = true;
#endif

TASK_FUNCTION(menusTask)
{
#if defined(LIBOPENUI)
  LvglWrapper::instance();
#endif

  edgeTxInit();

  mixerTaskInit();

#if defined(PWR_BUTTON_PRESS)
  while (true) {
    uint32_t pwr_check = pwrCheck();
    if (pwr_check == e_power_off) {
      break;
    }
    else if (pwr_check == e_power_press) {
      RTOS_WAIT_TICKS(MENU_TASK_PERIOD_TICKS);
      continue;
    }
#else
  while (pwrCheck() != e_power_off) {
#endif
    uint32_t start = (uint32_t)RTOS_GET_TIME();
    DEBUG_TIMER_START(debugTimerPerMain);
#if defined(COLORLCD) && defined(CLI)
    if (perMainEnabled) {
      perMain();
    }
#else
    perMain();
#endif
    DEBUG_TIMER_STOP(debugTimerPerMain);
    // TODO remove completely massstorage from sky9x firmware
    uint32_t runtime = ((uint32_t)RTOS_GET_TIME() - start);
    // deduct the thread run-time from the wait, if run-time was more than
    // desired period, then skip the wait all together
    if (runtime < MENU_TASK_PERIOD_TICKS) {
      RTOS_WAIT_TICKS(MENU_TASK_PERIOD_TICKS - runtime);
    }

    resetForcePowerOffRequest();
  }

#if defined(PCBX9E)
  toplcdOff();
#endif

  drawSleepBitmap();
  edgeTxClose();
  boardOff();

  TASK_RETURN();
}

#if !defined(SIMU)
// Handle 10ms timer asynchronously
#include <FreeRTOS/include/FreeRTOS.h>
#include <FreeRTOS/include/timers.h>

static TimerHandle_t _timer10ms = nullptr;
static StaticTimer_t _timer10msBuffer;

static void _timer_10ms_cb(TimerHandle_t xTimer)
{
  (void)xTimer;
  per10ms();
}

void timer10msStart()
{
  if (!_timer10ms) {
    _timer10ms =
        xTimerCreateStatic("10ms", 10 / RTOS_MS_PER_TICK, pdTRUE, (void*)0,
                           _timer_10ms_cb, &_timer10msBuffer);
  }

  if (_timer10ms) {
    if( xTimerStart( _timer10ms, 0 ) != pdPASS ) {
      /* The timer could not be set into the Active state. */
    }
  }
}
#endif

void tasksStart()
{
  RTOS_CREATE_MUTEX(audioMutex);

#if defined(CLI) && !defined(SIMU)
  cliStart();
#endif

#if !defined(SIMU)
  timer10msStart();
#endif

  RTOS_CREATE_TASK(menusTaskId, menusTask, "menus", menusStack,
                   MENUS_STACK_SIZE, MENUS_TASK_PRIO);

#if !defined(SIMU) && defined(AUDIO)
  RTOS_CREATE_TASK(audioTaskId, audioTask, "audio", audioStack,
                   AUDIO_STACK_SIZE, AUDIO_TASK_PRIO);
#endif

  RTOS_START();
}
