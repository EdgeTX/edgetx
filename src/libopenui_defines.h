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
#define BLINK                          0x01

/* drawText flags */
#define INVERS                         0x02
#define LEFT                           0x00 /* align left */
#define CENTERED                       0x04 /* align center */
#define RIGHT                          0x08 /* align right */
#define SHADOWED                       0x80 /* black copy at +1 +1 */

/* drawNumber flags */
#define LEADING0                       0x10
#define PREC1                          0x20
#define PREC2                          0x30
#define MODE(flags)                    ((((int8_t)(flags) & 0x30) - 0x10) >> 4)

/* rect, square flags */
#define ROUND                          0x04

/* telemetry flags */
#define NO_UNIT                        0x40

enum FontSizeIndex
{
  STDSIZE_INDEX,
  TINSIZE_INDEX,
  SMLSIZE_INDEX,
  MIDSIZE_INDEX,
  DBLSIZE_INDEX,
  XXLSIZE_INDEX,
  SPARE6_INDEX,
  SPARE7_INDEX,
  STDSIZE_BOLD_INDEX,
  SPARE9_INDEX,
  SPAREa_INDEX,
  SPAREb_INDEX,
  SPAREc_INDEX,
  SPAREd_INDEX,
  SPAREe_INDEX,
  SPAREf_INDEX,
};

#define STDSIZE                        (STDSIZE_INDEX << 8)
#define TINSIZE                        (TINSIZE_INDEX << 8)
#define SMLSIZE                        (SMLSIZE_INDEX << 8)
#define MIDSIZE                        (MIDSIZE_INDEX << 8)
#define DBLSIZE                        (DBLSIZE_INDEX << 8)
#define XXLSIZE                        (XXLSIZE_INDEX << 8)
#define BOLD                           (STDSIZE_BOLD_INDEX << 8)
#define FONTSIZE_MASK                  0x0f00

#if !defined(BOOT)
#define FONTSIZE(flags)                ((flags) & FONTSIZE_MASK)
#define FONTINDEX(flags)               (FONTSIZE(flags) >> 8)
#else
#define FONTSIZE(flags)                STDSIZE
#define FONTINDEX(flags)               STDSIZE_INDEX
#endif

#define TIMEBLINK                      0x1000
#define TIMEHOUR                       0x2000
#define EXPANDED                       0x2000
#define VERTICAL                       0x4000

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

// remove windows default definitions
#undef OPAQUE
#undef RGB

#define OPACITY_MAX                    0x0F
#define OPACITY(x)                     ((x)<<24)

#define RGB(r, g, b)                   (uint16_t)((((r) & 0xF8) << 8) + (((g) & 0xFC) << 3) + (((b) & 0xF8) >> 3))
#define ARGB(a, r, g, b)               (uint16_t)((((a) & 0xF0) << 8) + (((r) & 0xF0) << 4) + (((g) & 0xF0) << 0) + (((b) & 0xF0) >> 4))

#define COLOR(index)                   LcdFlags((index) << 16)
#define COLOR_IDX(att)                 uint8_t((att) >> 16)

#define DEFAULT_COLOR                  COLOR(DEFAULT_COLOR_INDEX)
#define DEFAULT_BGCOLOR                COLOR(DEFAULT_BGCOLOR_INDEX)
#define FOCUS_COLOR                    COLOR(FOCUS_COLOR_INDEX)
#define FOCUS_BGCOLOR                  COLOR(FOCUS_BGCOLOR_INDEX)
#define DISABLE_COLOR                  COLOR(DISABLE_COLOR_INDEX)
#define HIGHLIGHT_COLOR                COLOR(HIGHLIGHT_COLOR_INDEX)
#define CHECKBOX_COLOR                 COLOR(CHECKBOX_COLOR_INDEX)
#define MENU_COLOR                     COLOR(MENU_COLOR_INDEX)
#define MENU_BGCOLOR                   COLOR(MENU_BGCOLOR_INDEX)
#define CUSTOM_COLOR                   COLOR(CUSTOM_COLOR_INDEX)

#endif
