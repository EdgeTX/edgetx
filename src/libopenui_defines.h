/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef _LIBOPEUI_DEFINES_H_
#define _LIBOPEUI_DEFINES_H_

#include "libopenui_types.h"

/* lcd common flags */
#define BLINK                          0x01u

/* drawText flags */
#define INVERS                         0x02u
#define LEFT                           0x00u /* align left */
#define CENTERED                       0x04u /* align center */
#define RIGHT                          0x08u /* align right */
#define SHADOWED                       0x80u /* black copy at +1 +1 */

/* drawNumber flags */
#define LEADING0                       0x10u
#define PREC1                          0x20u
#define PREC2                          0x30u
#define MODE(flags)                    ((((int8_t)(flags) & 0x30) - 0x10) >> 4)

/* telemetry flags */
#define NO_UNIT                        0x40u

#define FONT_MASK                       0x0F00u
#define FONT_INDEX(flags)               (((flags) & FONT_MASK) >> 8u)
#define FONT(xx)                        (unsigned(FONT_ ## xx ## _INDEX) << 8u)

#define TIMEHOUR                       0x2000u
#define EXPANDED                       0x2000u
#define VERTICAL                       0x4000u

#define ARGB_SPLIT(color, a, r, g, b) \
  uint16_t a = ((color) & 0xF000) >> 12; \
  uint16_t r = ((color) & 0x0F00) >> 8; \
  uint16_t g = ((color) & 0x00F0) >> 4; \
  uint16_t b = ((color) & 0x000F)

#define RGB_SPLIT(color, r, g, b) \
  uint16_t r = ((color) & 0xF800) >> 11; \
  uint16_t g = ((color) & 0x07E0) >> 5; \
  uint16_t b = ((color) & 0x001F)

#define RGB_JOIN(r, g, b) \
  (((r) << 11) + ((g) << 5) + (b))

#define GET_RED(color) \
  (((color) & 0xF800) >> 8)

#define GET_GREEN(color) \
  (((color) & 0x07E0) >> 3)

#define GET_BLUE(color) \
  (((color) & 0x001F) << 3)

#if defined(__MINGW32__) || !defined(__GNUC__)
  #include <windows.h>
  #include <tchar.h>
  #define sleep(x) Sleep(x)
  #define strcasecmp  _stricmp
  #define strncasecmp _strnicmp
  #define chdir  _chdir
  #define getcwd _getcwd
  // remove windows default definitions
  #undef OPAQUE
  #undef RGB
  #undef EXTERN_C
#endif

#define OPACITY_MAX                    0x0Fu
#define OPACITY(value)                 ((value) << 24u)

#define RGB(r, g, b)                   (uint16_t)((((r) & 0xF8) << 8) + (((g) & 0xFC) << 3) + (((b) & 0xF8) >> 3))
#define ARGB(a, r, g, b)               (uint16_t)((((a) & 0xF0) << 8) + (((r) & 0xF0) << 4) + (((g) & 0xF0) << 0) + (((b) & 0xF0) >> 4))

#define COLOR(index)                   LcdFlags(unsigned(index) << 16u)
#define COLOR_IDX(att)                 uint8_t((att) >> 16u)

#define DEFAULT_COLOR                  COLOR(DEFAULT_COLOR_INDEX)
#define DEFAULT_BGCOLOR                COLOR(DEFAULT_BGCOLOR_INDEX)
#define FOCUS_COLOR                    COLOR(FOCUS_COLOR_INDEX)
#define FOCUS_BGCOLOR                  COLOR(FOCUS_BGCOLOR_INDEX)
#define DISABLE_COLOR                  COLOR(DISABLE_COLOR_INDEX)
#define HIGHLIGHT_COLOR                COLOR(HIGHLIGHT_COLOR_INDEX)
#define CHECKBOX_COLOR                 COLOR(CHECKBOX_COLOR_INDEX)
#define SCROLLBAR_COLOR                COLOR(SCROLLBAR_COLOR_INDEX)
#define MENU_COLOR                     COLOR(MENU_COLOR_INDEX)
#define MENU_BGCOLOR                   COLOR(MENU_BGCOLOR_INDEX)
#define MENU_TITLE_BGCOLOR             COLOR(MENU_TITLE_BGCOLOR_INDEX)
#define MENU_LINE_COLOR                COLOR(MENU_LINE_COLOR_INDEX)
#define MENU_HIGHLIGHT_COLOR           COLOR(MENU_HIGHLIGHT_COLOR_INDEX)
#define MENU_HIGHLIGHT_BGCOLOR         COLOR(MENU_HIGHLIGHT_BGCOLOR_INDEX)
#define OVERLAY_COLOR                  COLOR(OVERLAY_COLOR_INDEX)
#define TABLE_BGCOLOR                  COLOR(TABLE_BGCOLOR_INDEX)
#define TABLE_HEADER_BGCOLOR           COLOR(TABLE_HEADER_BGCOLOR_INDEX)
#define CUSTOM_COLOR                   COLOR(CUSTOM_COLOR_INDEX)

#endif
