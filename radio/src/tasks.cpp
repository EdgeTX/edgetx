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
#include "os/sleep.h"
#include "os/task.h"
#include "os/time.h"
#include "timers_driver.h"
#include "hal/abnormal_reboot.h"
#include "hal/watchdog_driver.h"

#include "tasks.h"
#include "tasks/mixer_task.h"


#if defined(LIBOPENUI)
#include "startup_shutdown.h"
#endif

task_handle_t menusTaskId;
TASK_DEFINE_STACK(menusStack, MENUS_STACK_SIZE);

#if defined(AUDIO)
task_handle_t audioTaskId;
TASK_DEFINE_STACK(audioStack, AUDIO_STACK_SIZE);
#endif

mutex_handle_t audioMutex;

#define MENU_TASK_PERIOD (50)  // 50ms

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
      sleep_ms(MENU_TASK_PERIOD);
      continue;
    }
#else
  while (pwrCheck() != e_power_off) {
#endif
    uint32_t start = time_get_ms();
    DEBUG_TIMER_START(debugTimerPerMain);
#if defined(COLORLCD) && defined(CLI)
    if (perMainEnabled) {
      perMain();
    }
#else
    perMain();
#endif
    DEBUG_TIMER_STOP(debugTimerPerMain);

    // deduct the thread run-time from the wait, if run-time was more than
    // desired period, then skip the wait all together
    uint32_t runtime = (time_get_ms() - start);
    if (runtime < MENU_TASK_PERIOD) {
      sleep_ms(MENU_TASK_PERIOD - runtime);
    }

    resetForcePowerOffRequest();
  }

#if defined(PCBX9E)
  toplcdOff();
#endif

  drawSleepBitmap();
  edgeTxClose();
  boardOff();

  TASK_RETURN;
}

TASK_FUNCTION(audioTask)
{
  while (!audioQueue.started()) {
    sleep_ms(1);
  }

#if defined(PCBX12S) || defined(RADIO_TX16S) || defined(RADIO_F16) || defined(RADIO_V16)
  // The audio amp needs ~2s to start
  sleep_ms(1000); // 1s
#endif

  while (true) {
    DEBUG_TIMER_SAMPLE(debugTimerAudioIterval);
    DEBUG_TIMER_START(debugTimerAudioDuration);
    audioQueue.wakeup();
    DEBUG_TIMER_STOP(debugTimerAudioDuration);
    sleep_ms(4); // ???
  }
}

void tasksStart()
{
  mutex_create(&audioMutex);

#if defined(CLI) && !defined(SIMU)
  cliStart();
#endif

  task_create(&menusTaskId, menusTask, "menus", menusStack, MENUS_STACK_SIZE,
              MENUS_TASK_PRIO);

#if defined(AUDIO)
  task_create(&audioTaskId, audioTask, "audio", audioStack, AUDIO_STACK_SIZE,
              AUDIO_TASK_PRIO);
#endif

  RTOS_START();
}
