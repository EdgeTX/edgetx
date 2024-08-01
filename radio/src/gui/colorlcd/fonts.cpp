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

#include <stdio.h>
#include <stdlib.h>

#include "fonts.h"
#include "lz4/lz4.h"
#include "libopenui_defines.h"
#include "lz4_fonts.h"

#if !defined(BOOT)

#define FONT_TABLE(name)                                 \
  extern const etxLz4Font lv_font_##name##_bold_16;      \
  extern const etxLz4Font lv_font_##name##_9;            \
  extern const etxLz4Font lv_font_##name##_13;           \
  extern const etxLz4Font lv_font_##name##_24;           \
  extern const etxLz4Font lv_font_##name##_bold_32;      \
  extern const etxLz4Font lv_font_##name##_bold_64;      \
  static const etxLz4Font* lz4FontTable[FONTS_COUNT] = { \
      nullptr,                   /* FONT_STD_INDEX */    \
      &lv_font_##name##_bold_16, /* FONT_BOLD_INDEX */   \
      &lv_font_##name##_9,       /* FONT_XXS_INDEX */    \
      &lv_font_##name##_13,      /* FONT_XS_INDEX */     \
      &lv_font_##name##_24,      /* FONT_L_INDEX */      \
      &lv_font_##name##_bold_32, /* FONT_XL_INDEX */     \
      &lv_font_##name##_bold_64, /* FONT_XXL_INDEX */    \
  };                                                     \
  static const lv_font_t* lvglFontTable[FONTS_COUNT] = { \
      LV_FONT_DEFAULT, /* FONT_STD_INDEX */              \
      nullptr,         /* FONT_BOLD_INDEX */             \
      nullptr,         /* FONT_XXS_INDEX */              \
      nullptr,         /* FONT_XS_INDEX */               \
      nullptr,         /* FONT_L_INDEX */                \
      nullptr,         /* FONT_XL_INDEX */               \
      nullptr,         /* FONT_XXL_INDEX */              \
  }

#if defined(TRANSLATIONS_CN)
FONT_TABLE(noto_cn);
#elif defined(TRANSLATIONS_TW)
FONT_TABLE(noto_tw);
#elif defined(TRANSLATIONS_JP)
FONT_TABLE(noto_jp);
#elif defined(TRANSLATIONS_HE)
FONT_TABLE(arimo_he);
#elif defined(TRANSLATIONS_RU)
FONT_TABLE(arimo_ru);
#elif defined(TRANSLATIONS_UA)
FONT_TABLE(arimo_ua);
#else
FONT_TABLE(roboto);
#endif

