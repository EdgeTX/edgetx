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

#include "keys.h"

#include "opentx_helpers.h"
#include "definitions.h"

#include "timers_driver.h"
#include "watchdog_driver.h"
#include "hal/rotary_encoder.h"

// required by watchdog macro..
#if !defined(SIMU)
#include "stm32_cmsis.h"
#endif

// long key press minimum duration (x10ms),
// must be less than KEY_REPEAT_DELAY
#define KEY_LONG_DELAY              32

// press longer than this enables repeat (but does not fire it yet)
#define KEY_REPEAT_DELAY            40

// repeat trigger, used in combination with m_state
// to produce decreasing times between repeat events
#define KEY_REPEAT_TRIGGER          48

#define KEY_REPEAT_PAUSE_DELAY      64

// defines how many bits are used for debounce
#if defined(SIMU)
  #define FILTERBITS 1
#else
  #define FILTERBITS 4
#endif

#define FILTER_MASK ((1 << FILTERBITS) - 1)

#define KSTATE_OFF                  0
#define KSTATE_RPTDELAY             95
#define KSTATE_START                97
#define KSTATE_PAUSE                98
#define KSTATE_KILLED               99

// global last event
event_t s_evt;
event_t s_trim_evt;

InactivityData inactivity = {0};

class Key
{
 private:
  uint8_t m_vals = 0;
  uint8_t m_cnt = 0;
  uint8_t m_state = 0;

 public:
  event_t input(bool val);
  bool pressed() const { return (m_vals & FILTER_MASK) == FILTER_MASK; }
  void pauseEvents();
  void killEvents();
};

event_t Key::input(bool val)
{
  event_t evt = 0;

  // store new value in the bits that hold the key state history (used for debounce)
  uint8_t t_vals = m_vals ;
  t_vals <<= 1 ;
  if (val) t_vals |= 1;
  m_vals = t_vals ;

  m_cnt++;

  if (m_state && m_vals == 0) {
    // key is released
    if (m_state != KSTATE_KILLED) {
      evt = _MSK_KEY_BREAK;
    }
    m_state = KSTATE_OFF;
    m_cnt = 0;
    return evt;
  }

  switch (m_state) {
    case KSTATE_OFF:
      if (m_vals == FILTER_MASK) {
        m_state = KSTATE_START;
        m_cnt = 0;
      }
      break;

    case KSTATE_START:
      evt = _MSK_KEY_FIRST;
      inactivity.counter = 0;
      m_state = KSTATE_RPTDELAY;
      m_cnt = 0;
      break;

    case KSTATE_RPTDELAY: // gruvin: delay state before first key repeat
      if (m_cnt == KEY_LONG_DELAY) {
        // generate long key press
        evt = _MSK_KEY_LONG;
      }
      if (m_cnt == KEY_REPEAT_DELAY) {
        m_state = 16;
        m_cnt = 0;
      }
      break;

    case 16:
    case 8:
    case 4:
    case 2:
      if (m_cnt >= KEY_REPEAT_TRIGGER) { //3 6 12 24 48 pulses in every 480ms
        m_state >>= 1;
        m_cnt = 0;
      }
      // no break
    case 1:
      if ((m_cnt & (m_state - 1)) == 0) {
        // this produces repeat events that at first repeat slowly and then increase in speed
        evt = _MSK_KEY_REPT;
      }
      break;

    case KSTATE_PAUSE: //pause repeat events
      if (m_cnt >= KEY_REPEAT_PAUSE_DELAY) {
        m_state = 8;
        m_cnt = 0;
      }
      break;

    case KSTATE_KILLED: //killed
      break;
  }

  return evt;
}

void Key::pauseEvents()
{
  m_state = KSTATE_PAUSE;
  m_cnt = 0;
}

void Key::killEvents()
{
  // TRACE("key %d killed", key());
  m_state = KSTATE_KILLED;
}

static Key keys[MAX_KEYS];
static Key trim_keys[MAX_TRIMS * 2];

/**
 * @brief returns true if there is an event waiting.
 * 
 */
bool isEvent()
{
  return s_evt != 0;
}

void pushEvent(event_t evt)
{
  s_evt = evt;
}

event_t getEvent()
{
  auto event = s_evt;
  s_evt = 0;
  return event;
}

void pushTrimEvent(event_t evt)
{
  s_trim_evt = evt;
}

event_t getTrimEvent()
{
  auto event = s_trim_evt;
  s_trim_evt = 0;
  return event;
}

// Introduce a slight delay in the key repeat sequence
void pauseEvents(event_t event)
{
  event = EVT_KEY_MASK(event);
  if (event < (int)DIM(keys)) keys[event].pauseEvents();
}

void pauseTrimEvents(event_t event)
{
  event = EVT_KEY_MASK(event);
  if (event < (int)DIM(trim_keys)) trim_keys[event].pauseEvents();
}

// Disables any further event generation (BREAK and REPEAT) for this key,
// until the key is released
void killEvents(event_t event)
{
  event = EVT_KEY_MASK(event);
  if (event < (int)DIM(keys)) {
    keys[event].killEvents();
  }
}

void killTrimEvents(event_t event)
{
  event = EVT_KEY_MASK(event);
  if (event < (int)DIM(trim_keys)) {
    trim_keys[event].killEvents();
  }
}

