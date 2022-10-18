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

#include "opentx.h"
#include "switches.h"

vertpos_t menuVerticalOffset;
vertpos_t menuVerticalPosition;
horzpos_t menuHorizontalPosition;

int8_t s_editMode;
uint8_t noHighlightCounter;
uint8_t menuCalibrationState;

int8_t  checkIncDec_Ret;

INIT_STOPS(stops100, 3, -100, 0, 100)
INIT_STOPS(stops1000, 3, -1000, 0, 1000)
INIT_STOPS(stopsSwitch, 15, SWSRC_FIRST,
           CATEGORY_END(-SWSRC_FIRST_LOGICAL_SWITCH),
           CATEGORY_END(-SWSRC_FIRST_TRIM),
           CATEGORY_END(-SWSRC_LAST_SWITCH + 1), 0,
           CATEGORY_END(SWSRC_LAST_SWITCH), CATEGORY_END(SWSRC_FIRST_TRIM - 1),
           CATEGORY_END(SWSRC_FIRST_LOGICAL_SWITCH - 1), SWSRC_LAST)

extern int checkIncDecSelection;

int checkIncDec(event_t event, int val, int i_min, int i_max,
                unsigned int i_flags, IsValueAvailable isValueAvailable,
                const CheckIncDecStops &stops)
{
  int newval = val;

  if (s_editMode > 0) {
    if (event == EVT_KEY_FIRST(KEY_RIGHT) || event == EVT_KEY_REPT(KEY_RIGHT) ||
        event == EVT_KEY_FIRST(KEY_UP) || event == EVT_KEY_REPT(KEY_UP)) {
      do {
        if (IS_KEY_REPT(event) && (i_flags & INCDEC_REP10)) {
          newval += min(10, i_max-val);
        }
        else {
          newval++;
        }
      } while (isValueAvailable && !isValueAvailable(newval) &&
               newval <= i_max);

      if (newval > i_max) {
        newval = val;
        killEvents(event);
        AUDIO_KEY_ERROR();
      }
    } else if (event == EVT_KEY_FIRST(KEY_LEFT) ||
               event == EVT_KEY_REPT(KEY_LEFT) ||
               event == EVT_KEY_FIRST(KEY_DOWN) ||
               event == EVT_KEY_REPT(KEY_DOWN)) {
      do {
        if (IS_KEY_REPT(event) && (i_flags & INCDEC_REP10)) {
          newval -= min(10, val-i_min);
        }
        else {
          newval--;
        }
      } while (isValueAvailable && !isValueAvailable(newval) && newval>=i_min);

      if (newval < i_min) {
        newval = val;
        killEvents(event);
        AUDIO_KEY_ERROR();
      }
    }

#if defined(AUTOSWITCH)
    if (i_flags & INCDEC_SWITCH) {
      newval = checkIncDecMovedSwitch(newval);
    }
#endif

#if defined(AUTOSOURCE)
    if (i_flags & INCDEC_SOURCE) {
      int8_t source = GET_MOVED_SOURCE(i_min, i_max);
      if (source) {
        newval = source;
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
  }

  if (!READ_ONLY() && i_min == 0 && i_max == 1 &&
      event == EVT_KEY_BREAK(KEY_ENTER)) {
    s_editMode = 0;
    newval = !val;
  }

  if (newval != val) {
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
    storageDirty(i_flags & (EE_GENERAL|EE_MODEL));
    checkIncDec_Ret = (newval > val ? 1 : -1);
  }
  else {
    checkIncDec_Ret = 0;
  }

  if (i_flags & INCDEC_SOURCE) {
    if (event == EVT_KEY_LONG(KEY_ENTER) && !keysGetState(KEY_SHIFT)) {
      killEvents(event);
      checkIncDecSelection = MIXSRC_NONE;

      if (i_min <= MIXSRC_FIRST_INPUT && i_max >= MIXSRC_FIRST_INPUT) {
        if (getFirstAvailable(MIXSRC_FIRST_INPUT, MIXSRC_LAST_INPUT, isInputAvailable) != MIXSRC_NONE) {
          POPUP_MENU_ADD_ITEM(STR_MENU_INPUTS);
        }
      }
#if defined(LUA_MODEL_SCRIPTS)
      if (i_min <= MIXSRC_FIRST_LUA && i_max >= MIXSRC_FIRST_LUA) {
        if (getFirstAvailable(MIXSRC_FIRST_LUA, MIXSRC_LAST_LUA, isSourceAvailable) != MIXSRC_NONE) {
          POPUP_MENU_ADD_ITEM(STR_MENU_LUA);
        }
      }
#endif
      if (i_min <= MIXSRC_FIRST_STICK && i_max >= MIXSRC_FIRST_STICK)      POPUP_MENU_ADD_ITEM(STR_MENU_STICKS);
      if (i_min <= MIXSRC_FIRST_POT && i_max >= MIXSRC_FIRST_POT)          POPUP_MENU_ADD_ITEM(STR_MENU_POTS);
      if (i_min <= MIXSRC_MAX && i_max >= MIXSRC_MAX)                      POPUP_MENU_ADD_ITEM(STR_MENU_MAX);
#if defined(HELI)
      if (i_min <= MIXSRC_FIRST_HELI && i_max >= MIXSRC_FIRST_HELI)        POPUP_MENU_ADD_ITEM(STR_MENU_HELI);
#endif
      if (i_min <= MIXSRC_FIRST_TRIM && i_max >= MIXSRC_FIRST_TRIM)        POPUP_MENU_ADD_ITEM(STR_MENU_TRIMS);
      if (i_min <= MIXSRC_FIRST_SWITCH && i_max >= MIXSRC_FIRST_SWITCH)    POPUP_MENU_ADD_ITEM(STR_MENU_SWITCHES);
      if (i_min <= MIXSRC_FIRST_TRAINER && i_max >= MIXSRC_FIRST_TRAINER)  POPUP_MENU_ADD_ITEM(STR_MENU_TRAINER);
      if (i_min <= MIXSRC_FIRST_CH && i_max >= MIXSRC_FIRST_CH)            POPUP_MENU_ADD_ITEM(STR_MENU_CHANNELS);
      if (i_min <= MIXSRC_FIRST_GVAR && i_max >= MIXSRC_FIRST_GVAR && isValueAvailable(MIXSRC_FIRST_GVAR)) {
        POPUP_MENU_ADD_ITEM(STR_MENU_GVARS);
      }

      if (i_min <= MIXSRC_FIRST_TELEM && i_max >= MIXSRC_FIRST_TELEM) {
        for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
          TelemetrySensor * sensor = & g_model.telemetrySensors[i];
          if (sensor->isAvailable()) {
            POPUP_MENU_ADD_ITEM(STR_MENU_TELEMETRY);
            break;
          }
        }
      }
      POPUP_MENU_START(onSourceLongEnterPress);
    }
    if (checkIncDecSelection != 0) {
      newval = checkIncDecSelection;
      if (checkIncDecSelection != MIXSRC_MAX)
        s_editMode = EDIT_MODIFY_FIELD;
      checkIncDecSelection = 0;
    }
  }
  else if (i_flags & INCDEC_SWITCH) {
    if (event == EVT_KEY_LONG(KEY_ENTER) && !keysGetState(KEY_SHIFT)) {
      killEvents(event);
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

#define SCROLL_POT1_TH 32

#define CURSOR_NOT_ALLOWED_IN_ROW(row) ((int8_t)MAXCOL(row) < 0)

#define INC(val, min, max)             if (val<max) {val++;} else {val=min;}
#define DEC(val, min, max)             if (val>min) {val--;} else {val=max;}

tmr10ms_t menuEntryTime;

#define MAXCOL_RAW(row) \
  (horTab ? *(horTab + min(row, (vertpos_t)horTabMax)) : (const uint8_t)0)

#define MAXCOL(row)              \
  (MAXCOL_RAW(row) >= HIDDEN_ROW \
       ? MAXCOL_RAW(row)         \
       : (const uint8_t)(MAXCOL_RAW(row) & (~NAVIGATION_LINE_BY_LINE)))

#define POS_HORZ_INIT(posVert) 0

void check(event_t event, uint8_t curr, const MenuHandlerFunc *menuTab,
           uint8_t menuTabSize, const uint8_t *horTab, uint8_t horTabMax,
           vertpos_t maxrow, uint8_t flags)
{
  vertpos_t l_posVert = menuVerticalPosition;
  horzpos_t l_posHorz = menuHorizontalPosition;

  uint8_t maxcol = MAXCOL(l_posVert);

  if (menuTab) {
    uint8_t attr = 0;

    if (l_posVert==0 && !menuCalibrationState) {
      attr = INVERS;

      int8_t cc = curr;

      switch (event) {
        case EVT_KEY_FIRST(KEY_LEFT):
          if (curr > 0)
            cc = curr - 1;
          else
            cc = menuTabSize-1;
          break;

        case EVT_KEY_FIRST(KEY_RIGHT):
          if (curr < (menuTabSize-1))
            cc = curr + 1;
          else
            cc = 0;
          break;
      }

      if (cc != curr) {
        chainMenu(menuTab[cc]);
      }
    }

    menuCalibrationState = 0;
    drawScreenIndex(curr, menuTabSize, attr);
  }

  switch (event) {
    case EVT_ENTRY:
      menuEntryTime = get_tmr10ms();
      l_posVert = 0;
      l_posHorz = POS_HORZ_INIT(l_posVert);
      s_editMode = EDIT_MODE_INIT;
      break;

    case EVT_KEY_FIRST(KEY_ENTER):
      if (!menuTab || l_posVert > 0) {
        if (READ_ONLY_UNLOCKED()) {
          s_editMode = (s_editMode <= 0);
        }
      }
      break;

    case EVT_KEY_LONG(KEY_EXIT):
      s_editMode = 0; // TODO needed? we call ENTRY_UP after which does the same
      popMenu();
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      AUDIO_KEY_PRESS();
      if (s_editMode > 0) {
        s_editMode = 0;
        break;
      }

      if (l_posVert == 0 || !menuTab) {
        popMenu();  // beeps itself
      }
      else {
        l_posVert = 0;
        l_posHorz = 0;
      }
      break;

    case EVT_KEY_REPT(KEY_RIGHT):  //inc
      if (l_posHorz == maxcol) break;
      // no break

    case EVT_KEY_FIRST(KEY_RIGHT)://inc
      if (!horTab || s_editMode > 0) break;
      INC(l_posHorz, 0, maxcol);
      break;

    case EVT_KEY_REPT(KEY_DOWN):
      if (l_posVert == maxrow) break;
      // no break

    case EVT_KEY_FIRST(KEY_DOWN):
      if (s_editMode > 0) break;
      do {
        INC(l_posVert, 0, maxrow);
      } while (CURSOR_NOT_ALLOWED_IN_ROW(l_posVert));
      l_posHorz = min<horzpos_t>(l_posHorz, MAXCOL(l_posVert));
      break;

    case EVT_KEY_REPT(KEY_LEFT):  //dec
      if (l_posHorz==0) break;
      // no break

    case EVT_KEY_FIRST(KEY_LEFT)://dec
      if (!horTab || s_editMode > 0) break;
      DEC(l_posHorz, 0, maxcol);
      break;

    case EVT_KEY_REPT(KEY_UP):
      if (l_posVert == 0) break;
      // no break
    case EVT_KEY_FIRST(KEY_UP):
      if (s_editMode > 0) break;
      do {
        DEC(l_posVert, 0, maxrow);
      } while (CURSOR_NOT_ALLOWED_IN_ROW(l_posVert));
      l_posHorz = min((uint8_t)l_posHorz, MAXCOL(l_posVert));
      break;
  }

  uint8_t maxLines = menuTab ? LCD_LINES-1 : LCD_LINES-2;

  int linesCount = maxrow;
  if (l_posVert == 0 || (l_posVert==1 && MAXCOL(vertpos_t(0)) >= HIDDEN_ROW) || (l_posVert==2 && MAXCOL(vertpos_t(0)) >= HIDDEN_ROW && MAXCOL(vertpos_t(1)) >= HIDDEN_ROW)) {
    menuVerticalOffset = 0;
    if (horTab) {
      linesCount = 0;
      for (int i=0; i<maxrow; i++) {
        if (i>=horTabMax || horTab[i] != HIDDEN_ROW) {
          linesCount++;
        }
      }
    }
  }
  else if (horTab) {
    if (maxrow > maxLines) {
      while (1) {
        vertpos_t firstLine = 0;
        for (int numLines=0; firstLine<maxrow && numLines<menuVerticalOffset; firstLine++) {
          if (firstLine>=horTabMax || horTab[firstLine+1] != HIDDEN_ROW) {
            numLines++;
          }
        }
        if (l_posVert <= firstLine) {
          menuVerticalOffset--;
        }
        else {
          vertpos_t lastLine = firstLine;
          for (int numLines=0; lastLine<maxrow && numLines<maxLines; lastLine++) {
            if (lastLine>=horTabMax || horTab[lastLine+1] != HIDDEN_ROW) {
              numLines++;
            }
          }
          if (l_posVert > lastLine) {
            menuVerticalOffset++;
          }
          else {
            linesCount = menuVerticalOffset + maxLines;
            for (int i=lastLine; i<maxrow; i++) {
              if (i>=horTabMax || horTab[i] != HIDDEN_ROW) {
                linesCount++;
              }
            }
            break;
          }
        }
      }
    }
  }
  else {
    if (l_posVert>maxLines+menuVerticalOffset) {
      menuVerticalOffset = l_posVert-maxLines;
    }
    else if (l_posVert<=menuVerticalOffset) {
      menuVerticalOffset = l_posVert-1;
    }
  }

  menuVerticalPosition = l_posVert;
  menuHorizontalPosition = l_posHorz;
  // cosmetics on 9x
  if (menuVerticalOffset > 0) {
    l_posVert--;
    if (l_posVert == menuVerticalOffset && CURSOR_NOT_ALLOWED_IN_ROW(l_posVert)) {
      menuVerticalOffset = l_posVert-1;
    }
  }
}
