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

#pragma once

#include <inttypes.h>

#include "edgetx_types.h"
#include "board.h"

constexpr coord_t BOX_WIDTH         = 23;
constexpr coord_t BOX_CENTERY       = LCD_H - BOX_WIDTH/2 - 9;
constexpr coord_t MARKER_WIDTH      = 5;
#define CENTER_OFS                    0

#define BITMAP_BUFFER_SIZE(w, h)      (2 + (w) * (((h)+7)/8))
#define DISPLAY_BUFFER_SIZE           (LCD_W*((LCD_H+7)/8))

#include "lcd_common.h"

void drawTimerWithMode(coord_t x, coord_t y, uint8_t index, LcdFlags att);

inline void lcdDrawBitmap(coord_t x, coord_t y, const uint8_t * bitmap)
{
  lcdDraw1bitBitmap(x, y, bitmap, 0);
}

uint8_t * lcdLoadBitmap(uint8_t * dest, const char * filename, uint8_t width, uint8_t height);

inline pixel_t getPixel(uint8_t x, uint8_t y)
{
  if (x>=LCD_W || y>=LCD_H) {
    return 0;
  }

  pixel_t pixel = displayBuf[(y / 8) * LCD_W + x];
  pixel_t mask = 1 << (y & 7);
  return ((pixel & mask) ? 0xf : 0);
}
