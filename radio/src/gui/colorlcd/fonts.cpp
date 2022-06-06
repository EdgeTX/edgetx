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

#include "libopenui_config.h"
#include "font.h"

#if !defined(BOOT)

#define FONT_TABLE(name)                                 \
  static const lv_font_t* lvglFontTable[FONTS_COUNT] = { \
      LV_FONT_DEFAULT,         /* FONT_STD_INDEX */      \
      &lv_font_ ## name ## _bold_16, /* FONT_BOLD_INDEX */     \
      &lv_font_ ## name ## _9,       /* FONT_XXS_INDEX */      \
      &lv_font_ ## name ## _13,      /* FONT_XS_INDEX */       \
      &lv_font_ ## name ## _24,      /* FONT_L_INDEX */        \
      &lv_font_ ## name ## _bold_32, /* FONT_XL_INDEX */       \
  }

#if defined(TRANSLATIONS_CN)
  FONT_TABLE(noto_cn);
#elif defined(TRANSLATIONS_TW)
  FONT_TABLE(noto_tw);
#elif defined(TRANSLATIONS_JP)
  FONT_TABLE(noto_jp);
#else
  FONT_TABLE(roboto);
#endif

#endif // BOOT

const lv_font_t* getFont(LcdFlags flags)
{
#if defined(BOOT)
  return LV_FONT_DEFAULT;
#else
  auto fontIndex = FONT_INDEX(flags);
  if (fontIndex >= FONTS_COUNT) return LV_FONT_DEFAULT;
  return lvglFontTable[fontIndex];
#endif
}

uint8_t getFontHeight(LcdFlags flags)
{
  auto font = getFont(flags);
  return lv_font_get_line_height(font);
}

int getTextWidth(const char * s, int len, LcdFlags flags)
{
  auto font = getFont(flags);
  lv_coord_t letter_space = 0;
  if (!len) len = strlen(s);
  return lv_txt_get_width(s, len, font, letter_space, LV_TEXT_FLAG_EXPAND);
}
