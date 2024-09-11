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

constexpr coord_t BOX_WIDTH         = 31;
constexpr coord_t BOX_CENTERY       = LCD_H - BOX_WIDTH / 2 - 10;
constexpr coord_t MARKER_WIDTH      = 5;
#define CENTER_OFS                    (7 * FW - FW / 2)

#define BITMAP_BUFFER_SIZE(w, h)      (2 + (w) * (((h)+7)/8)*4)
#define DISPLAY_BUFFER_SIZE           (LCD_W*LCD_H*4/8)

#define GREY(x)                       ((x) * 0x010000)
#define WHITE                         GREY(0xf)
#define GREY_DEFAULT                  GREY(11)
#define COLOUR_MASK(x)                ((x) & 0x0F0000)

#include "lcd_common.h"

void drawMainControlLabel(coord_t x, coord_t y, uint8_t idx, LcdFlags att=0);

void lcdDrawBitmap(coord_t x, coord_t y, const uint8_t * img, coord_t offset=0, coord_t width=0);
void lcdDrawRleBitmap(coord_t x, coord_t y, const uint8_t * img, coord_t offset=0, coord_t width=0, bool overlay=false);
#define LCD_ICON(x, y, icon) lcdDrawRleBitmap(x, y, icons, icon)

uint8_t * lcdLoadBitmap(uint8_t * dest, const char * filename, uint16_t width, uint16_t height);

inline pixel_t getPixel(unsigned int x, unsigned int y)
{
  if (x>=LCD_W || y>=LCD_H) {
    return 0;
  }

  pixel_t * p = &displayBuf[y / 2 * LCD_W + x];
  return (y & 1) ? (*p >> 4) : (*p & 0x0F);
}

class RleBitmap
{
public:
  RleBitmap(const uint8_t *src, coord_t offset) :
    state(RLE_FIRST_BYTE), src(src), curPtr(src), byte(0), curCount(0), pos(0)
  {
    width = *curPtr++;
    rawRows = *curPtr++;
    rows = (rawRows +1)/2;
    skip(offset);
  }

  void skip(coord_t count)
  {
    while(count)
    {
      count--;
      getNext();
    }
  }

  uint8_t getNext()
  {
    pos++;
    switch(state)
    {
    case RLE_FIRST_BYTE:
      byte = *curPtr++;
      if(byte == *curPtr)
        state = RLE_SECOND_BYTE;
      break;
    case RLE_SECOND_BYTE:
      byte = *curPtr++;
      curCount = (*curPtr++)+1;
      state = RLE_CONTINUE;
      // fall through
    case RLE_CONTINUE:
      curCount--;
      if(!curCount)
        state = RLE_FIRST_BYTE;
      break;
    }
    return byte;
  }

  uint8_t getWidth() const { return width; }
  uint8_t getRows() const { return rows; }
  uint8_t getRawRows() const { return rawRows; }
  void goToNextRow()
  {
    coord_t offset = pos%width;
    if(offset)
      skip(width - pos%width);
  }

  void reset() { curPtr = src; }
  
private:
  enum State {RLE_FIRST_BYTE, RLE_SECOND_BYTE, RLE_CONTINUE} state;
  const uint8_t* src;
  const uint8_t* curPtr;

  uint8_t width;
  uint8_t rows;
  uint8_t rawRows;

  uint8_t byte;
  uint16_t curCount;

  coord_t pos;
};
