/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#pragma once

#include <lvgl/lvgl.h>

#include "edgetx_types.h"

enum FontIndex {
  FONT_STD_INDEX,
  FONT_BOLD_INDEX,
  FONT_XXS_INDEX,
  FONT_XS_INDEX,
  FONT_L_INDEX,
  FONT_XL_INDEX,
  FONT_XXL_INDEX,

  // this one MUST be last
  FONTS_COUNT
};

#define FONT_MASK 0x0F00u
#define FONT_INDEX(flags) ((FontIndex)(((flags) & FONT_MASK) >> 8u))
#define FONT(xx) (unsigned(FONT_##xx##_INDEX) << 8u)

const lv_font_t* getFont(LcdFlags flags);
uint8_t getFontHeight(LcdFlags flags);
uint8_t getFontHeightCondensed(LcdFlags flags);
int getTextWidth(const char* s, int len = 0, LcdFlags flags = 0);
