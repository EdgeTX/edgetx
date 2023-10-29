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

#include "libopenui_defines.h"
#include "font.h"

#if !defined(BOOT)

#define FONT_TABLE(name)                                 \
  static const lv_font_t* lvglFontTable[FONTS_COUNT] = { \
      LV_FONT_DEFAULT,         /* FONT_STD_INDEX */      \
      &lv_font_ ## name ## _bold_16, /* FONT_BOLD_INDEX */     \
      &lv_font_ ## name ## _9,       /* FONT_XXS_INDEX */      \
      &lv_font_ ## name ## _13,      /* FONT_XS_INDEX */       \
      nullptr,                       /* FONT_L_INDEX */        \
      nullptr,                       /* FONT_XL_INDEX */       \
      nullptr,                       /* FONT_XXL_INDEX */      \
  }

#if defined(TRANSLATIONS_CN)
  FONT_TABLE(noto_cn);
  #define XXL_FONT "F/FONTS/lv_font_noto_cn_bold_64.bin"
  #define XL_FONT "F/FONTS/lv_font_noto_cn_bold_32.bin"
  #define L_FONT "F/FONTS/lv_font_noto_cn_24.bin"
#elif defined(TRANSLATIONS_TW)
  FONT_TABLE(noto_tw);
  #define XXL_FONT "F/FONTS/lv_font_noto_tw_bold_64.bin"
  #define XL_FONT "F/FONTS/lv_font_noto_tw_bold_32.bin"
  #define L_FONT "F/FONTS/lv_font_noto_tw_24.bin"
#elif defined(TRANSLATIONS_JP)
  FONT_TABLE(noto_jp);
  #define XXL_FONT "F/FONTS/lv_font_noto_jp_bold_64.bin"
  #define XL_FONT "F/FONTS/lv_font_noto_jp_bold_32.bin"
  #define L_FONT "F/FONTS/lv_font_noto_jp_24.bin"
#elif defined(TRANSLATIONS_HE)
  FONT_TABLE(arimo_he);
  #define XXL_FONT "F/FONTS/lv_font_arimo_he_bold_64.bin"
  #define XL_FONT "F/FONTS/lv_font_arimo_he_bold_32.bin"
  #define L_FONT "F/FONTS/lv_font_arimo_he_24.bin"
#elif defined(TRANSLATIONS_RU)
  FONT_TABLE(arimo_ru);
  #define XXL_FONT "F/FONTS/lv_font_arimo_ru_bold_64.bin"
  #define XL_FONT "F/FONTS/lv_font_arimo_ru_bold_32.bin"
  #define L_FONT "F/FONTS/lv_font_arimo_ru_24.bin"
#else
  FONT_TABLE(roboto);
  #define XXL_FONT "F/FONTS/lv_font_roboto_bold_64.bin"
  #define XL_FONT "F/FONTS/lv_font_roboto_bold_32.bin"
  #define L_FONT "F/FONTS/lv_font_roboto_24.bin"
#endif

void initFont(uint8_t font)
{
  if (font >= FONT_L_INDEX && lvglFontTable[FONT_L_INDEX] == nullptr) {
    const lv_font_t* lvFont = lv_font_load(L_FONT);
    if (lvFont == nullptr)
      lvFont = lvglFontTable[FONT_STD_INDEX];
    lvglFontTable[FONT_L_INDEX] = lvFont;
  }

  if (font >= FONT_XL_INDEX && lvglFontTable[FONT_XL_INDEX] == nullptr) {
    const lv_font_t* lvFont = lv_font_load(XL_FONT);
    if (lvFont == nullptr)
      lvFont = lvglFontTable[FONT_L_INDEX];
    lvglFontTable[FONT_XL_INDEX] = lvFont;
  }

  if (font == FONT_XXL_INDEX && lvglFontTable[font] == nullptr) {
    const lv_font_t* lvFont = lv_font_load(XXL_FONT);
    if (lvFont == nullptr)
      lvFont = lvglFontTable[font-1];
    lvglFontTable[font] = lvFont;
  }
}

#endif // BOOT

// used to set the line height to the line heights used in Edgetx < 2.7 and OpenTX
static const int8_t FontHeightCorrection[FONTS_COUNT] {
  -2, // STD
  -2, // BOLD
  -2, // XXS
  -2, // XS
  -3, // L
  -5, // XL
  -2, // XXL
};

const lv_font_t* getFont(LcdFlags flags)
{
#if defined(BOOT)
  return LV_FONT_DEFAULT;
#else
  auto fontIndex = FONT_INDEX(flags);
  initFont(fontIndex);
  if (fontIndex >= FONTS_COUNT) return LV_FONT_DEFAULT;
  return lvglFontTable[fontIndex];
#endif
}

uint8_t getFontHeight(LcdFlags flags)
{
  auto font = getFont(flags);
  return lv_font_get_line_height(font);
}

uint8_t getFontHeightCondensed(LcdFlags flags)
{
  auto font = getFont(flags);
  return lv_font_get_line_height(font) + FontHeightCorrection[FONT_INDEX(flags)];
}

int getTextWidth(const char * s, int len, LcdFlags flags)
{
  auto font = getFont(flags);
  lv_coord_t letter_space = 0;
  if (!len) len = strlen(s);
  return lv_txt_get_width(s, len, font, letter_space, LV_TEXT_FLAG_EXPAND);
}
