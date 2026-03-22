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
#include "navigation.h"

#include "hal/switch_driver.h"
#include "switches.h"

#if defined(AUTOSWITCH)
swsrc_t checkIncDecMovedSwitch(swsrc_t val)
{
  SwitchRef moved = getMovedSwitch();
  if (!moved.isNone()) {
    swsrc_t swtch = switchRefToSwSrc(moved);
    if (moved.type == SWITCH_TYPE_SWITCH) {
      div_t info = switchInfo(swtch);
      if (IS_CONFIG_TOGGLE(info.quot)) {
        if (info.rem != 0) {
          val = (val == swtch ? swtch-2 : swtch);
        }
      }
      else {
        val = swtch;
      }
    }
    else {
      val = swtch;
    }
  }
  return val;
}
#endif

void repeatLastCursorMove(event_t event)
{
  if (IS_PREVIOUS_MOVE_EVENT(event) || IS_NEXT_MOVE_EVENT(event)) {
    pushEvent(event);
  }
  else {
    menuHorizontalPosition = 0;
  }
}

#if defined(NAVIGATION_9X) || defined(NAVIGATION_XLITE)
inline bool IS_NEXT_HOR_MOVE_EVENT(event_t evt)
{
  return evt == EVT_KEY_FIRST(KEY_RIGHT) || evt == EVT_KEY_REPT(KEY_RIGHT) ||
         evt == EVT_ROTARY_RIGHT;
}

inline bool IS_PREVIOUS_HOR_MOVE_EVENT(event_t evt)
{
  return evt == EVT_KEY_FIRST(KEY_LEFT) || evt == EVT_KEY_REPT(KEY_LEFT) ||
         evt == EVT_ROTARY_LEFT;
}

void repeatLastCursorHorMove(event_t event)
{
  if (IS_PREVIOUS_HOR_MOVE_EVENT(event) || IS_NEXT_HOR_MOVE_EVENT(event)) {
    pushEvent(event);
  }
  else {
    menuHorizontalPosition = 0;
  }
}
#endif

void check_submenu_simple(event_t event, uint8_t rowcount)
{
  check_simple(event, 0, nullptr, 0, rowcount);
}

void check_simple(event_t event, uint8_t curr, const MenuHandler *menuTab, uint8_t menuTabSize, vertpos_t rowcount)
{
  check(event, curr, menuTab, menuTabSize, nullptr, 0, rowcount);
}
