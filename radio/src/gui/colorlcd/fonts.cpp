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

#include "opentx.h"

const uint16_t font_xxs_specs[] = {
#include "font_9.specs"
};

const unsigned char font_xxs[] = {
#include "font_9.lbm"
};

#if LCD_H > 272

#if defined(BOOT)
const uint16_t font_std_en_specs[] = {
#include "font_17en.specs"
};

const unsigned char font_std_en[] = {
#include "font_17en.lbm"
};
#endif

const uint16_t font_xs_specs[] = {
#include "font_15.specs"
};

const unsigned char font_xs[] = {
#include "font_15.lbm"
};

const uint16_t font_std_specs[] = {
#include "font_17.specs"
};

const unsigned char font_std[] = {
#include "font_17.lbm"
};

const uint16_t font_bold_specs[] = {
#include "font_bold17.specs"
};

const unsigned char font_bold[] = {
#include "font_bold17.lbm"
};

#else // LCD_H <= 272

#if defined(BOOT)
const uint16_t font_std_en_specs[] = {
#include "font_16en.specs"
};

const unsigned char font_std_en[] = {
#include "font_16en.lbm"
};
#endif

const uint16_t font_xs_specs[] = {
#include "font_13.specs"
};

const unsigned char font_xs[] = {
#include "font_13.lbm"
};

const uint16_t font_std_specs[] = {
#include "font_16.specs"
};

const unsigned char font_std[] = {
#include "font_16.lbm"
};

const uint16_t font_bold_specs[] = {
#include "font_bold16.specs"
};

const unsigned char font_bold[] = {
#include "font_bold16.lbm"
};

#endif // LCD_H > 272

const uint16_t font_l_specs[] = {
#include "font_24.specs"
};

const unsigned char font_l[] = {
#include "font_24.lbm"
};

const uint16_t font_xl_specs[] = {
#include "font_32.specs"
};

const unsigned char font_xl[] = {
#include "font_32.lbm"
};

const uint16_t font_xxl_specs[] = {
#include "font_64.specs"
};

const unsigned char font_xxl[] = {
#include "font_64.lbm"
};

#if defined(BOOT)
// -2 for: overall length and last boundary
const uint16_t fontCharactersTable[FONTS_COUNT] = { sizeof(font_std_en_specs)/2-2 };
const uint16_t * const fontspecsTable[FONTS_COUNT] = { font_std_en_specs };
const uint8_t * fontsTable[FONTS_COUNT] = { font_std_en };
const int fontsSizeTable[FONTS_COUNT] = { sizeof(font_std_en) };
#else
// -2 for: overall length and last boundary
const uint16_t fontCharactersTable[FONTS_COUNT] = {
    sizeof(font_std_specs)/2-2, sizeof(font_bold_specs)/2-2, sizeof(font_xxs_specs)/2-2,
    sizeof(font_xs_specs)/2-2,  sizeof(font_l_specs)/2-2,    sizeof(font_xl_specs)/2-2,
    sizeof(font_xxl_specs)/2-2
};
const uint16_t *const fontspecsTable[FONTS_COUNT] = {
    font_std_specs, font_bold_specs, font_xxs_specs, font_xs_specs,
    font_l_specs,   font_xl_specs,   font_xxl_specs
};
const uint8_t *fontsTable[FONTS_COUNT] = {
    font_std, font_bold, font_xxs, font_xs, font_l, font_xl, font_xxl
};
const int fontsSizeTable[FONTS_COUNT] = {
    sizeof(font_std), sizeof(font_bold), sizeof(font_xxs),
    sizeof(font_xs),  sizeof(font_l),    sizeof(font_xl),
    sizeof(font_xxl)
};
#endif

uint8_t * decompressFont(const uint8_t * font, unsigned len)
{
  int width  = 0;
  int height = 0;

  uint8_t* raw_font = BitmapBuffer::loadFont(font, len, width, height);
  if (!raw_font) {
    return nullptr;
  }

  size_t font_size = width * height;
  uint8_t * buf = (uint8_t *)malloc(font_size + 4);

  ((uint16_t*)buf)[0] = (uint16_t)width;
  ((uint16_t*)buf)[1] = (uint16_t)height;

#if defined(LCD_VERTICAL_INVERT)
  uint8_t* src = raw_font + font_size - 1;
  for(uint8_t* dst = buf + 4; src >= raw_font; --src, ++dst) {
    *dst = 0xFF - *src;
  }    
#else
  uint8_t* src = raw_font;
  for(uint8_t* dst = buf + 4; src != raw_font + font_size; ++src, ++dst) {
    *dst = 0xFF - *src;
  }    
#endif
  
  return buf;
}

void loadFonts()
{
  static bool fontsLoaded = false;
  if (fontsLoaded)
    return;

#if defined(BOOT)
  fontsTable[0] = decompressFont(fontsTable[0], fontsSizeTable[0]);
#else
  for (int i = 0; i < FONTS_COUNT; i++) {
    fontsTable[i] = decompressFont(fontsTable[i], fontsSizeTable[i]);
  }
#endif

  fontsLoaded = true;
}
