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

#ifndef _KEYS_H_
#define _KEYS_H_

#include <inttypes.h>
#include "board.h"
#include "opentx_types.h"
#include "libopenui/src/libopenui_types.h"

constexpr event_t EVT_REFRESH =        0x1000;
constexpr event_t EVT_ENTRY =          0x1001;
constexpr event_t EVT_ENTRY_UP =       0x1002;
constexpr event_t EVT_ROTARY_LEFT =    0x1003;
constexpr event_t EVT_ROTARY_RIGHT =   0x1004;

#if defined(PCBHORUS) || defined (PCBNV14) || defined(PCBPL18)
constexpr event_t _MSK_KEY_BREAK =     0x0200;
constexpr event_t _MSK_KEY_REPT =      0x0400;
constexpr event_t _MSK_KEY_FIRST =     0x0600;
constexpr event_t _MSK_KEY_LONG =      0x0800;
constexpr event_t _MSK_KEY_FLAGS =     0x0E00;
#else
constexpr event_t _MSK_KEY_BREAK =     0x0020;
constexpr event_t _MSK_KEY_REPT =      0x0040;
constexpr event_t _MSK_KEY_FIRST =     0x0060;
constexpr event_t _MSK_KEY_LONG =      0x0080;
constexpr event_t _MSK_KEY_FLAGS =     0x00E0;
#endif

#if defined(HARDWARE_TOUCH)
constexpr event_t _MSK_VIRTUAL_KEY =   0x2000;

constexpr event_t EVT_VIRTUAL_KEY(uint8_t key)
{
  return (key | _MSK_VIRTUAL_KEY);
}

constexpr event_t EVT_VIRTUAL_KEY_MIN = EVT_VIRTUAL_KEY('m');
constexpr event_t EVT_VIRTUAL_KEY_MAX = EVT_VIRTUAL_KEY('M');
constexpr event_t EVT_VIRTUAL_KEY_PLUS = EVT_VIRTUAL_KEY('+');
constexpr event_t EVT_VIRTUAL_KEY_MINUS = EVT_VIRTUAL_KEY('-');
constexpr event_t EVT_VIRTUAL_KEY_FORWARD = EVT_VIRTUAL_KEY('F');
constexpr event_t EVT_VIRTUAL_KEY_BACKWARD = EVT_VIRTUAL_KEY('B');
constexpr event_t EVT_VIRTUAL_KEY_DEFAULT = EVT_VIRTUAL_KEY('0');
constexpr event_t EVT_VIRTUAL_KEY_UP = EVT_VIRTUAL_KEY('U');
constexpr event_t EVT_VIRTUAL_KEY_DOWN = EVT_VIRTUAL_KEY('D');
constexpr event_t EVT_VIRTUAL_KEY_LEFT = EVT_VIRTUAL_KEY('L');
constexpr event_t EVT_VIRTUAL_KEY_RIGHT = EVT_VIRTUAL_KEY('R');
constexpr event_t EVT_VIRTUAL_KEY_NEXT = EVT_VIRTUAL_KEY('N');
constexpr event_t EVT_VIRTUAL_KEY_PREVIOUS = EVT_VIRTUAL_KEY('P');
constexpr event_t EVT_VIRTUAL_KEY_SIGN = EVT_VIRTUAL_KEY(177);

constexpr bool IS_VIRTUAL_KEY_EVENT(event_t event)
{
  return (event & 0xF000) == _MSK_VIRTUAL_KEY;
}

// Touch events for Lua widgets - assuming _MSK_KEY_* for PCBHORUS
constexpr event_t EVT_TOUCH = EVT_VIRTUAL_KEY('t');
constexpr event_t EVT_TOUCH_FIRST = EVT_TOUCH | _MSK_KEY_FIRST;
constexpr event_t EVT_TOUCH_BREAK = EVT_TOUCH | _MSK_KEY_BREAK;
constexpr event_t EVT_TOUCH_SLIDE = EVT_TOUCH | _MSK_KEY_LONG;
constexpr event_t EVT_TOUCH_TAP   = EVT_TOUCH | _MSK_KEY_REPT;

constexpr bool IS_TOUCH_EVENT(event_t event)
{
  return (event & ~_MSK_KEY_FLAGS) == EVT_TOUCH;
}
#endif

// normal order of events is: FIRST, LONG, REPEAT, REPEAT, ..., BREAK
#define EVT_KEY_MASK(e)                ((e) & 0x1F)

constexpr event_t EVT_KEY_FIRST(uint8_t key)
{
  return (key | _MSK_KEY_FIRST);  // fired when key is pressed
}

constexpr event_t EVT_KEY_REPT(uint8_t key)
{
  return (key | _MSK_KEY_REPT);  // fired when key is held pressed long enough, fires multiple times with increasing speed
}

constexpr event_t EVT_KEY_LONG(uint8_t key)
{
  return (key | _MSK_KEY_LONG);  // fired when key is held pressed for a while
}

