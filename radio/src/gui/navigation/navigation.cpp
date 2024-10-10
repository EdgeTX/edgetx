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

#include "switches.h"

vertpos_t menuVerticalOffset;
vertpos_t menuVerticalPosition;
horzpos_t menuHorizontalPosition;

int8_t s_editMode;
uint8_t noHighlightCounter;
uint8_t menuCalibrationState;

int8_t  checkIncDec_Ret;

tmr10ms_t menuEntryTime;

extern int checkIncDecSelection;

INIT_STOPS(stops100, 3, -100, 0, 100)
INIT_STOPS(stops1000, 3, -1000, 0, 1000)
INIT_STOPS(stopsSwitch, 15, SWSRC_FIRST,
           CATEGORY_END(-SWSRC_FIRST_LOGICAL_SWITCH),
           CATEGORY_END(-SWSRC_FIRST_TRIM),
           CATEGORY_END(-SWSRC_LAST_SWITCH + 1), 0,
           CATEGORY_END(SWSRC_LAST_SWITCH), CATEGORY_END(SWSRC_FIRST_TRIM - 1),
           CATEGORY_END(SWSRC_FIRST_LOGICAL_SWITCH - 1), SWSRC_LAST)

#define INC(val, min, max)  if (val<max) {val++;} else {val=min;}
#define DEC(val, min, max)  if (val>min) {val--;} else {val=max;}

#define MAXCOL_RAW(row)     (horTab ? *(horTab+min(row, (vertpos_t)horTabMax)) : (const uint8_t)0)
#define MAXCOL(row)         (MAXCOL_RAW(row) >= HIDDEN_ROW ? MAXCOL_RAW(row) : (const uint8_t)(MAXCOL_RAW(row) & (~NAVIGATION_LINE_BY_LINE)))

#define CURSOR_NOT_ALLOWED_IN_ROW(row) ((int8_t)MAXCOL(row) < 0)

uint8_t chgMenu(uint8_t curr, const MenuHandler * menuTab, uint8_t menuTabSize, int direction)
{
  int cc = curr + direction;
  while (cc != curr) {
    if (cc < 0)
      cc = menuTabSize - 1;
    else if (cc >= menuTabSize)
      cc = 0;
    if (menuTab[cc].isEnabled())
      return cc;
    cc += direction;
  }
  return curr;
}

uint8_t menuSize(const MenuHandler * menuTab, uint8_t menuTabSize)
{
  uint8_t sz = 0;
  for (int i = 0; i < menuTabSize; i += 1) {
    if (menuTab[i].isEnabled()) {
      sz += 1;
    }
  }
  return sz;
}

uint8_t menuIdx(const MenuHandler * menuTab, uint8_t curr)
{
  return menuSize(menuTab, curr + 1) - 1;
}

void addPopupItem(int i_min, int i_max, int rangeMin, int rangeMax, IsValueAvailable isValueAvailable, const char* menuItem)
{
  if (i_min <= rangeMin && i_max >= rangeMin && getFirstAvailable(rangeMin, rangeMax, isValueAvailable) != MIXSRC_NONE) {
    POPUP_MENU_ADD_ITEM(menuItem);
  }
}

