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

bool menuTextView(event_t event)
{
  static int lines_count;

  drawMenuTemplate(STR_TEXT_VIEWER, ICON_OPENTX, nullptr, OPTION_MENU_NO_SCROLLBAR);

  switch (event) {
    case EVT_ENTRY:
      menuVerticalOffset = 0;
      lines_count = 0;
      sdReadTextFile(reusableBuffer.viewText.filename, reusableBuffer.viewText.lines, lines_count);
      break;

    case EVT_ROTARY_LEFT:
      if (menuVerticalOffset == 0)
        break;
      else
        menuVerticalOffset--;
      sdReadTextFile(reusableBuffer.viewText.filename, reusableBuffer.viewText.lines, lines_count);
      break;

    case EVT_ROTARY_RIGHT:
      if (menuVerticalOffset + TEXT_VIEWER_LINES >= lines_count)
        break;
      else
        ++menuVerticalOffset;
      sdReadTextFile(reusableBuffer.viewText.filename, reusableBuffer.viewText.lines, lines_count);
      break;

    case EVT_KEY_FIRST(KEY_EXIT):
      menuVerticalOffset = 0;
      popMenu();
      break;
  }

  for (int i = 0; i < TEXT_VIEWER_LINES; i++) {
    lcd->drawTextMaxWidth(MENUS_MARGIN_LEFT, MENU_HEADER_HEIGHT + i * FH, reusableBuffer.viewText.lines[i], 0, LCD_W - 2 * MENUS_MARGIN_LEFT);
  }

  drawVerticalScrollbar(LCD_W-5, 50, 195, menuVerticalOffset, lines_count, TEXT_VIEWER_LINES);

  return true;
}

void pushMenuTextView(const char *filename)
{
  if (strlen(filename) < TEXT_FILENAME_MAXLEN) {
    strcpy(reusableBuffer.viewText.filename, filename);
    pushMenu(menuTextView);
  }
}
