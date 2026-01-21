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
  swsrc_t swtch = getMovedSwitch();
  if (swtch) {
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
  return val;
}
#endif

int checkIncDecSelection = 0;

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

void onSwitchLongEnterPress(const char * result)
{
  if (result == STR_MENU_SWITCHES) {
    checkIncDecSelection = SWSRC_FIRST_SWITCH;
  } else if (result == STR_MENU_TRIMS) {
    checkIncDecSelection = SWSRC_FIRST_TRIM;
  } else if (result == STR_MENU_LOGICAL_SWITCHES) {
    checkIncDecSelection =
        SWSRC_FIRST_LOGICAL_SWITCH +
        getFirstAvailable(0, MAX_LOGICAL_SWITCHES, isLogicalSwitchAvailable);
  } else if (result == STR_MENU_OTHER) {
    checkIncDecSelection = SWSRC_ON;
  } else if (result == STR_MENU_INVERT) {
    checkIncDecSelection = SWSRC_INVERT;
  }
}

void onSourceLongEnterPress(const char * result)
{
  if (result == STR_MENU_INPUTS) {
    checkIncDecSelection =
        getFirstAvailable(MIXSRC_FIRST_INPUT, MIXSRC_LAST_INPUT,
                          isInputAvailable);
  }
#if defined(LUA_MODEL_SCRIPTS)
  else if (result == STR_MENU_LUA) {
    checkIncDecSelection =
        getFirstAvailable(MIXSRC_FIRST_LUA, MIXSRC_LAST_LUA, isSourceAvailable);
  }
#endif
  else if (result == STR_MENU_STICKS)
    checkIncDecSelection = MIXSRC_FIRST_STICK;
  else if (result == STR_MENU_POTS)
    checkIncDecSelection = MIXSRC_FIRST_POT;
  else if (result == STR_MENU_MIN)
    checkIncDecSelection = MIXSRC_MIN;
  else if (result == STR_MENU_MAX)
    checkIncDecSelection = MIXSRC_MAX;
  else if (result == STR_MENU_HELI)
    checkIncDecSelection = MIXSRC_FIRST_HELI;
  else if (result == STR_MENU_TRIMS)
    checkIncDecSelection = MIXSRC_FIRST_TRIM;
  else if (result == STR_MENU_SWITCHES)
    checkIncDecSelection = MIXSRC_FIRST_SWITCH;
  else if (result == STR_MENU_TRAINER)
    checkIncDecSelection = MIXSRC_FIRST_TRAINER;
  else if (result == STR_MENU_CHANNELS)
    checkIncDecSelection = getFirstAvailable(MIXSRC_FIRST_CH, MIXSRC_LAST_CH, isSourceAvailable);
  else if (result == STR_MENU_GVARS)
    checkIncDecSelection = MIXSRC_FIRST_GVAR;
  else if (result == STR_MENU_TELEMETRY) {
    for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
      TelemetrySensor * sensor = & g_model.telemetrySensors[i];
      if (sensor->isAvailable()) {
        checkIncDecSelection = MIXSRC_FIRST_TELEM + 3*i;
        break;
      }
    }
  } else if (result == STR_MENU_INVERT) {
    checkIncDecSelection = MIXSRC_INVERT;
  } else if (result == STR_CONSTANT) {
    checkIncDecSelection = MIXSRC_VALUE;
  }
}

void check_submenu_simple(event_t event, uint8_t rowcount)
{
  check_simple(event, 0, nullptr, 0, rowcount);
}

void check_simple(event_t event, uint8_t curr, const MenuHandler *menuTab, uint8_t menuTabSize, vertpos_t rowcount)
{
  check(event, curr, menuTab, menuTabSize, nullptr, 0, rowcount);
}