constexpr event_t EVT_KEY_BREAK(uint8_t key)
{
  return (key | _MSK_KEY_BREAK);  // fired when key is released (short or long), but only if the event was not killed
}

constexpr bool IS_KEY_EVENT(event_t event)
{
  return (event & 0xF000) == 0;  // fired when key is released (short or long), but only if the event was not killed
}

constexpr bool IS_TRIM_EVENT(event_t event)
{
  return (IS_KEY_EVENT(event) && EVT_KEY_MASK(event) >= TRM_BASE);
}

inline bool IS_KEY_FIRST(event_t evt)
{
  return (evt & _MSK_KEY_FLAGS) == _MSK_KEY_FIRST;
}

inline bool IS_KEY_REPT(event_t evt)
{
  return (evt & _MSK_KEY_FLAGS) == _MSK_KEY_REPT;
}

inline bool IS_KEY_LONG(event_t evt)
{
  return (evt & _MSK_KEY_FLAGS) == _MSK_KEY_LONG;
}

inline bool IS_KEY_BREAK(event_t evt)
{
  return (evt & _MSK_KEY_FLAGS) == _MSK_KEY_BREAK;
}

inline bool IS_KEY_EVT(event_t evt, uint8_t key)
{
  return (evt & _MSK_KEY_FLAGS) && (EVT_KEY_MASK(evt) == key);
}

#if defined(PCBXLITE)
  #define EVT_ROTARY_BREAK             EVT_KEY_BREAK(KEY_ENTER)
  #define EVT_ROTARY_LONG              EVT_KEY_LONG(KEY_ENTER)
  #define IS_NEXT_EVENT(event)         (event==EVT_KEY_FIRST(KEY_DOWN) || event==EVT_KEY_REPT(KEY_DOWN))
  #define IS_PREVIOUS_EVENT(event)     (event==EVT_KEY_FIRST(KEY_UP) || event==EVT_KEY_REPT(KEY_UP))
#elif defined(RADIO_T8)
  #define EVT_ROTARY_BREAK             EVT_KEY_BREAK(KEY_ENTER)
  #define EVT_ROTARY_LONG              EVT_KEY_LONG(KEY_ENTER)
  #define IS_NEXT_EVENT(event)         (event==EVT_KEY_FIRST(KEY_DOWN) || event==EVT_KEY_REPT(KEY_DOWN))
  #define IS_PREVIOUS_EVENT(event)     (event==EVT_KEY_FIRST(KEY_UP) || event==EVT_KEY_REPT(KEY_UP))
#elif defined(PCBFRSKY) && defined(ROTARY_ENCODER_NAVIGATION)
  #define EVT_ROTARY_BREAK             EVT_KEY_BREAK(KEY_ENTER)
  #define EVT_ROTARY_LONG              EVT_KEY_LONG(KEY_ENTER)
  #define IS_NEXT_EVENT(event)         (event==EVT_ROTARY_RIGHT)
  #define IS_PREVIOUS_EVENT(event)     (event==EVT_ROTARY_LEFT)
#elif defined(ROTARY_ENCODER_NAVIGATION)
  #define EVT_ROTARY_BREAK             0xcf
  #define EVT_ROTARY_LONG              0xce
  #define IS_NEXT_EVENT(event)         (event==EVT_ROTARY_RIGHT || event==EVT_KEY_FIRST(KEY_DOWN) || event==EVT_KEY_REPT(KEY_DOWN))
  #define IS_PREVIOUS_EVENT(event)     (event==EVT_ROTARY_LEFT || event==EVT_KEY_FIRST(KEY_UP) || event==EVT_KEY_REPT(KEY_UP))
#else
  #define IS_NEXT_EVENT(event)         (event==EVT_KEY_FIRST(KEY_DOWN) || event==EVT_KEY_REPT(KEY_DOWN))
  #define IS_PREVIOUS_EVENT(event)     (event==EVT_KEY_FIRST(KEY_UP) || event==EVT_KEY_REPT(KEY_UP))
#endif

class Key
{
  private:
    uint8_t m_vals  = 0;
    uint8_t m_cnt   = 0;
    uint8_t m_state = 0;
  public:
    void input(bool val);
    bool state() const { return m_vals > 0; }
    void pauseEvents();
    void killEvents();
    uint8_t key() const;
};

extern Key keys[NUM_KEYS];
extern event_t s_evt;

inline void pushEvent(event_t evt)
{
  s_evt = evt;
}

void pauseEvents(event_t event);
void killEvents(event_t event);
void killAllEvents();
bool waitKeysReleased();
event_t getEvent(bool trim=false);
bool isEvent();
bool keyDown();

#if defined(ROTARY_ENCODER_NAVIGATION)
extern uint8_t rotencSpeed;
#endif

struct InactivityData
{
  uint16_t counter;
  uint8_t  sum;
};

extern InactivityData inactivity;

#endif // _KEYS_H_
