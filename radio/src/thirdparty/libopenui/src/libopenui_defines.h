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

#pragma once

#include "libopenui_types.h"
#include "libopenui_compat.h"

/* obsolete flags */
#define BLINK                          0
#define TIMEHOUR                       0

/* drawText flags */
#define LEFT                           0x00u /* align left */
// 0x01u used by Lua in api_colorlcd.h
#define VCENTERED                      0x02u /* align center vertically */
#define CENTERED                       0x04u /* align center */
#define RIGHT                          0x08u /* align right */
#define SHADOWED                       0x80u /* black copy at +1 +1 */
// 0x1000u used by Lua in api_colorlcd.h
#define SPACING_NUMBERS_CONST          0x2000u
// 0x8000u used by Lua in api_colorlcd.h

/* drawNumber flags */
#define LEADING0                       0x10u
#define PREC1                          0x20u
#define PREC2                          0x30u
#define MODE(flags)                    ((((int8_t)(flags) & 0x30) - 0x10) >> 4)

/* telemetry flags */
#define NO_UNIT                        0x40u

#define FONT_MASK                      0x0F00u
#define FONT_INDEX(flags)              (((flags) & FONT_MASK) >> 8u)
#define FONT(xx)                       (unsigned(FONT_ ## xx ## _INDEX) << 8u)

#define ARGB_SPLIT(color, a, r, g, b) \
  uint16_t a = ((color) & 0xF000) >> 12; \
  uint16_t r = ((color) & 0x0F00) >> 8; \
  uint16_t g = ((color) & 0x00F0) >> 4; \
  uint16_t b = ((color) & 0x000F)

#define RGB_SPLIT(color, r, g, b) \
  uint16_t r = ((color) & 0xF800) >> 11; \
  uint16_t g = ((color) & 0x07E0) >> 5; \
  uint16_t b = ((color) & 0x001F)

#define ARGB_JOIN(a, r, g, b) \
  (((a&0xF) << 12) + ((r&0xF) << 8) + ((g&0xF) << 4) + (b&0xF))

#define RGB_JOIN(r, g, b) \
  (((r) << 11) + ((g) << 5) + (b))

#define GET_RED(color) \
  (((color) & 0xF800) >> 8)

#define GET_GREEN(color) \
  (((color) & 0x07E0) >> 3)

#define GET_BLUE(color) \
  (((color) & 0x001F) << 3)

#define OPACITY_MAX                    0x0Fu
#define OPACITY(value)                 ((value) & OPACITY_MAX)

#define RGB(r, g, b)                   (uint16_t)((((r) & 0xF8) << 8) + (((g) & 0xFC) << 3) + (((b) & 0xF8) >> 3))
#define ARGB(a, r, g, b)               (uint16_t)((((a) & 0xF0) << 8) + (((r) & 0xF0) << 4) + (((g) & 0xF0) << 0) + (((b) & 0xF0) >> 4))

#define COLOR2FLAGS(color)             LcdFlags(unsigned(color) << 16u)
#define COLOR_VAL(flags)               ((flags) >> 16u)
#define COLOR_MASK(flags)              ((flags) & 0xFFFF0000u)

