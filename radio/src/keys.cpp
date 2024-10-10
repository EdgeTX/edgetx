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

#include "edgetx_helpers.h"
#include "definitions.h"

#include "timers_driver.h"
#include "hal/watchdog_driver.h"
#include "hal/rotary_encoder.h"
#include "dataconstants.h"

#if !defined(BOOT) && (defined(USE_HATS_AS_KEYS) || defined(PCBXLITE))
#include "edgetx.h"
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

#define KFLAG_KILLED                1
#define KFLAG_LONG_PRESS            2

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
  uint8_t m_flags = 0;

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

  if ((m_state || m_flags) && m_vals == 0) {
    // key is released
#if defined(COLORLCD)
    if ((m_flags & (KFLAG_KILLED)) == 0) {
      evt = (m_flags & KFLAG_LONG_PRESS) ? _MSK_KEY_LONG_BRK : _MSK_KEY_BREAK;
    }
#else
    if ((m_flags & (KFLAG_KILLED)) == 0) {
      evt = _MSK_KEY_BREAK;
    }
#endif
    m_state = KSTATE_OFF;
    m_cnt = 0;
    m_flags = 0;
    return evt;
  }

  if (m_flags & KFLAG_KILLED) return evt;

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
        m_flags |= KFLAG_LONG_PRESS;
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
  if (m_state)
    m_flags |= KFLAG_KILLED;
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

#if defined(PCBXLITE) && !defined(BOOT)
uint32_t _readTrims()
{
  uint32_t trims = readTrims();

  uint8_t lr = trims & 0x3;
  uint8_t ud = trims & 0xc;
  bool shift = readKeys() & (1 << KEY_SHIFT);
  // Mode 1 or 2 - AIL on right stick
  bool ailRight = g_eeGeneral.stickMode < 2;
  // Mode 2 or 4 - ELE on right stick
  bool eleRight = (g_eeGeneral.stickMode & 1) == 1;
  // Ensure non-shifted trims are AIL and ELE
  if (ailRight == !shift) lr <<= 6;
  if (eleRight == !shift) ud <<= 2;

  return lr | ud;
}

#define READ_TRIMS() _readTrims()
#else
#define READ_TRIMS() readTrims()
#endif

bool keyDown()
{
  return readKeys() || readTrims();
}

