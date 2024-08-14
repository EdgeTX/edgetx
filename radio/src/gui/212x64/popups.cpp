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

const unsigned char ASTERISK_BITMAP[]  = {
#include "asterisk.lbm"
};

void drawAlertBox(const char * title, const char * text, const char * action)
{
  lcdClear();
  lcdDrawRleBitmap(0, 0, ASTERISK_BITMAP);

#define MESSAGE_LCD_OFFSET   60

#if defined(TRANSLATIONS_FR) || defined(TRANSLATIONS_IT) || defined(TRANSLATIONS_CZ)
  lcdDrawText(MESSAGE_LCD_OFFSET, 0, STR_WARNING, DBLSIZE);
  lcdDrawText(MESSAGE_LCD_OFFSET, 2*FH, title, DBLSIZE);
#else
  lcdDrawText(MESSAGE_LCD_OFFSET, 0, title, DBLSIZE);
  lcdDrawText(MESSAGE_LCD_OFFSET, 2*FH, STR_WARNING, DBLSIZE);
#endif

  lcdDrawFilledRect(MESSAGE_LCD_OFFSET, 0, LCD_W-MESSAGE_LCD_OFFSET, 32);

  if (text) {
    lcdDrawText(MESSAGE_LCD_OFFSET, 5*FH, text);
  }

  if (action) {
    lcdDrawText(MESSAGE_LCD_OFFSET, 7*FH, action);
  }

#undef MESSAGE_LCD_OFFSET
}
