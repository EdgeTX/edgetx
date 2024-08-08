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

#if defined(BUZZER)
extern uint8_t beepAgainOrig;
extern uint8_t beepOn;
extern bool warble;
extern bool warbleC;
#if defined(HAPTIC)
extern uint8_t hapticTick;
#endif /* HAPTIC */
#endif /* BUZZER */

#if defined(BUZZER)

#if defined(SIMU)
inline void _beep(uint8_t b)
{
  g_beepCnt = b;
}
#else /* SIMU */
inline void _beep(uint8_t b)
{
  buzzerSound(b);
}
#endif /* SIMU */

void beep(uint8_t val);
#else /* BUZZER */
inline void beep(uint8_t) { }
#endif /* BUZZER */

#if !defined(AUDIO)

#if defined(BUZZER)
    #define AUDIO_HELLO()           PUSH_SYSTEM_PROMPT(AUDIO_HELLO)
    #define AUDIO_BYE()
    #define AUDIO_TX_BATTERY_LOW() PUSH_SYSTEM_PROMPT(AU_TX_BATTERY_LOW)
    #define AUDIO_INACTIVITY()     PUSH_SYSTEM_PROMPT(AU_INACTIVITY)
    #define AUDIO_ERROR_MESSAGE(e) PUSH_SYSTEM_PROMPT((e))
    #define AUDIO_TIMER_MINUTE(t)  playDuration(t)
    // TODO
    #define AUDIO_TIMER_30()       PUSH_SYSTEM_PROMPT(AU_TIMER_30)
    #define AUDIO_TIMER_20()       PUSH_SYSTEM_PROMPT(AU_TIMER_20)

  #define AUDIO_KEY_PRESS()        beep(0)
  #define AUDIO_KEY_ERROR()        beep(2)
  #define AUDIO_WARNING2()         beep(2)
  #define AUDIO_WARNING1()         beep(3)
  #define AUDIO_ERROR()            beep(4)
  #define AUDIO_MIX_WARNING(x)     beep(1)
  #define AUDIO_POT_MIDDLE(x)      beep(2)
  #define AUDIO_TIMER_COUNTDOWN(idx, val)  beep(2)
  #define AUDIO_TIMER_ELAPSED(idx) beep(3)
  #define AUDIO_VARIO_UP()         _beep(1)
  #define AUDIO_VARIO_DOWN()       _beep(1)
  #define AUDIO_TRIM_PRESS(f)      { if (!IS_KEY_FIRST(event)) warble = true; beep(1); }
  #define AUDIO_TRIM_MIDDLE()      beep(2)
  #define AUDIO_TRIM_MIN()         beep(2)
  #define AUDIO_TRIM_MAX()         beep(2)
  #define AUDIO_PLAY(p)            beep(3)

  #define IS_AUDIO_BUSY() (g_beepCnt || beepAgain || beepOn)
#else /* BUZZER */
  #define AUDIO_HELLO()
  #define AUDIO_BYE()
  #define AUDIO_TX_BATTERY_LOW()
  #define AUDIO_INACTIVITY()
  #define AUDIO_ERROR_MESSAGE(e)
  #define AUDIO_TIMER_MINUTE(t)
  #define AUDIO_TIMER_30()
  #define AUDIO_TIMER_20()
  #define AUDIO_WARNING2()
  #define AUDIO_WARNING1()
  #define AUDIO_ERROR()
  #define AUDIO_MIX_WARNING(x)
  #define AUDIO_POT_MIDDLE(x)
  #define AUDIO_TIMER_LT10(m, x)
  #define AUDIO_TIMER_00(m)
  #define AUDIO_VARIO_UP()
  #define AUDIO_VARIO_DOWN()
  #define AUDIO_TRIM(event, f)
  #define AUDIO_TRIM_MIDDLE(f)
  #define AUDIO_TRIM_END(f)
  #define AUDIO_PLAY(p)
  #define IS_AUDIO_BUSY() false
#endif /* BUZZER */

  #define AUDIO_RESET()
  #define AUDIO_FLUSH()

  #define PLAY_PHASE_OFF(phase)
  #define PLAY_PHASE_ON(phase)
  #define PLAY_SWITCH_MOVED(sw)
  #define PLAY_LOGICAL_SWITCH_OFF(sw)
  #define PLAY_LOGICAL_SWITCH_ON(sw)
  #define PLAY_MODEL_NAME()
  #define START_SILENCE_PERIOD()
#endif /* !AUDIO */