bool trimDown(uint8_t idx)
{
  return READ_TRIMS() & (1 << idx);
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

#if defined(USE_HATS_AS_KEYS)
#define ROTARY_EMU_KEY_REPEAT_RATE 12  // times 10 [ms]

static bool _trims_as_buttons = false;
static bool _trims_as_buttons_LUA = false;

void setHatsAsKeys(bool val) { _trims_as_buttons = val; }
bool getHatsAsKeys() { return _trims_as_buttons; }

void setTransposeHatsForLUA(bool val) { _trims_as_buttons_LUA = val; }
bool getTransposeHatsForLUA() { return _trims_as_buttons_LUA; }

int16_t getEmuRotaryData()
{
  static bool rotaryTrimPressed = false;
  static tmr10ms_t timePressed = 0;

  if (getHatsAsKeys() || getTransposeHatsForLUA()) {
    tmr10ms_t now = get_tmr10ms();

    if (rotaryTrimPressed) {
      if (now < (timePressed + ROTARY_EMU_KEY_REPEAT_RATE)) return 0;

      rotaryTrimPressed = false;
    }

    auto trims = readTrims();

    if (trims & (1 << 4)) {
      rotaryTrimPressed = true;
      timePressed = now;
      return 1;
    }

    if (trims & (1 << 5)) {
      rotaryTrimPressed = true;
      timePressed = now;
      return -1;
    }
  }

  return 0;
}

static void transpose_trims(uint32_t *keys)
{
#if defined(BOOT)
  auto trims = readTrims();

  if (trims & (1 << 4)) *keys |= 1 << KEY_DOWN;  // right hat, down    0x10
  if (trims & (1 << 5)) *keys |= 1 << KEY_UP;    // right hat, up      0x20
#else
  static uint8_t state = 0;

  bool allowModeSwitch = ((g_model.hatsMode == HATSMODE_GLOBAL &&
                           g_eeGeneral.hatsMode == HATSMODE_SWITCHABLE) ||
                          (g_model.hatsMode == HATSMODE_SWITCHABLE)) &&
                         !getTransposeHatsForLUA();

  if (allowModeSwitch) {
    static bool lastExitState = false;
    static bool lastEnterState = false;

    bool exitState =
        *keys & (1 << KEY_EXIT);  // edge detection for EXIT and ENTER keys
    bool enterState = *keys & (1 << KEY_ENTER);

    bool exitPressed = !lastExitState && exitState;
    bool exitReleased = lastExitState && !exitState;
    bool enterPressed = !lastEnterState && enterState;

    lastExitState = exitState;
    lastEnterState = enterState;

    switch (state) {
      case 0:  // idle state waiting for EXIT or ENTER key
        if (exitPressed) {
          state = 1;
        }
        break;

      case 1:                // state EXIT received
        if (exitReleased) {  // if exit released go back to idle state
          state = 0;
          break;
        }

        if (enterPressed) {  // ENTER received with EXIT still pressed
          setHatsAsKeys(!getHatsAsKeys());  // change mode and don't forward
                                            // EXIT and ENTER keys
          killEvents(KEY_EXIT);
          killEvents(KEY_ENTER);
          state = 0;  // go to for EXIT to be released
          break;
        }
        break;
    }
  } else
    state = 0;  // state machine in idle if not in mode "BOTH"

  if (getHatsAsKeys() ||  // map hats to keys in button mode or LUA active
      getTransposeHatsForLUA()) {
    auto trims = readTrims();

    // spare key in buttons mode: left hat left
    // if (trims & (1 << 0)) *keys |= 1 << tbd;      // left hat, left    0x01
    if (trims & (1 << 1)) *keys |= 1 << KEY_MODEL;  // left hat, right   0x02
    if (trims & (1 << 2)) *keys |= 1 << KEY_TELE;   // left hat, down    0x04
    if (trims & (1 << 3)) *keys |= 1 << KEY_SYS;    // left hat, up      0x08

    if (trims & (1 << 6)) *keys |= 1 << KEY_PAGEUP;  // rht, left    0x40
    if (trims & (1 << 7)) *keys |= 1 << KEY_PAGEDN;  // rht, right   0x80
  }

#endif
}
#endif

bool keysPollingCycle()
{
  uint32_t trims_input;
  uint32_t keys_input = readKeys();

#if defined(USE_HATS_AS_KEYS)
  transpose_trims(&keys_input);

  if (getHatsAsKeys() || getTransposeHatsForLUA()) {
    trims_input = 0;
  } else {
    trims_input = readTrims();
  }
#else
  trims_input = READ_TRIMS();
#endif

  for (int i = 0; i < MAX_KEYS; i++) {
    event_t evt = keys[i].input(keys_input & (1 << i));
    if (evt) {
      evt |= i;
#if defined(KEYS_GPIO_REG_PAGEDN) && !defined(KEYS_GPIO_REG_PAGEUP)
      // Radio with single PAGEDN key
      if (evt == EVT_KEY_LONG(KEY_PAGEDN)) {
        // Convert long press PAGEDN to short press PAGEUP
        evt = EVT_KEY_BREAK(KEY_PAGEUP);
        killEvents(KEY_PAGEDN);
      }
#endif
#if defined(KEYS_GPIO_REG_SHIFT)
      // SHIFT key should not trigger REPT events
      if (evt != EVT_KEY_REPT(KEY_SHIFT)) {
        pushEvent(evt);
      }
#else
      pushEvent(evt);
#endif
    }
  }

  auto trim_switches = keysGetMaxTrims() * 2;
  for (int i = 0; i < trim_switches; i++) {
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