inline int showPopupMenus(event_t event, int newval, int i_min, int i_max,
                          unsigned int i_flags, IsValueAvailable isValueAvailable,
                          bool& isSource)
{
  if (i_flags & (INCDEC_SOURCE | INCDEC_SOURCE_VALUE)) {
    if (event == EVT_KEY_LONG(KEY_ENTER)) {
      checkIncDecSelection = MIXSRC_NONE;

      if (i_flags & INCDEC_SOURCE_VALUE && isSource) {
        POPUP_MENU_ADD_ITEM(STR_CONSTANT);
      }

      addPopupItem(i_min, i_max, MIXSRC_FIRST_INPUT, MIXSRC_LAST_INPUT, isValueAvailable, STR_MENU_INPUTS);
#if defined(LUA_MODEL_SCRIPTS)
      addPopupItem(i_min, i_max, MIXSRC_FIRST_LUA, MIXSRC_LAST_LUA, isValueAvailable, STR_MENU_LUA);
#endif
      if (i_min <= MIXSRC_FIRST_STICK && i_max >= MIXSRC_FIRST_STICK)      POPUP_MENU_ADD_ITEM(STR_MENU_STICKS);
      if (i_min <= MIXSRC_FIRST_POT && i_max >= MIXSRC_FIRST_POT)          POPUP_MENU_ADD_ITEM(STR_MENU_POTS);
      if (i_min <= MIXSRC_MIN && i_max >= MIXSRC_MIN)                      POPUP_MENU_ADD_ITEM(STR_MENU_MIN);
      if (i_min <= MIXSRC_MAX && i_max >= MIXSRC_MAX)                      POPUP_MENU_ADD_ITEM(STR_MENU_MAX);
#if defined(HELI)
      if (modelHeliEnabled())
        addPopupItem(i_min, i_max, MIXSRC_FIRST_HELI, MIXSRC_LAST_HELI, isValueAvailable, STR_MENU_HELI);
#endif
      if (i_min <= MIXSRC_FIRST_TRIM && i_max >= MIXSRC_FIRST_TRIM)        POPUP_MENU_ADD_ITEM(STR_MENU_TRIMS);
      if (i_min <= MIXSRC_FIRST_SWITCH && i_max >= MIXSRC_FIRST_SWITCH)    POPUP_MENU_ADD_ITEM(STR_MENU_SWITCHES);
      addPopupItem(i_min, i_max, MIXSRC_FIRST_TRAINER, MIXSRC_LAST_TRAINER, isValueAvailable, STR_MENU_TRAINER);
      if (i_min <= MIXSRC_FIRST_CH && i_max >= MIXSRC_FIRST_CH)            POPUP_MENU_ADD_ITEM(STR_MENU_CHANNELS);
      addPopupItem(i_min, i_max, MIXSRC_FIRST_GVAR, MIXSRC_LAST_GVAR, isValueAvailable, STR_MENU_GVARS);

      if (modelTelemetryEnabled() && i_min <= MIXSRC_FIRST_TELEM && i_max >= MIXSRC_FIRST_TELEM) {
        for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
          TelemetrySensor * sensor = & g_model.telemetrySensors[i];
          if (sensor->isAvailable()) {
            POPUP_MENU_ADD_ITEM(STR_MENU_TELEMETRY);
            break;
          }
        }
      }
      if (i_flags & INCDEC_SOURCE_INVERT) POPUP_MENU_ADD_ITEM(STR_MENU_INVERT);
      POPUP_MENU_START(onSourceLongEnterPress);
    }
    if (checkIncDecSelection != 0) {
      if (i_flags & INCDEC_SOURCE_VALUE) {
        if (checkIncDecSelection == MIXSRC_VALUE) {
          newval = 0;
          isSource = false;
        } else {
          newval = (checkIncDecSelection == MIXSRC_INVERT ? -newval : checkIncDecSelection);
          if (checkIncDecSelection != MIXSRC_INVERT)
            isSource = true;
        }
      } else {
        newval = (checkIncDecSelection == MIXSRC_INVERT ? -newval : checkIncDecSelection);
      }
      if (checkIncDecSelection != MIXSRC_MIN && checkIncDecSelection != MIXSRC_MAX)
        s_editMode = EDIT_MODIFY_FIELD;
      checkIncDecSelection = 0;
    }
  }
  else if (i_flags & INCDEC_SWITCH) {
    if (event == EVT_KEY_LONG(KEY_ENTER)) {
      checkIncDecSelection = SWSRC_NONE;
      if (i_min <= SWSRC_FIRST_SWITCH && i_max >= SWSRC_LAST_SWITCH)       POPUP_MENU_ADD_ITEM(STR_MENU_SWITCHES);
      if (i_min <= SWSRC_FIRST_TRIM && i_max >= SWSRC_LAST_TRIM)           POPUP_MENU_ADD_ITEM(STR_MENU_TRIMS);
      if (i_min <= SWSRC_FIRST_LOGICAL_SWITCH && i_max >= SWSRC_LAST_LOGICAL_SWITCH) {
        for (int i = 0; i < MAX_LOGICAL_SWITCHES; i++) {
          if (isValueAvailable && isValueAvailable(SWSRC_FIRST_LOGICAL_SWITCH+i)) {
            POPUP_MENU_ADD_ITEM(STR_MENU_LOGICAL_SWITCHES);
            break;
          }
        }
      }
      if (isValueAvailable && isValueAvailable(SWSRC_ON))                  POPUP_MENU_ADD_ITEM(STR_MENU_OTHER);
      if (isValueAvailable && isValueAvailable(-newval))                   POPUP_MENU_ADD_ITEM(STR_MENU_INVERT);
      POPUP_MENU_START(onSwitchLongEnterPress);
      s_editMode = EDIT_MODIFY_FIELD;
    }
    if (checkIncDecSelection != 0) {
      newval = (checkIncDecSelection == SWSRC_INVERT ? -newval : checkIncDecSelection);
      s_editMode = EDIT_MODIFY_FIELD;
      checkIncDecSelection = 0;
    }
  }

  return newval;
}

