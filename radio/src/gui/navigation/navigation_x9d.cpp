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

vertpos_t menuVerticalOffset;
vertpos_t menuVerticalPosition;
horzpos_t menuHorizontalPosition;
int8_t s_editMode;
uint8_t noHighlightCounter;
uint8_t menuCalibrationState;
extern int checkIncDecSelection;
int8_t  checkIncDec_Ret;

INIT_STOPS(stops100, 3, -100, 0, 100)
INIT_STOPS(stops1000, 3, -1000, 0, 1000)
INIT_STOPS(stopsSwitch, 15, SWSRC_FIRST, CATEGORY_END(-SWSRC_FIRST_LOGICAL_SWITCH), CATEGORY_END(-SWSRC_FIRST_TRIM), CATEGORY_END(-SWSRC_LAST_SWITCH+1), 0, CATEGORY_END(SWSRC_LAST_SWITCH), CATEGORY_END(SWSRC_FIRST_TRIM-1), CATEGORY_END(SWSRC_FIRST_LOGICAL_SWITCH-1), SWSRC_LAST)

void onSwitchLongEnterPress(const char * result)
{
  if (result == STR_MENU_SWITCHES)
    checkIncDecSelection = SWSRC_FIRST_SWITCH;
  else if (result == STR_MENU_TRIMS)
    checkIncDecSelection = SWSRC_FIRST_TRIM;
  else if (result == STR_MENU_LOGICAL_SWITCHES)
    checkIncDecSelection = SWSRC_FIRST_LOGICAL_SWITCH + getFirstAvailable(0, MAX_LOGICAL_SWITCHES, isLogicalSwitchAvailable);
  else if (result == STR_MENU_OTHER)
    checkIncDecSelection = SWSRC_ON;
  else if (result == STR_MENU_INVERT)
    checkIncDecSelection = SWSRC_INVERT;
}

#define DBLKEYS_PRESSED_RGT_LFT(in) ((in & ((1<<KEY_PLUS) + (1<<KEY_MINUS))) == ((1<<KEY_PLUS) + (1<<KEY_MINUS)))
#define DBLKEYS_PRESSED_UP_DWN(in)  ((in & ((1<<KEY_MENU) + (1<<KEY_PAGE))) == ((1<<KEY_MENU) + (1<<KEY_PAGE)))
#define DBLKEYS_PRESSED_RGT_UP(in)  ((in & ((1<<KEY_ENTER) + (1<<KEY_MINUS))) == ((1<<KEY_ENTER) + (1<<KEY_MINUS)))
#define DBLKEYS_PRESSED_LFT_DWN(in) ((in & ((1<<KEY_PAGE) + (1<<KEY_EXIT))) == ((1<<KEY_PAGE) + (1<<KEY_EXIT)))

