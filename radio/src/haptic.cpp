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
#include "haptic.h"

hapticQueue::hapticQueue()
{
  buzzTimeLeft = 0;
  buzzPause = 0;

  t_queueRidx = 0;
  t_queueWidx = 0;

  hapticTick = 0;
}

void hapticQueue::heartbeat()
{
#if defined(SIMU)
  return;
#else
  if (buzzTimeLeft > 0) {
    buzzTimeLeft--; // time gets counted down
#if defined(HAPTIC_PWM)
    hapticOn(HAPTIC_STRENGTH() * 20);
#else
    if (hapticTick-- > 0) {
      hapticOn();
    }
    else {
      hapticOff();
      hapticTick = HAPTIC_STRENGTH();
    }
#endif
  }
  else {
    hapticOff();
    if (buzzPause > 0) {
      buzzPause--;
    }
    else if (t_queueRidx != t_queueWidx) {
      buzzTimeLeft = queueHapticLength[t_queueRidx];
      buzzPause = queueHapticPause[t_queueRidx];
      if (!queueHapticRepeat[t_queueRidx]--) {
        t_queueRidx = (t_queueRidx + 1) & (HAPTIC_QUEUE_LENGTH-1);
      }
    }
  }
#endif // defined(SIMU)
}

void hapticQueue::play(uint8_t tLen, uint8_t tPause, uint8_t tFlags)
{
  tLen = getHapticLength(tLen);

  if ((tFlags & PLAY_NOW) || (!busy() && empty())) {
    buzzTimeLeft = tLen;
    buzzPause = tPause;
    t_queueWidx = t_queueRidx;
  }
  else {
    tFlags += 1;
  }

  tFlags &= 0x0f;
  if (tFlags) {
    uint8_t next_queueWidx = (t_queueWidx + 1) & (HAPTIC_QUEUE_LENGTH-1);
    if (next_queueWidx != t_queueRidx) {
      queueHapticLength[t_queueWidx] = tLen;
      queueHapticPause[t_queueWidx] = tPause;
      queueHapticRepeat[t_queueWidx] = tFlags-1;
      t_queueWidx = next_queueWidx;
    }
  }
}

void hapticQueue::event(uint8_t e)
{
  if (g_eeGeneral.hapticMode >= e_mode_nokeys || (g_eeGeneral.hapticMode >= e_mode_alarms && e <= AU_ERROR)) {
    if (e <= AU_ERROR)
      play(15, 3, PLAY_NOW);
    else if (e <= AU_MIX_WARNING_3)
      play(15, 3, PLAY_NOW);
    else if (e >= AU_SPECIAL_SOUND_LAST && empty()) {
      play(30, 10, 0);
      play(10, 50-10*(e-AU_SPECIAL_SOUND_LAST), (e-AU_SPECIAL_SOUND_LAST));
    }
  }
}

hapticQueue haptic;

// from timers_driver.cpp
void per5ms()
{
  DEBUG_TIMER_START(debugTimerHaptic);
  HAPTIC_HEARTBEAT();
  DEBUG_TIMER_STOP(debugTimerHaptic);
}
