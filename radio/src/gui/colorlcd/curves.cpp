/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "edgetx.h"
#include "font.h"

coord_t getCurveYCoord(FnFuncP fn, int x, int width)
{
  return limit(-width, -divRoundClosest(fn(divRoundClosest(x * RESX, width)) * width, RESX), +width);
}

void drawFunction(BitmapBuffer * dc, FnFuncP fn, int x, int y, int width)
{
  int left = x - width;
  int right = x + width;

  // Axis
  dc->drawSolidHorizontalLine(left, y, width*2+1, DISABLE_COLOR);
  dc->drawSolidVerticalLine(x, y-width, width*2, DISABLE_COLOR);

  // Extra lines
  dc->drawVerticalLine(left+width/2, y-width, width*2, STASHED, DISABLE_COLOR);
  dc->drawVerticalLine(right-width/2, y-width, width*2, STASHED, DISABLE_COLOR);
  dc->drawHorizontalLine(left, y-width/2, width*2+1, STASHED, DISABLE_COLOR);
  dc->drawHorizontalLine(left, y+width/2, width*2+1, STASHED, DISABLE_COLOR);

  // Outside border
  dc->drawSolidVerticalLine(left, y-width, width*2, DEFAULT_COLOR);
  dc->drawSolidVerticalLine(right, y-width, width*2, DEFAULT_COLOR);
  dc->drawSolidHorizontalLine(left, y-width, width*2+1, DEFAULT_COLOR);
  dc->drawSolidHorizontalLine(left, y+width, width*2+1, DEFAULT_COLOR);

  coord_t prev_yv = (coord_t)-1;

  for (int xv=-width; xv<=width; xv+=1) {
    coord_t yv = y + getCurveYCoord(fn, xv, width);
    if (prev_yv != (coord_t)-1) {
      if (prev_yv < yv) {
        for (int y=prev_yv; y<=yv; y+=1) {
          dc->drawBitmapPattern(x+xv-2, y-2, LBM_POINT, DEFAULT_COLOR);
        }
      }
      else {
        for (int y=yv; y<=prev_yv; y+=1) {
          dc->drawBitmapPattern(x+xv-2, y-2, LBM_POINT, DEFAULT_COLOR);
        }
      }
    }
    prev_yv = yv;
  }
}

void drawCurveVerticalScale(BitmapBuffer * dc, int x)
{
  for (int i=0; i<=20; i++) {
    dc->drawSolidHorizontalLine(x, CURVE_CENTER_Y-CURVE_SIDE_WIDTH+i*CURVE_SIDE_WIDTH/10, 10, DEFAULT_COLOR);
  }
}

void drawCurveHorizontalScale(BitmapBuffer * dc)
{
  for (int i=0; i<=20; i++) {
    dc->drawSolidVerticalLine(CURVE_CENTER_X-CURVE_SIDE_WIDTH+i*CURVE_SIDE_WIDTH/10, CURVE_CENTER_Y+CURVE_SIDE_WIDTH+5, 10, DEFAULT_COLOR);
  }
}

void drawCurveCoord(BitmapBuffer * dc, int x, int y, const char * text, bool active)
{
  dc->drawSolidFilledRect(x, y, CURVE_COORD_WIDTH, CURVE_COORD_HEIGHT, CURVE_CURSOR_COLOR);
  dc->drawText(x+3+(CURVE_COORD_WIDTH - 1 - getTextWidth(text, 0, FONT(XS))) / 2, y + 1, text, LEFT|FONT(XS)|DEFAULT_BGCOLOR);
  if (active) {
    dc->drawBitmapPattern(x, y, LBM_CURVE_COORD_SHADOW, DEFAULT_COLOR);
  }
}

void drawCurvePoint(BitmapBuffer * dc, int x, int y, LcdFlags color)
{
  dc->drawBitmapPattern(x, y, LBM_CURVE_POINT, color);
  dc->drawBitmapPattern(x, y, LBM_CURVE_POINT_CENTER, DEFAULT_BGCOLOR);
}