int checkIncDec(event_t event, int val, int i_min, int i_max, unsigned int i_flags, IsValueAvailable isValueAvailable, const CheckIncDecStops &stops)
{
  int newval = val;

#if defined(DBLKEYS)
  uint32_t in = KEYS_PRESSED();
  if (!(i_flags & NO_DBLKEYS) && (EVT_KEY_MASK(event))) {
    bool dblkey = true;
    if (DBLKEYS_PRESSED_RGT_LFT(in)) {
      if (!isValueAvailable || isValueAvailable(-val)) {
        newval = -val;
      }
    }
    else if (DBLKEYS_PRESSED_RGT_UP(in)) {
      newval = (i_max > stops.max() ? stops.max() : i_max);
      while (isValueAvailable && !isValueAvailable(newval) && newval>i_min) {
        --newval;
      }
    }
    else if (DBLKEYS_PRESSED_LFT_DWN(in)) {
      newval = (i_min < stops.min() ? stops.min() : i_min);
      while (isValueAvailable && !isValueAvailable(newval) && newval<i_max) {
        ++newval;
      }
    }
    else if (DBLKEYS_PRESSED_UP_DWN(in)) {
      newval = 0;
    }
    else {
      dblkey = false;
    }

    if (dblkey) {
      killEvents(KEY_UP);
      killEvents(KEY_DOWN);
      killEvents(KEY_RIGHT);
      killEvents(KEY_LEFT);
      killEvents(KEY_PAGE);
      killEvents(KEY_MENU);
      killEvents(KEY_ENTER);
      killEvents(KEY_EXIT);
      event = 0;
    }
  }
#endif

#if defined(ROTARY_ENCODER_NAVIGATION)
  if (s_editMode>0 && event==EVT_ROTARY_RIGHT) {
    newval += min<int>(rotencSpeed, i_max-val);
    while (isValueAvailable && !isValueAvailable(newval) && newval<=i_max) {
      newval++;
    }
    if (newval > i_max) {
      newval = val;
      AUDIO_KEY_ERROR();
    }
  }
  else if (s_editMode>0 && event==EVT_ROTARY_LEFT) {
    newval -= min<int>(rotencSpeed, val-i_min);
    while (isValueAvailable && !isValueAvailable(newval) && newval>=i_min) {
      newval--;
    }
    if (newval < i_min) {
      newval = val;
      AUDIO_KEY_ERROR();
    }
  }
#else
  if (s_editMode>0 && (event==EVT_KEY_FIRST(KEY_PLUS) || event==EVT_KEY_REPT(KEY_PLUS))) {
    do {
      if (IS_KEY_REPT(event) && (i_flags & INCDEC_REP10)) {
        newval += min(10, i_max-val);
      }
      else {
        newval++;
      }
    } while (isValueAvailable && !isValueAvailable(newval) && newval<=i_max);
    if (newval > i_max) {
      newval = val;
      killEvents(event);
      AUDIO_KEY_ERROR();
    }
  }
  else if (s_editMode>0 && (event==EVT_KEY_FIRST(KEY_MINUS) || event==EVT_KEY_REPT(KEY_MINUS))) {
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
#endif

  if (!READ_ONLY() && i_min==0 && i_max==1 && event==EVT_KEY_BREAK(KEY_ENTER)) {
    s_editMode = 0;
    newval = !val;
  }

#if defined(AUTOSWITCH)
  if (i_flags & INCDEC_SWITCH) {
    newval = checkIncDecMovedSwitch(newval);
  }
#endif

#if defined(AUTOSOURCE)
  if (i_flags & INCDEC_SOURCE) {
    if (s_editMode>0) {
      int source = GET_MOVED_SOURCE(i_min, i_max);
      if (source) {
        newval = source;
      }
#if defined(AUTOSWITCH)
      else {
        unsigned int swtch = abs(getMovedSwitch());
        if (swtch && !IS_SWITCH_MULTIPOS(swtch)) {
          newval = switchToMix(swtch);
        }
      }
#endif
    }
  }
#endif

  if (newval != val) {
#if !defined(ROTARY_ENCODER_NAVIGATION)
    if (!(i_flags & NO_INCDEC_MARKS) && (newval != i_max) && (newval != i_min) && stops.contains(newval)) {
      bool pause = (newval > val ? !stops.contains(newval+1) : !stops.contains(newval-1));
      if (pause) {
        pauseEvents(event); // delay before auto-repeat continues
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

  if (i_flags & INCDEC_SOURCE) {
    if (event == EVT_KEY_LONG(KEY_ENTER)) {
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
    if (event == EVT_KEY_LONG(KEY_ENTER)) {
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

#define CURSOR_NOT_ALLOWED_IN_ROW(row) ((int8_t)MAXCOL(row) < 0)
#define MAXCOL_RAW(row)                (horTab ? *(horTab+min(row, (vertpos_t)horTabMax)) : (const uint8_t)0)
#define MAXCOL(row)                    (MAXCOL_RAW(row) >= HIDDEN_ROW ? MAXCOL_RAW(row) : (const uint8_t)(MAXCOL_RAW(row) & (~NAVIGATION_LINE_BY_LINE)))
#define COLATTR(row)                   (MAXCOL_RAW(row) == (uint8_t)-1 ? (const uint8_t)0 : (const uint8_t)(MAXCOL_RAW(row) & NAVIGATION_LINE_BY_LINE))
#define INC(val, min, max)             if (val<max) {val++;} else if (max>min) {val=min;}
#define DEC(val, min, max)             if (val>min) {val--;} else {val=max;}

coord_t scrollbar_X = DEFAULT_SCROLLBAR_X;

void onLongMenuPress(const char * result)
{
  if (result == STR_VIEW_CHANNELS) {
    pushMenu(menuChannelsView);
  }
  else if (result == STR_VIEW_NOTES) {
    pushModelNotes();
  }
}

tmr10ms_t menuEntryTime;

void check(event_t event, uint8_t curr, const MenuHandlerFunc *menuTab, uint8_t menuTabSize, const uint8_t *horTab, uint8_t horTabMax, vertpos_t rowcount, uint8_t flags)
{
  vertpos_t l_posVert = menuVerticalPosition;
  horzpos_t l_posHorz = menuHorizontalPosition;

  uint8_t maxcol = MAXCOL(l_posVert);

  if (menuTab) {
    int cc = curr;
    switch (event) {
      case EVT_KEY_LONG(KEY_MENU):
        if (menuTab == menuTabModel) {
          killEvents(event);
          if (modelHasNotes()) {
            POPUP_MENU_ADD_SD_ITEM(STR_VIEW_CHANNELS);
            POPUP_MENU_ADD_ITEM(STR_VIEW_NOTES);
            POPUP_MENU_START(onLongMenuPress);
          }
          else {
            pushMenu(menuChannelsView);
          }
        }
        break;

      case EVT_KEY_LONG(KEY_PAGE):
        if (s_editMode>0)
          break;

        if (curr > 0)
          cc = curr - 1;
        else
          cc = menuTabSize-1;
        killEvents(event);
        break;

      case EVT_KEY_BREAK(KEY_PAGE):
        if (s_editMode>0)
          break;

        if (curr < (menuTabSize-1))
          cc = curr + 1;
        else
          cc = 0;
        break;
    }

    if (!menuCalibrationState && cc != curr) {
      chainMenu(menuTab[cc]);
    }

    if (!(flags&CHECK_FLAG_NO_SCREEN_INDEX)) {
      drawScreenIndex(curr, menuTabSize, 0);
    }

    lcdDrawFilledRect(0, 0, LCD_W, MENU_HEADER_HEIGHT, SOLID, FILL_WHITE|GREY_DEFAULT);
  }

  switch (event) {
    case EVT_ENTRY:
      menuEntryTime = get_tmr10ms();
      s_editMode = EDIT_MODE_INIT;
      l_posVert = MENU_FIRST_LINE_EDIT(horTab, horTabMax);
      l_posHorz = POS_HORZ_INIT(l_posVert);
      SET_SCROLLBAR_X(LCD_W-1);
      break;

    case EVT_ENTRY_UP:
      menuEntryTime = get_tmr10ms();
      s_editMode = 0;
      l_posHorz = POS_HORZ_INIT(l_posVert);
      SET_SCROLLBAR_X(LCD_W-1);
      break;

    case EVT_KEY_BREAK(KEY_ENTER):
      if (s_editMode > 1)
        break;
      if (menuHorizontalPosition < 0 && maxcol > 0 && READ_ONLY_UNLOCKED()) {
        l_posHorz = 0;
        AUDIO_KEY_PRESS();
      }
      else if (READ_ONLY_UNLOCKED()) {
        s_editMode = (s_editMode<=0);
        AUDIO_KEY_PRESS();
      }
      break;

    case EVT_KEY_LONG(KEY_EXIT):
      s_editMode = 0; // TODO needed? we call ENTRY_UP after which does the same
      popMenu();
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      if (s_editMode > 0) {
        s_editMode = 0;
        AUDIO_KEY_PRESS();
        break;
      }

      if (l_posHorz >= 0 && (COLATTR(l_posVert) & NAVIGATION_LINE_BY_LINE)) {
        l_posHorz = -1;
        AUDIO_KEY_PRESS();
      }
      else {
        uint8_t posVertInit = MENU_FIRST_LINE_EDIT(horTab, horTabMax);
        if (menuVerticalOffset != 0 || l_posVert != posVertInit) {
          menuVerticalOffset = 0;
          l_posVert = posVertInit;
          l_posHorz = POS_HORZ_INIT(l_posVert);
          AUDIO_KEY_PRESS();
        }
        else {
          popMenu();
        }
      }
      break;

    CASE_EVT_ROTARY_RIGHT
    case EVT_KEY_FIRST(KEY_RIGHT):
      AUDIO_KEY_PRESS();
      // no break
    case EVT_KEY_REPT(KEY_RIGHT):
      if (s_editMode != 0) break;
      if ((COLATTR(l_posVert) & NAVIGATION_LINE_BY_LINE)) {
        if (l_posHorz >= 0) {
          INC(l_posHorz, 0, maxcol);
          break;
        }
      }
      else {
        if (l_posHorz < maxcol) {
          l_posHorz++;
          break;
        }
        else {
          l_posHorz = 0;
        }
      }

      do {
#if defined(ROTARY_ENCODER_NAVIGATION)
        if (g_eeGeneral.rotEncMode >=
            ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_NORM) {
          DEC(l_posVert, MENU_FIRST_LINE_EDIT(horTab, horTabMax), rowcount - 1);
        } else {
          INC(l_posVert, MENU_FIRST_LINE_EDIT(horTab, horTabMax), rowcount - 1);
        }
#else
        DEC(l_posVert, MENU_FIRST_LINE_EDIT(horTab, horTabMax), rowcount - 1);
#endif
      } while (CURSOR_NOT_ALLOWED_IN_ROW(l_posVert));

      s_editMode = 0; // if we go down, we must be in this mode

      l_posHorz = POS_HORZ_INIT(l_posVert);
      break;

    CASE_EVT_ROTARY_LEFT
    case EVT_KEY_FIRST(KEY_LEFT):
      AUDIO_KEY_PRESS();
      // no break
    case EVT_KEY_REPT(KEY_LEFT):
      if (s_editMode != 0) break;
      if ((COLATTR(l_posVert) & NAVIGATION_LINE_BY_LINE)) {
        if (l_posHorz >= 0) {
          DEC(l_posHorz, 0, maxcol);
          break;
        }
      } else if (l_posHorz > 0) {
        l_posHorz--;
        break;
      } else {
#if defined(ROTARY_ENCODER_NAVIGATION)
        l_posHorz =
            g_eeGeneral.rotEncMode == ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_ALT
                ? 0
                : 0xff;
#else
        l_posHorz = 0xff;
#endif
      }

      do {
#if defined(ROTARY_ENCODER_NAVIGATION)
        if (g_eeGeneral.rotEncMode >=
            ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_NORM) {
          INC(l_posVert, MENU_FIRST_LINE_EDIT(horTab, horTabMax), rowcount - 1);
        } else {
          DEC(l_posVert, MENU_FIRST_LINE_EDIT(horTab, horTabMax), rowcount - 1);
        }
#else
          DEC(l_posVert, MENU_FIRST_LINE_EDIT(horTab, horTabMax), rowcount - 1);
#endif
      } while (CURSOR_NOT_ALLOWED_IN_ROW(l_posVert));

      s_editMode = 0; // if we go up, we must be in this mode

      if ((COLATTR(l_posVert) & NAVIGATION_LINE_BY_LINE))
        l_posHorz = -1;
      else
        l_posHorz = min((uint8_t)l_posHorz, MAXCOL(l_posVert));

      break;
  }

  int linesCount = rowcount;

  if (l_posVert == 0 || (l_posVert==1 && MAXCOL(vertpos_t(0)) >= HIDDEN_ROW) || (l_posVert==2 && MAXCOL(vertpos_t(0)) >= HIDDEN_ROW && MAXCOL(vertpos_t(1)) >= HIDDEN_ROW)) {
    menuVerticalOffset = 0;
    if (horTab) {
      linesCount = 0;
      for (int i=0; i<rowcount; i++) {
        if (i>horTabMax || horTab[i] != HIDDEN_ROW) {
          linesCount++;
        }
      }
    }
  }
  else if (horTab) {
    if (rowcount > NUM_BODY_LINES) {
      while (1) {
        vertpos_t firstLine = 0;
        for (int numLines=0; firstLine<rowcount && numLines<menuVerticalOffset; firstLine++) {
          if (firstLine>=horTabMax || horTab[firstLine] != HIDDEN_ROW) {
            numLines++;
          }
        }
        if (l_posVert < firstLine) {
          menuVerticalOffset--;
        }
        else {
          vertpos_t lastLine = firstLine;
          for (int numLines=0; lastLine<rowcount && numLines<NUM_BODY_LINES; lastLine++) {
            if (lastLine>=horTabMax || horTab[lastLine] != HIDDEN_ROW) {
              numLines++;
            }
          }
          if (l_posVert >= lastLine) {
            menuVerticalOffset++;
          }
          else {
            linesCount = menuVerticalOffset + NUM_BODY_LINES;
            for (int i=lastLine; i<rowcount; i++) {
              if (i>horTabMax || horTab[i] != HIDDEN_ROW) {
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
    if (l_posVert>=NUM_BODY_LINES+menuVerticalOffset) {
      menuVerticalOffset = l_posVert-NUM_BODY_LINES+1;
    }
    else if (l_posVert<menuVerticalOffset) {
      menuVerticalOffset = l_posVert;
    }
  }

  if (scrollbar_X && linesCount > NUM_BODY_LINES) {
    drawVerticalScrollbar(scrollbar_X, MENU_HEADER_HEIGHT, LCD_H-MENU_HEADER_HEIGHT, menuVerticalOffset, linesCount, NUM_BODY_LINES);
  }

  menuVerticalPosition = l_posVert;
  menuHorizontalPosition = l_posHorz;
}

