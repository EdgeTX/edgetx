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

#include "lcd.h"
#include "opentx.h"

uint8_t getMappedChar(uint8_t c)
{
  uint8_t result;

  result = c - 0x20;

//  TRACE("getMappedChar '%c' (0x%x) = %d", c, c, result);

  return result;
}

int getFontPatternWidth(const uint16_t * spec, int index)
{
  return spec[index + 2] - spec[index + 1];
}

int getCharWidth(uint8_t c, const uint16_t * spec)
{
  return getFontPatternWidth(spec, getMappedChar(c));
}

uint8_t getFontHeight(LcdFlags flags)
{
  uint32_t fontindex = FONT_INDEX(flags);
  return fontspecsTable[fontindex][0];
}

int getTextWidth(const char * s, int len, LcdFlags flags)
{
  const uint16_t * specs = fontspecsTable[FONT_INDEX(flags)];

  int result = 0;
  for (int i = 0; len == 0 || i < len; ++i) {
    unsigned c = uint8_t(*s);
    if (!c) {
      break;
    }
    else if (c >= 0xFE) { // CJK marker
      s++;
      c = uint8_t(*s) + ((c & 0x01) << 8) - 1;
      if (c >= 0x100)
        c -= 1;
      c += CJK_FIRST_LETTER_INDEX;
      result += getFontPatternWidth(specs, c) + 1;
    }
    else if ((c >= 0x20u) && (c < fontCharactersTable[FONT_INDEX(flags)] + 0x20u)) {
      result += getCharWidth(c, specs);
    }
    else {
      TRACE("char out-of bound: 0x%X", c);
    }

    ++s;
  }

  return result;
}

void lcdSetContrast()
{
  lcdSetRefVolt(g_eeGeneral.contrast);
}

event_t getWindowEvent()
{
  return getEvent(false);
}
