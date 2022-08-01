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
int8_t s_editMode;
uint8_t noHighlightCounter;
uint8_t menuCalibrationState;
vertpos_t menuVerticalPosition;
horzpos_t menuHorizontalPosition;

int8_t  checkIncDec_Ret;

#define DBLKEYS_PRESSED_RGT_LFT(in)    ((in & (KEYS_GPIO_PIN_RIGHT + KEYS_GPIO_PIN_LEFT)) == (KEYS_GPIO_PIN_RIGHT + KEYS_GPIO_PIN_LEFT))
#define DBLKEYS_PRESSED_UP_DWN(in)     ((in & (KEYS_GPIO_PIN_UP + KEYS_GPIO_PIN_DOWN)) == (KEYS_GPIO_PIN_UP + KEYS_GPIO_PIN_DOWN))
#define DBLKEYS_PRESSED_RGT_UP(in)     ((in & (KEYS_GPIO_PIN_RIGHT + KEYS_GPIO_PIN_UP))  == (KEYS_GPIO_PIN_RIGHT + KEYS_GPIO_PIN_UP))
#define DBLKEYS_PRESSED_LFT_DWN(in)    ((in & (KEYS_GPIO_PIN_LEFT + KEYS_GPIO_PIN_DOWN)) == (KEYS_GPIO_PIN_LEFT + KEYS_GPIO_PIN_DOWN))

INIT_STOPS(stops100, 3, -100, 0, 100)
INIT_STOPS(stops1000, 3, -1000, 0, 1000)
INIT_STOPS(stopsSwitch, 15, SWSRC_FIRST, CATEGORY_END(-SWSRC_FIRST_LOGICAL_SWITCH), CATEGORY_END(-SWSRC_FIRST_TRIM), CATEGORY_END(-SWSRC_LAST_SWITCH+1), 0, CATEGORY_END(SWSRC_LAST_SWITCH), CATEGORY_END(SWSRC_FIRST_TRIM-1), CATEGORY_END(SWSRC_FIRST_LOGICAL_SWITCH-1), SWSRC_LAST)

extern int checkIncDecSelection;

int checkIncDec(event_t event, int val, int i_min, int i_max, unsigned int i_flags, IsValueAvailable isValueAvailable, const CheckIncDecStops &stops)
{
  int newval = val;

#if defined(DBLKEYS)
  uint8_t in = KEYS_PRESSED();
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
      event = 0;
    }
  }
