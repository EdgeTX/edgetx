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

#define POS_HORZ_INIT(posVert)         0

int checkIncDec(event_t event, int val, int i_min, int i_max,
                unsigned int i_flags, IsValueAvailable isValueAvailable,
                const CheckIncDecStops &stops)
{
  return checkIncDec(event, val, i_min, i_max, i_min, i_max, i_flags, isValueAvailable, stops);
}

int checkIncDec(event_t event, int val, int i_min, int i_max, int srcMin, int srcMax,
                unsigned int i_flags, IsValueAvailable isValueAvailable,
                const CheckIncDecStops &stops)
{
  int newval = val;

  bool isSource = false;
  if (i_flags & INCDEC_SOURCE_VALUE) {
    SourceNumVal v;
    v.rawValue = val;
    // Save isSource flag;
    isSource = v.isSource;
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

    int vmin = isSource ? srcMin : i_min;
    int vmax = isSource ? srcMax : i_max;

    if (event == EVT_KEY_FIRST(KEY_UP) || event == EVT_KEY_REPT(KEY_UP) ||
        event == EVT_KEY_FIRST(KEY_RIGHT) || event == EVT_KEY_REPT(KEY_RIGHT)) {
      do {
        if (IS_KEY_REPT(event) && (i_flags & INCDEC_REP10)) {
          newval += min(10, vmax - val);
        }
        else {
          newval++;
        }
      } while (!(i_flags & INCDEC_SKIP_VAL_CHECK_FUNC) && isValueAvailable && !isValueAvailable(newval) &&
               newval <= vmax);

      if (newval > vmax) {
        newval = val;
        killEvents(event);
        AUDIO_KEY_ERROR();
      }
    } else if (event == EVT_KEY_FIRST(KEY_DOWN) || event == EVT_KEY_REPT(KEY_DOWN) ||
               event == EVT_KEY_FIRST(KEY_LEFT) || event == EVT_KEY_REPT(KEY_LEFT)) {
      do {
        if (IS_KEY_REPT(event) && (i_flags & INCDEC_REP10)) {
          newval -= min(10, val - vmin);
        } else {
          newval--;
        }
      } while (!(i_flags & INCDEC_SKIP_VAL_CHECK_FUNC) && isValueAvailable && !isValueAvailable(newval) &&
               newval >= vmin);

      if (newval < vmin) {
        newval = val;
        killEvents(event);
        AUDIO_KEY_ERROR();
      }
    }

    auto moved = checkMovedInput(newval, i_flags, isSource);
    if (!isValueAvailable || isValueAvailable(moved))
      newval = moved;

    if (invert) {
      newval = -newval;
      val = -val;
    }
  }

  newval = checkBoolean(event, i_min, i_max, newval, val);

  newval = showPopupMenus(event, newval, srcMin, srcMax, i_flags, isValueAvailable, isSource);

  finishCheckIncDec(event, i_min, i_max, i_flags, newval, val, stops);

  if (i_flags & INCDEC_SOURCE_VALUE) {
    SourceNumVal v;
    v.isSource = isSource;
    v.value = newval;
    newval = v.rawValue;
  }

  return newval;
}

vertpos_t prevRow(vertpos_t l_posVert, vertpos_t maxrow, bool wrap,
                  horzpos_t& l_posHorz, const uint8_t *horTab, uint8_t horTabMax)
{
  if (s_editMode <= 0) {
    vertpos_t origPos = l_posVert;
    do {
      DEC(l_posVert, 0, maxrow - 1);
    } while (CURSOR_NOT_ALLOWED_IN_ROW(l_posVert));
    if (origPos == 0 && !wrap) {
      l_posVert = origPos;
    }
  }
  l_posHorz = min<horzpos_t>(l_posHorz, MAXCOL(l_posVert));
  return l_posVert;
}

vertpos_t nextRow(vertpos_t l_posVert, vertpos_t maxrow, bool wrap,
                  horzpos_t& l_posHorz, const uint8_t *horTab, uint8_t horTabMax)
{
  if (s_editMode <= 0) {
    vertpos_t origPos = l_posVert;
    do {
      INC(l_posVert, 0, maxrow - 1);
    } while (CURSOR_NOT_ALLOWED_IN_ROW(l_posVert));
    if (l_posVert == 0 && !wrap) {
      l_posVert = origPos;
    }
  }
  l_posHorz = min<horzpos_t>(l_posHorz, MAXCOL(l_posVert));
  return l_posVert;
}

void check(event_t event, uint8_t curr, const MenuHandler *menuTab,
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
          cc = chgMenu(curr, menuTab, menuTabSize, -1);
          break;

        case EVT_KEY_FIRST(KEY_RIGHT):
          cc = chgMenu(curr, menuTab, menuTabSize, 1);
          break;
      }

      if (cc != curr) {
        chainMenu(menuTab[cc].menuFunc);
      }
    }

    menuCalibrationState = 0;
    drawScreenIndex(menuIdx(menuTab, curr), menuSize(menuTab, menuTabSize), attr);
  }

  switch (event) {
    case EVT_ENTRY:
      menuEntryTime = get_tmr10ms();
      l_posVert = 0;
      l_posHorz = POS_HORZ_INIT(l_posVert);
      s_editMode = EDIT_MODE_INIT;
      break;

    case EVT_KEY_BREAK(KEY_ENTER):
      if (!menuTab || l_posVert > 0) {
        s_editMode = (s_editMode <= 0);
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
      l_posVert = nextRow(l_posVert, maxrow, false, l_posHorz, horTab, horTabMax);
      break;

    case EVT_KEY_FIRST(KEY_DOWN):
      l_posVert = nextRow(l_posVert, maxrow, true, l_posHorz, horTab, horTabMax);
      break;

    case EVT_KEY_REPT(KEY_LEFT):  //dec
      if (l_posHorz == 0) break;
      // no break

    case EVT_KEY_FIRST(KEY_LEFT)://dec
      if (!horTab || s_editMode > 0) break;
      DEC(l_posHorz, 0, maxcol);
      break;

    case EVT_KEY_REPT(KEY_UP):
      l_posVert = prevRow(l_posVert, maxrow, false, l_posHorz, horTab, horTabMax);
      break;

    case EVT_KEY_FIRST(KEY_UP):
      l_posVert = prevRow(l_posVert, maxrow, true, l_posHorz, horTab, horTabMax);
      break;
  }

  uint8_t maxLines = menuTab ? LCD_LINES-1 : LCD_LINES-2;

  int linesCount = maxrow;

  if (l_posVert == 0 || 
      (l_posVert==1 && MAXCOL(vertpos_t(0)) >= HIDDEN_ROW) ||
      (l_posVert==2 && MAXCOL(vertpos_t(0)) >= HIDDEN_ROW &&
       MAXCOL(vertpos_t(1)) >= HIDDEN_ROW)) {
    menuVerticalOffset = 0;
    if (horTab) {
      linesCount = 0;
      for (int i=0; i<maxrow; i++) {
        if (i>=horTabMax || horTab[i] != HIDDEN_ROW) {
          linesCount++;
        }
      }
    }
  } else if (horTab) {
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
  } else {
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
