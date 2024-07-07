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

#include "hal/rotary_encoder.h"

#define COLATTR(row)                   (MAXCOL_RAW(row) == (uint8_t)-1 ? (const uint8_t)0 : (const uint8_t)(MAXCOL_RAW(row) & NAVIGATION_LINE_BY_LINE))
#define POS_HORZ_INIT(posVert)         ((COLATTR(posVert) & NAVIGATION_LINE_BY_LINE) ? -1 : 0)

int checkIncDec(event_t event, int val, int i_min, int i_max,
                unsigned int i_flags, IsValueAvailable isValueAvailable,
                const CheckIncDecStops &stops)
{
  int newval = val;
  event_t evt_rot_inc = EVT_ROTARY_RIGHT;
  event_t evt_rot_dec = EVT_ROTARY_LEFT;
#if defined(ROTARY_ENCODER_NAVIGATION)
  if (g_eeGeneral.rotEncMode == ROTARY_ENCODER_MODE_VERT_NORM_HORZ_INVERT) {
    evt_rot_inc = EVT_ROTARY_LEFT;
    evt_rot_dec = EVT_ROTARY_RIGHT;
  }
#endif

  bool isSource = false;
  bool origIsSource = false;
  if (i_flags & INCDEC_SOURCE_VALUE) {
    SourceNumVal v;
    v.rawValue = val;
    // Save isSource flag;
    origIsSource = isSource = v.isSource;
    // Remove isSource flag;
    val = v.value;
    newval = v.value;
  }

  if (s_editMode > 0) {
    bool invert = false;
    if ((i_flags & INCDEC_SOURCE_INVERT) && (newval < 0)) {
      invert = true;
      newval = -newval;
      val = -val;
    }

    if (event == evt_rot_inc || event == EVT_KEY_FIRST(KEY_PLUS) ||
        event == EVT_KEY_REPT(KEY_PLUS)) {

      if (IS_KEY_REPT(event) && (i_flags & INCDEC_REP10)) {
        newval += min(10, i_max - val);
      } else {
        newval += min<int>(rotaryEncoderGetAccel(), i_max - val);
      }

      while (isValueAvailable && !isValueAvailable(newval) && newval <= i_max) {
        newval++;
      }

      if (newval > i_max) {
        newval = val;
        AUDIO_KEY_ERROR();
      }
    } else if (event == evt_rot_dec || event == EVT_KEY_FIRST(KEY_MINUS) ||
               event == EVT_KEY_REPT(KEY_MINUS)) {

      if (IS_KEY_REPT(event) && (i_flags & INCDEC_REP10)) {
        newval -= min(10, val - i_min);
      } else {
        newval -= min<int>(rotaryEncoderGetAccel(), val - i_min);
      }

      while (isValueAvailable && !isValueAvailable(newval) && newval >= i_min) {
        newval--;
      }

      if (newval < i_min) {
        newval = val;
        AUDIO_KEY_ERROR();
      }
    }

#if defined(AUTOSWITCH)
    if (i_flags & INCDEC_SWITCH) {
      newval = checkIncDecMovedSwitch(newval);
    }
#endif

#if defined(AUTOSOURCE)
    if (i_flags & (INCDEC_SOURCE|INCDEC_SOURCE_VALUE)) {
      int source = GET_MOVED_SOURCE(i_min, i_max);
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
        unsigned int swtch = abs(getMovedSwitch());
        if (swtch && !IS_SWITCH_MULTIPOS(swtch)) {
          newval = switchToMix(swtch);
        }
      }
#endif
    }
#endif

    if (invert) {
      newval = -newval;
      val = -val;
    }
  }

  if (i_min == 0 && i_max == 1 &&
      event == EVT_KEY_BREAK(KEY_ENTER)) {
    s_editMode = 0;
    newval = !val;
  }

  newval = showPopupMenus(event, newval, i_min, i_max, i_flags, isValueAvailable);

  if (newval != val) {
    storageDirty(i_flags & (EE_GENERAL|EE_MODEL));
    checkIncDec_Ret = (newval > val ? 1 : -1);
  }
  else {
    checkIncDec_Ret = 0;
  }

  if (i_flags & INCDEC_SOURCE_VALUE) {
    SourceNumVal v;
    v.isSource = isSource;
    v.value = newval;
    newval = v.rawValue;
  }

  return newval;
}

