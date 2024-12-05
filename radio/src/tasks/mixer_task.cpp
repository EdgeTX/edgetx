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

#include "tasks.h"
#include "mixer_task.h"
#include "mixer_scheduler.h"

#include "edgetx.h"
#include "switches.h"
#include "hal/usb_driver.h"

#include "hal/watchdog_driver.h"
#if defined(HALL_SYNC) && !defined(SIMU)
#include "stm32_gpio.h"
#include "hal/gpio.h"
#endif

RTOS_TASK_HANDLE mixerTaskId;
RTOS_DEFINE_STACK(mixerTaskId, mixerStack, MIXER_STACK_SIZE);

// mixer hold this mutex while computing
// channels and sending them out
static RTOS_MUTEX_HANDLE mixerMutex;

// The mixer will start in 'paused' mode
// and start working properly once
// mixerTaskStart() has been called.
static bool _mixer_started = false;
static bool _mixer_running = false;
static bool _mixer_exit = false;

void mixerTaskLock()
{
  RTOS_LOCK_MUTEX(mixerMutex);
}

// returns true if the lock could be acquired
bool mixerTaskTryLock()
{
  return RTOS_TRYLOCK_MUTEX(mixerMutex);
}

void mixerTaskUnlock()
{
  RTOS_UNLOCK_MUTEX(mixerMutex);
}

void mixerTaskInit()
{
  mixerSchedulerInit();
  RTOS_CREATE_MUTEX(mixerMutex);
  RTOS_CREATE_TASK(mixerTaskId, mixerTask, "mixer", mixerStack,
                   MIXER_STACK_SIZE, MIXER_TASK_PRIO);
  mixerSchedulerStart();
}

bool mixerTaskStarted()
{
  return _mixer_started;
}

void mixerTaskStart()
{
  _mixer_started = true;
  _mixer_running = true;
}

void mixerTaskStop()
{
  mixerTaskLock();
  _mixer_running = false;
  mixerTaskUnlock();
}

void mixerTaskExit()
{
  _mixer_exit = true;
}

bool mixerTaskRunning()
{
  return _mixer_running && !_mixer_exit;
}

volatile uint16_t timeForcePowerOffPressed = 0;

bool isForcePowerOffRequested()
{
  if (pwrOffPressed()) {
    if (timeForcePowerOffPressed == 0) {
      timeForcePowerOffPressed = get_tmr10ms();
    }
    else {
      uint16_t delay = (uint16_t)get_tmr10ms() - timeForcePowerOffPressed;
      if (delay > 1000/*10s*/) {
        return true;
      }
    }
  }
  else {
    resetForcePowerOffRequest();
  }
  return false;
}

constexpr uint8_t MIXER_FREQUENT_ACTIONS_PERIOD = 5 /*ms*/;
constexpr uint8_t MIXER_MAX_PERIOD = MAX_REFRESH_RATE / 1000 /*ms*/;

void execMixerFrequentActions()
{
#if defined(IMU)
  gyro.wakeup();
#endif

#if defined(BLUETOOTH)
  bluetooth.wakeup();
#endif

#if defined(SIMU)
  if (_mixer_running) {
    DEBUG_TIMER_START(debugTimerTelemetryWakeup);
    telemetryWakeup();
    DEBUG_TIMER_STOP(debugTimerTelemetryWakeup);
  }
#endif
}

TASK_FUNCTION(mixerTask)
{
#if defined(IMU)
  gyroInit();
#endif

  while (!_mixer_exit) {

    int timeout = 0;
    for (; timeout < MIXER_MAX_PERIOD; timeout += MIXER_FREQUENT_ACTIONS_PERIOD) {

      // run periodicals before waiting for the trigger
      // to keep the delay short
      execMixerFrequentActions();

      // mixer flag triggered?
      if (!mixerSchedulerWaitForTrigger(MIXER_FREQUENT_ACTIONS_PERIOD)) {
        break;
      }
    }

#if defined(DEBUG_MIXER_SCHEDULER)
    gpio_set(EXTMODULE_TX_GPIO);
    gpio_clear(EXTMODULE_TX_GPIO);
#endif

    // re-enable trigger
    mixerSchedulerEnableTrigger();

#if defined(SIMU)
    // TODO: should be using mixerTaskExit() instead...
    if (pwrCheck() == e_power_off) {
      TASK_RETURN();
    }
#else
    // Emergency power OFF: in case the UI is not functional anymore
    // or maybe locked, this will effectively shutdown and cut power.
    if (isForcePowerOffRequested()) {
      boardOff();
    }
#endif

    if (_mixer_running) {

      uint32_t t0 = timersGetUsTick();

      DEBUG_TIMER_START(debugTimerMixer);
      mixerTaskLock();

      doMixerCalculations();
      pulsesSendChannels();
      doMixerPeriodicUpdates();

      // TODO: what are these for???
      DEBUG_TIMER_START(debugTimerMixerCalcToUsage);
      DEBUG_TIMER_SAMPLE(debugTimerMixerIterval);

      mixerTaskUnlock();
      DEBUG_TIMER_STOP(debugTimerMixer);

#if defined(STM32) && !defined(SIMU)
      if (getSelectedUsbMode() == USB_JOYSTICK_MODE) {
        usbJoystickUpdate();
      }
#endif

      // we are the main actor to reset the watchdog timer
      // so let's do it here.
      WDG_RESET();

      t0 = timersGetUsTick() - t0;
      if (t0 > maxMixerDuration)
        maxMixerDuration = t0;
    }
  }

  TASK_RETURN();
}

void doMixerCalculations()
{
  static tmr10ms_t lastTMR = 0;

  tmr10ms_t tmr10ms = get_tmr10ms();

#if defined(HALL_SYNC) && !defined(SIMU)
  gpio_set(HALL_SYNC);
#endif

#if defined(DEBUG_LATENCY_MIXER_RF) || defined(DEBUG_LATENCY_RF_ONLY)
  static tmr10ms_t lastLatencyToggle = 0;
  if (tmr10ms - lastLatencyToggle >= 10) {
    lastLatencyToggle = tmr10ms;
    toggleLatencySwitch();
  }
#endif

  uint8_t tick10ms = (tmr10ms >= lastTMR ? tmr10ms - lastTMR : 1);
  // handle tick10ms overrun
  // correct overflow handling costs a lot of code; happens only each 11 min;
  // therefore forget the exact calculation and use only 1 instead; good compromise
  lastTMR = tmr10ms;

  DEBUG_TIMER_START(debugTimerGetAdc);
  getADC();
  DEBUG_TIMER_STOP(debugTimerGetAdc);

  DEBUG_TIMER_START(debugTimerGetSwitches);
  getSwitchesPosition(!s_mixer_first_run_done);
  DEBUG_TIMER_STOP(debugTimerGetSwitches);

  DEBUG_TIMER_START(debugTimerEvalMixes);
  evalMixes(tick10ms);
  DEBUG_TIMER_STOP(debugTimerEvalMixes);

#if defined(HALL_SYNC) && !defined(SIMU)
  gpio_clear(HALL_SYNC);
#endif
}
