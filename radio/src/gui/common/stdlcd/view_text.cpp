/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#if defined(ROTARY_ENCODER_NAVIGATION)
#define EVT_KEY_NEXT_LINE              EVT_ROTARY_RIGHT
#define EVT_KEY_PREVIOUS_LINE          EVT_ROTARY_LEFT
#else
#define EVT_KEY_NEXT_LINE              EVT_KEY_FIRST(KEY_DOWN)
#define EVT_KEY_PREVIOUS_LINE          EVT_KEY_FIRST(KEY_UP)
#endif

void readModelNotes()
{
  LED_ERROR_BEGIN();

  strcpy(reusableBuffer.viewText.filename, MODELS_PATH "/");
  char *buf = strcat_modelname(&reusableBuffer.viewText.filename[sizeof(MODELS_PATH)], g_eeGeneral.currModel);
  strcpy(buf, TEXT_EXT);

  waitKeysReleased();
  event_t event = EVT_ENTRY;
  while (event != EVT_KEY_BREAK(KEY_EXIT)) {
    lcdRefreshWait();
    lcdClear();
    menuTextView(event);
    event = getEvent();
    lcdRefresh();
    WDG_RESET();
  }

  LED_ERROR_END();
}

void menuTextView(event_t event)
{
  switch (event) {
    case EVT_ENTRY:
      menuVerticalOffset = 0;
      reusableBuffer.viewText.linesCount = 0;
      sdReadTextFile(reusableBuffer.viewText.filename, reusableBuffer.viewText.lines, reusableBuffer.viewText.linesCount);
      break;

    case EVT_KEY_PREVIOUS_LINE:
      if (menuVerticalOffset == 0)
        break;
      else
        menuVerticalOffset--;
      sdReadTextFile(reusableBuffer.viewText.filename, reusableBuffer.viewText.lines, reusableBuffer.viewText.linesCount);
      break;

    case EVT_KEY_NEXT_LINE:
      if (menuVerticalOffset+LCD_LINES-1 >= reusableBuffer.viewText.linesCount)
        break;
      else
        ++menuVerticalOffset;
      sdReadTextFile(reusableBuffer.viewText.filename, reusableBuffer.viewText.lines, reusableBuffer.viewText.linesCount);
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      popMenu();
      break;
  }

  for (int i=0; i<LCD_LINES-1; i++) {
    lcdDrawText(0, i*FH+FH+1, reusableBuffer.viewText.lines[i], FIXEDWIDTH);
  }

  char * title = reusableBuffer.viewText.filename;
#if defined(SIMU)
  if (!strncmp(title, "./", 2)) title += 2;
#else
  // TODO?
#endif
  lcdDrawText(LCD_W/2, 0, getBasename(title), CENTERED);
  lcdInvertLine(0);

  if (reusableBuffer.viewText.linesCount > LCD_LINES-1) {
    drawVerticalScrollbar(LCD_W-1, FH, LCD_H-FH, menuVerticalOffset, reusableBuffer.viewText.linesCount, LCD_LINES-1);
  }
}

void pushMenuTextView(const char *filename)
{
  if (strlen(filename) < TEXT_FILENAME_MAXLEN) {
    strcpy(reusableBuffer.viewText.filename, filename);
    pushMenu(menuTextView);
  }
}

#undef EVT_KEY_NEXT_LINE
#undef EVT_KEY_PREVIOUS_LINE