#endif

    if ((s_editMode>0 && (IS_ROTARY_RIGHT(event) || event==EVT_KEY_FIRST(KEY_UP) || event==EVT_KEY_REPT(KEY_UP)))) {
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
    else if ((s_editMode>0 && (IS_ROTARY_LEFT(event) || event==EVT_KEY_FIRST(KEY_DOWN) || event==EVT_KEY_REPT(KEY_DOWN)))) {
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

  if (!READ_ONLY() && i_min==0 && i_max==1 && (event==EVT_KEY_BREAK(KEY_ENTER) || IS_ROTARY_BREAK(event))) {
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
  }
#endif

  if (newval != val) {
    if (!(i_flags & NO_INCDEC_MARKS) && (newval != i_max) && (newval != i_min) && (newval==0 || newval==-100 || newval==+100) && !IS_ROTARY_EVENT(event)) {
      pauseEvents(event); // delay before auto-repeat continues
    }
    AUDIO_KEY_PRESS();
    storageDirty(i_flags & (EE_GENERAL|EE_MODEL));
    checkIncDec_Ret = (newval > val ? 1 : -1);
  }
  else {
    checkIncDec_Ret = 0;
  }

  return newval;
}

#define SCROLL_POT1_TH 32

#define CURSOR_NOT_ALLOWED_IN_ROW(row) ((int8_t)MAXCOL(row) < 0)

#define INC(val, min, max)             if (val<max) {val++;} else {val=min;}
#define DEC(val, min, max)             if (val>min) {val--;} else {val=max;}

tmr10ms_t menuEntryTime;

#define MAXCOL(row)                    (horTab ? *(horTab+min(row, (vertpos_t)horTabMax)) : (const uint8_t)0)
#define POS_HORZ_INIT(posVert)         0

void check(event_t event, uint8_t curr, const MenuHandlerFunc *menuTab, uint8_t menuTabSize, const uint8_t *horTab, uint8_t horTabMax, vertpos_t maxrow)
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
#if defined(ROTARY_ENCODER_NAVIGATION)
        case EVT_ROTARY_BREAK:
          if (s_editMode < 0 && maxrow > 0) {
            s_editMode = 0;
            // TODO ? l_posVert = (horTab && horTab[1]==0xff) ? 2 : 1;
            l_posHorz = 0;
          }
          else {
            s_editMode = -1;
          }
          event = 0;
          break;
#endif

#if defined(ROTARY_ENCODER_NAVIGATION)
        case EVT_ROTARY_LEFT:
          if (s_editMode >= 0)
            break;
#endif
        case EVT_KEY_FIRST(KEY_LEFT):
          if (curr > 0)
            cc = curr - 1;
          else
            cc = menuTabSize-1;
          break;

#if defined(ROTARY_ENCODER_NAVIGATION)
        case EVT_ROTARY_RIGHT:
          if (s_editMode >= 0)
            break;
#endif
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

#if defined(ROTARY_ENCODER_NAVIGATION)
      if (IS_ROTARY_ENCODER_NAVIGATION_ENABLE() && s_editMode < 0)
        attr = INVERS|BLINK;
#endif
    }

    menuCalibrationState = 0;
    drawScreenIndex(curr, menuTabSize, attr);

  }

  switch (event) {
    case EVT_ENTRY:
      menuEntryTime = get_tmr10ms();
      l_posVert = 0;
      l_posHorz = POS_HORZ_INIT(l_posVert);
#if defined(ROTARY_ENCODER_NAVIGATION)
      if (menuTab) {
        s_editMode = EDIT_MODE_INIT;
        break;
      }
      // no break
#else
      s_editMode = EDIT_MODE_INIT;
      break;
#endif

#if defined(ROTARY_ENCODER_NAVIGATION)
    case EVT_ENTRY_UP:
      s_editMode = 0;
      break;

    case EVT_ROTARY_BREAK:
      if (s_editMode > 1) break;
#endif

    case EVT_KEY_FIRST(KEY_ENTER):
      if (!menuTab || l_posVert>0) {
        if (READ_ONLY_UNLOCKED()) {
          s_editMode = (s_editMode <= 0);
        }
      }
      break;

#if defined(ROTARY_ENCODER_NAVIGATION)
    case EVT_ROTARY_LONG:
      if (s_editMode > 1) break;
      killEvents(event);
      if (l_posVert != 0) {
        l_posVert = 0;
        s_editMode = EDIT_MODE_INIT;
        break;
      }
      // no break
#endif

    case EVT_KEY_LONG(KEY_EXIT):
      s_editMode = 0; // TODO needed? we call ENTRY_UP after which does the same
      popMenu();
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      AUDIO_KEY_PRESS();
#if defined(ROTARY_ENCODER_NAVIGATION)
      if (s_editMode == 0)
        s_editMode = EDIT_MODE_INIT;
      else
#endif
      if (s_editMode>0) {
        s_editMode = 0;
        break;
      }

      if (l_posVert==0 || !menuTab) {
        popMenu();  // beeps itself
      }
      else {
        l_posVert = 0;
        l_posHorz = 0;
      }
      break;

    case EVT_KEY_REPT(KEY_RIGHT):  //inc
      if (l_posHorz==maxcol) break;
      // no break

    case EVT_KEY_FIRST(KEY_RIGHT)://inc
      if (!horTab || s_editMode>0) break;

#if defined(ROTARY_ENCODER_NAVIGATION)
    CASE_EVT_ROTARY_RIGHT
      if (s_editMode != 0) break;
      if (l_posHorz < maxcol) {
        l_posHorz++;
        break;
      }
      else {
        l_posHorz = 0;
        if (!IS_ROTARY_RIGHT(event))
          break;
      }
#else
      INC(l_posHorz, 0, maxcol);
      break;
#endif

    case EVT_KEY_REPT(KEY_DOWN):
      if (!IS_ROTARY_RIGHT(event) && l_posVert==maxrow) break;
      // no break

    case EVT_KEY_FIRST(KEY_DOWN):
      if (s_editMode>0) break;
      do {
#if defined(ROTARY_ENCODER_NAVIGATION)
        if (g_eeGeneral.rotEncMode >=
            ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_NORM) {
          DEC(l_posVert, 0, maxrow);
        } else {
          INC(l_posVert, 0, maxrow);
        }
#else
        INC(l_posVert, 0, maxrow);
#endif
      } while (CURSOR_NOT_ALLOWED_IN_ROW(l_posVert));

#if defined(ROTARY_ENCODER_NAVIGATION)
      s_editMode = 0; // if we go down, we must be in this mode
#endif

      l_posHorz = min<horzpos_t>(l_posHorz, MAXCOL(l_posVert));
      break;

    case EVT_KEY_REPT(KEY_LEFT):  //dec
      if (l_posHorz==0) break;
      // no break

    case EVT_KEY_FIRST(KEY_LEFT)://dec
      if (!horTab || s_editMode>0) break;

#if defined(ROTARY_ENCODER_NAVIGATION)
    CASE_EVT_ROTARY_LEFT
      if (s_editMode != 0) break;
      if (l_posHorz > 0) {
        l_posHorz--;
        break;
      } else if (IS_ROTARY_LEFT(event) && s_editMode == 0) {
        l_posHorz = 0xff;
      } else {
        l_posHorz =
            g_eeGeneral.rotEncMode == ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_ALT
                ? 0
                : maxcol;
        break;
      }
#else
      DEC(l_posHorz, 0, maxcol);
      break;
#endif

    case EVT_KEY_REPT(KEY_UP):
      if (!IS_ROTARY_LEFT(event) && l_posVert==0) break;
      // no break
    case EVT_KEY_FIRST(KEY_UP):
      if (s_editMode>0) break;

      do {
#if defined(ROTARY_ENCODER_NAVIGATION)
        if (g_eeGeneral.rotEncMode >=
            ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_NORM) {
          INC(l_posVert, 0, maxrow);
        } else {
          DEC(l_posVert, 0, maxrow);
        }
#else
        DEC(l_posVert, 0, maxrow);
#endif
      } while (CURSOR_NOT_ALLOWED_IN_ROW(l_posVert));

#if defined(ROTARY_ENCODER_NAVIGATION)
      s_editMode = 0; // if we go up, we must be in this mode
#endif

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