void killAllEvents()
{
  for (uint8_t key = 0; key < DIM(keys); key++) {
    keys[key].killEvents();
  }
}

bool waitKeysReleased()
{
  // loop until all keys are up
#if !defined(BOOT)
  tmr10ms_t start = get_tmr10ms();
#endif

  while (keyDown()) {
    WDG_RESET();

#if !defined(BOOT)
    if ((get_tmr10ms() - start) >= 300) {  // wait no more than 3 seconds
      //timeout expired, at least one key stuck
      return false;
    }
#endif
  }

  memclear(keys, sizeof(keys));
  pushEvent(0);
  return true;
}

bool keyDown()
{
  return readKeys() || readTrims();
}

bool trimDown(uint8_t idx)
{
  return readTrims() & (1 << idx);
}

uint8_t keysGetState(uint8_t key)
{
  if (key >= MAX_KEYS) return 0;
  return keys[key].pressed();
}

uint8_t keysGetTrimState(uint8_t trim)
{
  if (trim >= keysGetMaxTrims() * 2) return 0;
  return trim_keys[trim].pressed();
}

#if defined(USE_TRIMS_AS_BUTTONS)
static bool _trims_as_buttons = false;

void setTrimsAsButtons(bool val) { _trims_as_buttons = val; }
bool getTrimsAsButtons() { return _trims_as_buttons; }

static uint32_t transpose_trims()
{
  uint32_t keys = 0;
  auto trims = readTrims();

  if (trims & (1 << 0)) keys |= 1 << KEY_SYS;
  if (trims & (1 << 1)) keys |= 1 << KEY_TELE;
  if (trims & (1 << 2)) keys |= 1 << KEY_PAGEUP;
  if (trims & (1 << 3)) keys |= 1 << KEY_PAGEDN;
  if (trims & (1 << 4)) keys |= 1 << KEY_DOWN;
  if (trims & (1 << 5)) keys |= 1 << KEY_UP;
  if (trims & (1 << 6)) keys |= 1 << KEY_LEFT;
  if (trims & (1 << 7)) keys |= 1 << KEY_RIGHT;

  return keys;
}
#endif

bool keysPollingCycle()
{
  uint32_t i;

  uint32_t trims_input;
  uint32_t keys_input = readKeys();

#if defined(USE_TRIMS_AS_BUTTONS)
  if (getTrimsAsButtons()) {
    keys_input |= transpose_trims();
    trims_input = 0;
  } else {
    trims_input = readTrims();
  }
#else
  trims_input = readTrims();
#endif

  for (i = 0; i < MAX_KEYS; i++) {
    event_t evt = keys[i].input(keys_input & (1 << i));
    if (evt) {
      if (i != KEY_SHIFT && evt != _MSK_KEY_REPT) {
        pushEvent(evt | i);
      }
    }
  }

  for (i = 0; i < MAX_TRIMS * 2; i++) {
    event_t evt = trim_keys[i].input(trims_input & (1 << i));
    if (evt) pushTrimEvent(evt | i);
  }

  return keys_input || trims_input;
}

#if !defined(COLORLCD)

#define ROTENC_DELAY_MIDSPEED  32
#define ROTENC_DELAY_HIGHSPEED 16

int8_t rotencSpeed = ROTENC_LOWSPEED;

int8_t rotaryEncoderGetAccel()
{
  return rotencSpeed;
}

void rotaryEncoderResetAccel()
{
  rotencSpeed = ROTENC_LOWSPEED;
}

bool rotaryEncoderPollingCycle()
{
  static rotenc_t rePreviousValue;
  static bool cw = false;
  rotenc_t reNewValue = rotaryEncoderGetValue();
  rotenc_t scrollRE = reNewValue - rePreviousValue;
  if (scrollRE) {
    static uint32_t lastEvent;
    rePreviousValue = reNewValue;

    bool new_cw = (scrollRE < 0) ? false : true;
    if ((g_tmr10ms - lastEvent >= 10) || (cw == new_cw)) {  // 100ms

      pushEvent(new_cw ? EVT_ROTARY_RIGHT : EVT_ROTARY_LEFT);

      // rotary encoder navigation speed (acceleration) detection/calculation
      static uint32_t delay = 2 * ROTENC_DELAY_MIDSPEED;

      if (new_cw == cw) {
        // Modified moving average filter used for smoother change of speed
        delay = (((g_tmr10ms - lastEvent) << 3) + delay) >> 1;
      } else {
        delay = 2 * ROTENC_DELAY_MIDSPEED;
      }

      if (delay < ROTENC_DELAY_HIGHSPEED)
        rotencSpeed = ROTENC_HIGHSPEED;
      else if (delay < ROTENC_DELAY_MIDSPEED)
        rotencSpeed = ROTENC_MIDSPEED;
      else
        rotencSpeed = ROTENC_LOWSPEED;
      cw = new_cw;
      lastEvent = g_tmr10ms;
    }

    return true;
  }

  return false;
}

#elif !defined(COLORLCD)

int8_t rotaryEncoderGetAccel() { return ROTENC_LOWSPEED; }
void rotaryEncoderResetAccel() {}

#endif