void check(event_t event, uint8_t curr, const MenuHandler *menuTab,
           uint8_t menuTabSize, const uint8_t *horTab, uint8_t horTabMax,
           vertpos_t rowcount, uint8_t flags)
{
  vertpos_t l_posVert = menuVerticalPosition;
  horzpos_t l_posHorz = menuHorizontalPosition;

  uint8_t maxcol = MAXCOL(l_posVert);

  if (menuTab) {
    int cc = curr;
    switch (event) {
      case EVT_KEY_BREAK(KEY_PAGEUP):
        if (s_editMode>0)
          break;

        cc = chgMenu(curr, menuTab, menuTabSize, -1);
        break;

      case EVT_KEY_BREAK(KEY_PAGEDN):
        if (s_editMode>0)
          break;

        cc = chgMenu(curr, menuTab, menuTabSize, 1);
        break;
    }

    if (!menuCalibrationState && cc != curr) {
      chainMenu(menuTab[cc].menuFunc);
    }

    drawScreenIndex(menuIdx(menuTab, curr), menuSize(menuTab, menuTabSize), 0);
  }

  switch (event) {
    case EVT_ENTRY:
      menuEntryTime = get_tmr10ms();
      s_editMode = EDIT_MODE_INIT;
      l_posVert = MENU_FIRST_LINE_EDIT(horTab, horTabMax);
      l_posHorz = POS_HORZ_INIT(l_posVert);
      break;

    case EVT_ENTRY_UP:
      menuEntryTime = get_tmr10ms();
      s_editMode = 0;
      l_posHorz = POS_HORZ_INIT(l_posVert);
      break;

    case EVT_KEY_BREAK(KEY_ENTER):
      if (s_editMode > 1)
        break;
      if (menuHorizontalPosition < 0 && maxcol > 0) {
        l_posHorz = 0;
        AUDIO_KEY_PRESS();
      }
      else {
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

    case EVT_ROTARY_RIGHT:
    case EVT_KEY_FIRST(KEY_MINUS):
    case EVT_KEY_REPT(KEY_MINUS):
      AUDIO_KEY_PRESS();
      if (s_editMode > 0) break; // TODO it was !=
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
        if (g_eeGeneral.rotEncMode == ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_NORM || \
            g_eeGeneral.rotEncMode == ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_ALT) {
          DEC(l_posVert, MENU_FIRST_LINE_EDIT(horTab, horTabMax), rowcount - 1);
        } else {
          INC(l_posVert, MENU_FIRST_LINE_EDIT(horTab, horTabMax), rowcount - 1);
        }
#else
        INC(l_posVert, MENU_FIRST_LINE_EDIT(horTab, horTabMax), rowcount-1);
#endif
      } while (CURSOR_NOT_ALLOWED_IN_ROW(l_posVert));

      s_editMode = 0; // if we go down, we must be in this mode

      l_posHorz = POS_HORZ_INIT(l_posVert);
      break;

    case EVT_ROTARY_LEFT:
    case EVT_KEY_FIRST(KEY_PLUS):
    case EVT_KEY_REPT(KEY_PLUS):
      AUDIO_KEY_PRESS();
      if (s_editMode > 0) break; // TODO it was !=
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
        if (g_eeGeneral.rotEncMode == ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_NORM || \
            g_eeGeneral.rotEncMode == ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_ALT) {
          INC(l_posVert, MENU_FIRST_LINE_EDIT(horTab, horTabMax), rowcount - 1);
        } else {
          DEC(l_posVert, MENU_FIRST_LINE_EDIT(horTab, horTabMax), rowcount - 1);
        }
#else
        DEC(l_posVert, MENU_FIRST_LINE_EDIT(horTab, horTabMax), rowcount-1);
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

  if (l_posVert == 0 ||
      (l_posVert == 1 && MAXCOL(vertpos_t(0)) >= HIDDEN_ROW) ||
      (l_posVert == 2 && MAXCOL(vertpos_t(0)) >= HIDDEN_ROW &&
       MAXCOL(vertpos_t(1)) >= HIDDEN_ROW)) {
    menuVerticalOffset = 0;
    if (horTab) {
      linesCount = 0;
      for (int i=0; i<rowcount; i++) {
        if (i>horTabMax || horTab[i] != HIDDEN_ROW) {
          linesCount++;
        }
      }
    }
  } else if (horTab) {
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
  } else {
    if (l_posVert>=NUM_BODY_LINES+menuVerticalOffset) {
      menuVerticalOffset = l_posVert-NUM_BODY_LINES+1;
    }
    else if (l_posVert<menuVerticalOffset) {
      menuVerticalOffset = l_posVert;
    }
  }

  menuVerticalPosition = l_posVert;
  menuHorizontalPosition = l_posHorz;
}
