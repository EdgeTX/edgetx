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

#include "debug.h"
#include "edgetx.h"
#include "os/sleep.h"
#include "os/task.h"
#include "os/time.h"
#include "os/timer.h"
#include "timers_driver.h"
#include "hal/abnormal_reboot.h"
#include "hal/watchdog_driver.h"

#include "tasks.h"
#include "tasks/mixer_task.h"

#if defined(COLORLCD)
#include "LvglWrapper.h"
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

static void menusTask()
{
#if defined(COLORLCD)
  LvglWrapper::instance();
#endif

  edgeTxInit();

  mixerTaskInit();

#if defined(PWR_BUTTON_PRESS)
  while (task_running()) {
    uint32_t pwr_check = pwrCheck();
    if (pwr_check == e_power_off) {
      break;
    } else if (pwr_check == e_power_press) {
      sleep_ms(MENU_TASK_PERIOD);
      continue;
    }
#else
  while (pwrCheck() != e_power_off) {
#endif
    time_point_t next_tick = time_point_now();
    DEBUG_TIMER_START(debugTimerPerMain);
#if defined(COLORLCD) && defined(CLI)
    if (perMainEnabled) {
      perMain();
    }
#else
    perMain();
#endif
    DEBUG_TIMER_STOP(debugTimerPerMain);

    sleep_until(&next_tick, MENU_TASK_PERIOD);
    resetForcePowerOffRequest();
  }

#if defined(PCBX9E)
  toplcdOff();
#endif

  drawSleepBitmap();
  edgeTxClose();
  boardOff();
}

static void audioTask()
{
  while (!audioQueue.started()) {
    sleep_ms(1);
  }

#if defined(PCBX12S) || defined(RADIO_TX16S) || defined(RADIO_F16) || defined(RADIO_V16)
  // The audio amp needs ~2s to start
  sleep_ms(1000); // 1s
#endif

  time_point_t next_tick = time_point_now();
  while (task_running()) {
    DEBUG_TIMER_SAMPLE(debugTimerAudioIterval);
    DEBUG_TIMER_START(debugTimerAudioDuration);
    audioQueue.wakeup();
    DEBUG_TIMER_STOP(debugTimerAudioDuration);
    sleep_until(&next_tick, 4);
  }
}

static timer_handle_t _timer10ms = TIMER_INITIALIZER;

static void _timer_10ms_cb(timer_handle_t* h)
{
  per10ms();
}

static void timer10msStart()
{
  if (!timer_is_created(&_timer10ms)) {
    timer_create(&_timer10ms, _timer_10ms_cb, "10ms", 10, true);
  }

  timer_start(&_timer10ms);
}

#if defined(COLORLCD) && defined(SIMU)
static timer_handle_t _timer1ms = TIMER_INITIALIZER;

static void _timer_1ms_cb(timer_handle_t* h)
{
  // Increment LVGL animation timer
  lv_tick_inc(1);
}

static void timer1msStart()
{
  if (!timer_is_created(&_timer1ms)) {
    timer_create(&_timer1ms, _timer_1ms_cb, "1ms", 1, true);
  }

  timer_start(&_timer1ms);
}
#endif

void tasksStart()
{
  mutex_create(&audioMutex);

#if defined(CLI) && !defined(SIMU)
  cliStart();
#endif

#if defined(COLORLCD) && defined(SIMU)
  timer1msStart();
#endif

  timer10msStart();

  task_create(&menusTaskId, menusTask, "menus", menusStack, MENUS_STACK_SIZE,
              MENUS_TASK_PRIO);

#if defined(AUDIO)
  task_create(&audioTaskId, audioTask, "audio", audioStack, AUDIO_STACK_SIZE,
              AUDIO_TASK_PRIO);
#endif

  RTOS_START();
}