int checkMovedInput(int newval, unsigned int i_flags, bool isSource)
{
#if defined(AUTOSWITCH)
  if (i_flags & INCDEC_SWITCH) {
    newval = checkIncDecMovedSwitch(newval);
  }
#endif

#if defined(AUTOSOURCE)
    if (i_flags & (INCDEC_SOURCE|INCDEC_SOURCE_VALUE)) {
      int source = getMovedSource(MIXSRC_FIRST_STICK);
      if (source) {
        if (i_flags & INCDEC_SOURCE_VALUE) {
          if (isSource) {
            // Only use moved source if already a source value
            newval = source;
          }
        } else {
          newval = source;
        }
      }
#if defined(AUTOSWITCH)
    else {
      uint8_t swtch = abs(getMovedSwitch());
      if (swtch && !IS_SWITCH_MULTIPOS(swtch)) {
        newval = switchToMix(swtch);
      }
    }
#endif
  }
#endif
  return newval;
}

int checkBoolean(event_t event, int i_min, int i_max, int newval, int val)
{
  if (i_min == 0 && i_max == 1 &&
      (event == EVT_KEY_BREAK(KEY_ENTER))) {
    s_editMode = 0;
    newval = !val;
  }

  return newval;
}

void finishCheckIncDec(event_t event, int i_min, int i_max,
                      unsigned int i_flags, int newval, int val,
                      const CheckIncDecStops &stops)
{
  if (newval != val) {
#if !defined(ROTARY_ENCODER_NAVIGATION)
    if (!(i_flags & NO_INCDEC_MARKS) && (newval != i_max) &&
        (newval != i_min) && stops.contains(newval)) {
      bool pause = (newval > val ? !stops.contains(newval + 1)
                                 : !stops.contains(newval - 1));
      if (pause) {
        pauseEvents(event);  // delay before auto-repeat continues
      }
    }
    if (!IS_KEY_REPT(event)) {
      AUDIO_KEY_PRESS();
    }
#endif
    storageDirty(i_flags & (EE_GENERAL|EE_MODEL));
    checkIncDec_Ret = (newval > val ? 1 : -1);
  }
  else {
    checkIncDec_Ret = 0;
  }
}

#if defined(NAVIGATION_X7) || defined(NAVIGATION_X9D)
  #include "navigation_x7.cpp"
#elif defined(NAVIGATION_9X) || defined(NAVIGATION_XLITE)
  #include "navigation_9x.cpp"
#else
  static_assert("navigation not defined");
#endif
