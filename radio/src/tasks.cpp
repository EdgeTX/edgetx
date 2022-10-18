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
#include "timers_driver.h"

#include "tasks.h"
#include "tasks/mixer_task.h"

RTOS_TASK_HANDLE menusTaskId;
RTOS_DEFINE_STACK(menusTaskId, menusStack, MENUS_STACK_SIZE);

RTOS_TASK_HANDLE audioTaskId;
RTOS_DEFINE_STACK(audioTaskId, audioStack, AUDIO_STACK_SIZE);

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

#if defined(SPLASH) && !defined(STARTUP_ANIMATION)
  if (!UNEXPECTED_SHUTDOWN()) {
    drawSplash();
    TRACE("drawSplash() completed");
  }
#endif

#if defined(HARDWARE_TOUCH) && !defined(PCBFLYSKY) && !defined(SIMU)
  touchPanelInit();
#endif

  opentxInit();

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
  opentxClose();
  boardOff();

  TASK_RETURN();
}

void tasksStart()
{
  RTOS_CREATE_MUTEX(audioMutex);

#if defined(CLI) && !defined(SIMU)
  cliStart();
#endif

  mixerTaskInit();

  RTOS_CREATE_TASK(menusTaskId, menusTask, "menus", menusStack,
                   MENUS_STACK_SIZE, MENUS_TASK_PRIO);

#if !defined(SIMU)
  RTOS_CREATE_TASK(audioTaskId, audioTask, "audio", audioStack,
                   AUDIO_STACK_SIZE, AUDIO_TASK_PRIO);
#endif

  RTOS_START();
}
