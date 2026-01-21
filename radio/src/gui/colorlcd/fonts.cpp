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
#if !defined(SIMU)
#include <unistd.h>
#endif

#include "fonts.h"
#include "lz4/lz4.h"
#include "lz4_fonts.h"
#include "translations/tts/tts.h"

#if !defined(BOOT)

extern "C" {

// All languages uss the same font for XXL size
extern const etxLz4Font lv_font_en_bold_XXL;

} // extern "C"

struct etxLvglFont {
  const etxLz4Font* lz4Font;
  lv_font_t*        lvglFont;
  bool              loaded;
};

static bool fontAllocFailed = false;

#define BUFSIZE(x) (((x) + 15) & 0xFFFFFFF0)

#if !defined(ALL_LANGS)

extern "C" {

#define FONT_TABLE(x)                                   \
  extern const lv_font_t lv_font_##x##_STD;             \
  extern const etxLz4Font lv_font_##x##_bold_STD;       \
  extern const etxLz4Font lv_font_##x##_XXS;            \
  extern const etxLz4Font lv_font_##x##_XS;             \
  extern const etxLz4Font lv_font_##x##_L;              \
  extern const etxLz4Font lv_font_##x##_bold_XL;        \
  static etxLvglFont fontTable[FONTS_COUNT] = { \
      { nullptr,       (lv_font_t*)&lv_font_##x##_STD,  true },   /* FONT_STD_INDEX */ \
      { &lv_font_##x##_bold_STD,    nullptr,            false },  /* FONT_BOLD_INDEX */ \
      { &lv_font_##x##_XXS,         nullptr,            false },  /* FONT_XXS_INDEX */ \
      { &lv_font_##x##_XS,          nullptr,            false },  /* FONT_XS_INDEX */ \
      { &lv_font_##x##_L,           nullptr,            false },  /* FONT_L_INDEX */ \
      { &lv_font_##x##_bold_XL,     nullptr,            false },  /* FONT_XL_INDEX */ \
      { &lv_font_en_bold_XXL,       nullptr,            false },  /* FONT_XXL_INDEX */ \
  };

#if defined(TRANSLATIONS_CN)
FONT_TABLE(cn)
#define ENABLE_FALLBACK
#elif defined(TRANSLATIONS_TW)
FONT_TABLE(tw)
#define ENABLE_FALLBACK
#elif defined(TRANSLATIONS_JP)
FONT_TABLE(jp)
#define ENABLE_FALLBACK
#elif defined(TRANSLATIONS_KO)
FONT_TABLE(ko)
#define ENABLE_FALLBACK
#elif defined(TRANSLATIONS_HE)
FONT_TABLE(he)
#define ENABLE_FALLBACK
#elif defined(TRANSLATIONS_RU)
FONT_TABLE(ru)
#define ENABLE_FALLBACK
#elif defined(TRANSLATIONS_UA)
FONT_TABLE(ua)
#define ENABLE_FALLBACK
#else
FONT_TABLE(en)
#endif

#if defined(ENABLE_FALLBACK)
// Fallback fonts - language fonts only contain language specific characters
extern const lv_font_t lv_font_en_STD;
extern const etxLz4Font lv_font_en_bold_STD;
extern const etxLz4Font lv_font_en_XXS;
extern const etxLz4Font lv_font_en_XS;
extern const etxLz4Font lv_font_en_L;
extern const etxLz4Font lv_font_en_bold_XL;
static etxLvglFont en_fontTable[FONTS_COUNT] = {
  { nullptr,     (lv_font_t*)&lv_font_en_STD, true },   /* FONT_STD_INDEX */
  { &lv_font_en_bold_STD,    nullptr,         false },  /* FONT_BOLD_INDEX */
  { &lv_font_en_XXS,         nullptr,         false },  /* FONT_XXS_INDEX */
  { &lv_font_en_XS,          nullptr,         false },  /* FONT_XS_INDEX */
  { &lv_font_en_L,           nullptr,         false },  /* FONT_L_INDEX */
  { &lv_font_en_bold_XL,     nullptr,         false },  /* FONT_XL_INDEX */
  { nullptr,                 nullptr,         true },   /* FONT_XXL_INDEX */
};
#endif

} // extern "C"

int getSize(etxLvglFont* fonts)
{
  int sz = 0;
  for (int i = FONT_STD_INDEX; i < FONTS_COUNT; i += 1) {
    if (fonts[i].lz4Font) {
      sz += BUFSIZE(fonts[i].lz4Font->lvglFontBufSize);
    }
  }
  return sz;
}

uint8_t* allocBuf(etxLvglFont* fonts, uint8_t* b)
{
  for (int i = FONT_STD_INDEX; i < FONTS_COUNT; i += 1) {
    if (fonts[i].lz4Font) {
      fonts[i].lvglFont = (lv_font_t*)b;
      b += BUFSIZE(fonts[i].lz4Font->lvglFontBufSize);
    }
  }
  return b;
}

void forceStd(etxLvglFont* fonts)
{
  for (int i = FONT_STD_INDEX; i < FONTS_COUNT; i += 1) {
    if (fonts[i].lz4Font) {
      fonts[i].lvglFont = fonts[FONT_STD_INDEX].lvglFont;
      fonts[i].loaded = true;
    }
  }
}

void initFontBuffers()
{
  if (fontTable[FONT_BOLD_INDEX].lvglFont || fontAllocFailed) return;

  // Calc max size needed for all compressed fonts
  int sz = 0;
#if defined(ENABLE_FALLBACK)
  sz += getSize(en_fontTable);
#endif
  sz += getSize(fontTable);

  // Allocate buffer and assign to fonts
#if defined(SIMU)
  uint8_t* b = (uint8_t*)malloc(sz);
#else
  uint8_t* b = (uint8_t*)sbrk(sz);
#endif
  if (b) {
#if defined(ENABLE_FALLBACK)
    b = allocBuf(en_fontTable, b);
#endif
    b = allocBuf(fontTable, b);
  } else {
    // Force all fonts to use STD size
    fontAllocFailed = true;
#if defined(ENABLE_FALLBACK)
    forceStd(en_fontTable);
#endif
    forceStd(fontTable);
  }
}

#else

extern "C" {

#define ENABLE_FALLBACK

#define FONT_TABLE(x)                                 \
  extern const lv_font_t lv_font_##x##_STD;           \
  extern const etxLz4Font lv_font_##x##_bold_STD;     \
  extern const etxLz4Font lv_font_##x##_XXS;          \
  extern const etxLz4Font lv_font_##x##_XS;           \
  extern const etxLz4Font lv_font_##x##_L;            \
  extern const etxLz4Font lv_font_##x##_bold_XL;      \
  static etxLvglFont x##_fontTable[FONTS_COUNT] = { \
      { nullptr,       (lv_font_t*)&lv_font_##x##_STD,  true },   /* FONT_STD_INDEX */ \
      { &lv_font_##x##_bold_STD,    nullptr,            false },  /* FONT_BOLD_INDEX */ \
      { &lv_font_##x##_XXS,         nullptr,            false },  /* FONT_XXS_INDEX */ \
      { &lv_font_##x##_XS,          nullptr,            false },  /* FONT_XS_INDEX */ \
      { &lv_font_##x##_L,           nullptr,            false },  /* FONT_L_INDEX */ \
      { &lv_font_##x##_bold_XL,     nullptr,            false },  /* FONT_XL_INDEX */ \
      { &lv_font_en_bold_XXL,       nullptr,            false },  /* FONT_XXL_INDEX */ \
  };

FONT_TABLE(en);
FONT_TABLE(cn);
FONT_TABLE(tw);
FONT_TABLE(jp);
FONT_TABLE(ko);
FONT_TABLE(he);
FONT_TABLE(ru);
FONT_TABLE(ua);

} // extern "C"

// Must match RadioLanguage order
etxLvglFont* etxFonts[] = {
  cn_fontTable,   // CN
  en_fontTable,
  en_fontTable,
  en_fontTable,
  en_fontTable,
  en_fontTable,
  en_fontTable,
  en_fontTable,
  he_fontTable,   // HE
  en_fontTable,
  en_fontTable,
  jp_fontTable,   // JP
  ko_fontTable,   // KO
  en_fontTable,
  en_fontTable,
  en_fontTable,
  ru_fontTable,   // RU
  en_fontTable,
  en_fontTable,
  tw_fontTable,   // TW
  ua_fontTable,   // UA
};

etxLvglFont* fontTable = en_fontTable;

extern void setAllFonts();

void setLanguageFont(int idx)
{
  if (fontAllocFailed) return;

  if (fontTable != etxFonts[idx]) {
    fontTable = etxFonts[idx];
    if (fontTable != en_fontTable) {
      // Force fonts to be decompressed.
      for (int i = FONT_STD_INDEX; i < FONTS_COUNT; i += 1) {
        if ((fontTable[i].lz4Font != nullptr) && (i != FONT_XXL_INDEX)) {
          fontTable[i].loaded = false;
        }
      }
    }
    setAllFonts();
  }
}

uint32_t getMaxFontSize(int fontIdx)
{
  uint32_t max = 0;
  for (int l = 0; l < LANG_COUNT; l += 1) {
    if (etxFonts[l] != en_fontTable) {
      if (etxFonts[l][fontIdx].lz4Font->lvglFontBufSize > max)
        max = etxFonts[l][fontIdx].lz4Font->lvglFontBufSize;
    }
  }
  return BUFSIZE(max);
}

void initFontBuffers()
{
  if (en_fontTable[FONT_BOLD_INDEX].lvglFont || fontAllocFailed) return;

  // Calc max size needed for all compressed fonts
  int sz = 0;
  for (int i = FONT_STD_INDEX; i < FONTS_COUNT; i += 1) {
    if (en_fontTable[i].lz4Font) {
      // EN data
      sz += BUFSIZE(en_fontTable[i].lz4Font->lvglFontBufSize);
      // Shared buf for other languages
      sz += getMaxFontSize(i);
    }
  }

  // Allocate buffer and assign to fonts
#if defined(SIMU)
  uint8_t* b = (uint8_t*)malloc(sz);
#else
  uint8_t* b = (uint8_t*)sbrk(sz);
#endif
  if (b) {
    for (int i = FONT_STD_INDEX; i < FONTS_COUNT; i += 1) {
      if (en_fontTable[i].lz4Font) {
        // EN data
        en_fontTable[i].lvglFont = (lv_font_t*)b;
        b += BUFSIZE(en_fontTable[i].lz4Font->lvglFontBufSize);
        // All languages except EN use the same buffer for the uncompressed data (only one active)
        cn_fontTable[i].lvglFont = (lv_font_t*)b;
        tw_fontTable[i].lvglFont = (lv_font_t*)b;
        jp_fontTable[i].lvglFont = (lv_font_t*)b;
        ko_fontTable[i].lvglFont = (lv_font_t*)b;
        he_fontTable[i].lvglFont = (lv_font_t*)b;
        ru_fontTable[i].lvglFont = (lv_font_t*)b;
        ua_fontTable[i].lvglFont = (lv_font_t*)b;
        b += getMaxFontSize(i);
      }
    }
  } else {
    // Force all fonts to use STD size
    fontAllocFailed = true;
    for (int i = FONT_STD_INDEX; i < FONTS_COUNT; i += 1) {
      for (int l = 0; l < LANG_COUNT; l += 1) {
        if (etxFonts[l][i].lvglFont == nullptr) {
          etxFonts[l][i].lvglFont = etxFonts[l][FONT_STD_INDEX].lvglFont;
          etxFonts[l][i].loaded = true;
        }
      }
    }
  }
}

#endif

/*
  Decompress an LZ4 font and build LVGL font structures.
*/
void decompressFont(int idx, etxLvglFont* fonts)
{
  // Check if already decompressed
  if (fonts[idx].loaded) return;

  const etxLz4Font* etxFont = fonts[idx].lz4Font;

  // Init SDRAM buffer
  initFontBuffers();
  uint8_t* data = (uint8_t*)fonts[idx].lvglFont;
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

#if defined(ENABLE_FALLBACK)
  if (fonts[idx].lz4Font != en_fontTable[idx].lz4Font) {
    decompressFont(idx, en_fontTable);
    lvglFont->fallback = en_fontTable[idx].lvglFont;
  }
#endif

  // Set LVGL font loaded flag
  fonts[idx].loaded = true;
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
  if (fontIndex >= FONTS_COUNT) return fontTable[FONT_STD_INDEX].lvglFont;
  decompressFont(fontIndex, fontTable);
  return fontTable[fontIndex].lvglFont;
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