/*
  Decompress an LZ4 font and build LVGL font structures.
*/
void decompressFont(int idx)
{
  // Check if already decompressed
  if (lvglFontTable[idx]) return;

  const etxLz4Font* etxFont = lz4FontTable[idx];

  // Calculate size of memory block to allocate for uncompressed data plus LVGL
  // font structures.
  int size = etxFont->uncomp_size + sizeof(lv_font_t) +
             sizeof(lv_font_fmt_txt_dsc_t) +
             sizeof(lv_font_fmt_txt_glyph_cache_t) +
             etxFont->cmap_num * sizeof(lv_font_fmt_txt_cmap_t);
  if (etxFont->kern_classes) size += sizeof(lv_font_fmt_txt_kern_classes_t);

  // Init SDRAM buffer
  uint8_t* data = etxFont->lvglFontBuf;
  memset(data, 0, etxFont->lvglFontBufSize);

  // Pointer to next free area in data block
  uint8_t* next = data;

  // 'Create' lv_font_t structure
  lv_font_t* lvglFont = (lv_font_t*)next;
  next += sizeof(lv_font_t);

  // 'Create' lv_font_fmt_txt_dsc_t structure
  lv_font_fmt_txt_dsc_t* lvglFontDsc = (lv_font_fmt_txt_dsc_t*)next;
  next += sizeof(lv_font_fmt_txt_dsc_t);

  // 'Create' lv_font_fmt_txt_glyph_cache_t structure
  lv_font_fmt_txt_glyph_cache_t* lvglCache =
      (lv_font_fmt_txt_glyph_cache_t*)next;
  next += sizeof(lv_font_fmt_txt_glyph_cache_t);

  // 'Create' lv_font_fmt_txt_kern_classes_t structure (optional)
  lv_font_fmt_txt_kern_classes_t* lvglKernClasses = nullptr;
  if (etxFont->kern_classes) {
    lvglKernClasses = (lv_font_fmt_txt_kern_classes_t*)next;
    next += sizeof(lv_font_fmt_txt_kern_classes_t);
  }

  // 'Create' lv_font_fmt_txt_cmap_t structure array
  lv_font_fmt_txt_cmap_t* lvglCmaps = (lv_font_fmt_txt_cmap_t*)next;
  next += sizeof(lv_font_fmt_txt_cmap_t) * etxFont->cmap_num;

  // Decompress the compressed data into the remaining space
  // 'next' is now the start of the uncompressed data
  LZ4_decompress_safe((const char*)etxFont->compressed, (char*)next,
                      etxFont->comp_size, etxFont->uncomp_size);

  // Rebuild the lv_font_t structure
  // Relocate pointers
  lvglFont->get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt;
  lvglFont->get_glyph_bitmap = lv_font_get_bitmap_fmt_txt;
  lvglFont->dsc = lvglFontDsc;
  // Fill in other properties
  lvglFont->line_height = etxFont->line_height;
  lvglFont->base_line = etxFont->base_line;
  lvglFont->subpx = etxFont->subpx;
  lvglFont->underline_position = etxFont->underline_position;
  lvglFont->underline_thickness = etxFont->underline_thickness;

  // Rebuild the lv_font_fmt_txt_dsc_t structure
  // Relocate pointers
  lvglFontDsc->glyph_bitmap = &next[etxFont->glyph_bitmap];
  lvglFontDsc->glyph_dsc = (lv_font_fmt_txt_glyph_dsc_t*)&next[0];
  lvglFontDsc->cmaps = lvglCmaps;
  lvglFontDsc->kern_dsc = lvglKernClasses;
  lvglFontDsc->kern_classes = etxFont->kern_classes;
  lvglFontDsc->bitmap_format = etxFont->bitmap_format;
  lvglFontDsc->cache = lvglCache;
  // Fill in other properties
  lvglFontDsc->kern_scale = etxFont->kern_scale;
  lvglFontDsc->cmap_num = etxFont->cmap_num;
  lvglFontDsc->bpp = etxFont->bpp;

  // Rebuild the lv_font_fmt_txt_kern_classes_t structure
  if (etxFont->kern_classes) {
    // Relocate pointers
    lvglKernClasses->class_pair_values =
        (int8_t*)&next[etxFont->class_pair_values];
    lvglKernClasses->left_class_mapping = &next[etxFont->left_class_mapping];
    lvglKernClasses->right_class_mapping = &next[etxFont->right_class_mapping];
    // Fill in other properties
    lvglKernClasses->left_class_cnt = etxFont->left_class_cnt;
    lvglKernClasses->right_class_cnt = etxFont->right_class_cnt;
  }

  // Rebuild the lv_font_t lv_font_fmt_txt_cmap_t structure array
  for (int i = 0; i < etxFont->cmap_num; i += 1) {
    // Relocate pointers
    if (etxFont->cmaps[i].unicode_list)
      lvglCmaps[i].unicode_list =
          (uint16_t*)&next[etxFont->cmaps[i].unicode_list];
    if (etxFont->cmaps[i].glyph_id_ofs_list)
      lvglCmaps[i].glyph_id_ofs_list =
          &next[etxFont->cmaps[i].glyph_id_ofs_list];
    // Fill in other properties
    lvglCmaps[i].range_start = etxFont->cmaps[i].range_start;
    lvglCmaps[i].range_length = etxFont->cmaps[i].range_length;
    lvglCmaps[i].glyph_id_start = etxFont->cmaps[i].glyph_id_start;
    lvglCmaps[i].list_length = etxFont->cmaps[i].list_length;
    lvglCmaps[i].type = etxFont->cmaps[i].type;
  }

  // Save LVGL font to lookup array
  lvglFontTable[idx] = lvglFont;
}

#endif  // BOOT

// used to set the line height to the line heights used in Edgetx < 2.7 and
// OpenTX
static const int8_t FontHeightCorrection[FONTS_COUNT] = {
    -2,  // ST
    -2,  // BOLD
    -2,  // XXS
    -2,  // XS
    -3,  // L
    -5,  // XL
    -2,  // XXL
};

const lv_font_t* getFont(LcdFlags flags)
{
#if defined(BOOT)
  return LV_FONT_DEFAULT;
#else
  auto fontIndex = FONT_INDEX(flags);
  if (fontIndex >= FONTS_COUNT) return LV_FONT_DEFAULT;
  decompressFont(fontIndex);
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
  return getFontHeight(flags) + FontHeightCorrection[FONT_INDEX(flags)];
}

int getTextWidth(const char* s, int len, LcdFlags flags)
{
  auto font = getFont(flags);
  lv_coord_t letter_space = 0;
  if (!len) len = strlen(s);
  return lv_txt_get_width(s, len, font, letter_space, LV_TEXT_FLAG_EXPAND);
}
