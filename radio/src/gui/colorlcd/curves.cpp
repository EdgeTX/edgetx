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

#include "opentx.h"
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
  dc->drawSolidHorizontalLine(left, y, width*2+1, COLOR_THEME_DISABLED);
  dc->drawSolidVerticalLine(x, y-width, width*2, COLOR_THEME_DISABLED);

  // Extra lines
  dc->drawVerticalLine(left+width/2, y-width, width*2, STASHED, COLOR_THEME_DISABLED);
  dc->drawVerticalLine(right-width/2, y-width, width*2, STASHED, COLOR_THEME_DISABLED);
  dc->drawHorizontalLine(left, y-width/2, width*2+1, STASHED, COLOR_THEME_DISABLED);
  dc->drawHorizontalLine(left, y+width/2, width*2+1, STASHED, COLOR_THEME_DISABLED);

  // Outside border
  dc->drawSolidVerticalLine(left, y-width, width*2, COLOR_THEME_SECONDARY1);
  dc->drawSolidVerticalLine(right, y-width, width*2, COLOR_THEME_SECONDARY1);
  dc->drawSolidHorizontalLine(left, y-width, width*2+1, COLOR_THEME_SECONDARY1);
  dc->drawSolidHorizontalLine(left, y+width, width*2+1, COLOR_THEME_SECONDARY1);

  coord_t prev_yv = (coord_t)-1;

  for (int xv=-width; xv<=width; xv+=1) {
    coord_t yv = y + getCurveYCoord(fn, xv, width);
    if (prev_yv != (coord_t)-1) {
      if (prev_yv < yv) {
        for (int y=prev_yv; y<=yv; y+=1) {
          dc->drawBitmapPattern(x+xv-2, y-2, LBM_POINT, COLOR_THEME_SECONDARY1);
        }
      }
      else {
        for (int y=yv; y<=prev_yv; y+=1) {
          dc->drawBitmapPattern(x+xv-2, y-2, LBM_POINT, COLOR_THEME_SECONDARY1);
        }
      }
    }
    prev_yv = yv;
  }
}

void drawCurveVerticalScale(BitmapBuffer * dc, int x)
{
  for (int i=0; i<=20; i++) {
    dc->drawSolidHorizontalLine(x, CURVE_CENTER_Y-CURVE_SIDE_WIDTH+i*CURVE_SIDE_WIDTH/10, 10, COLOR_THEME_SECONDARY1);
  }
}

void drawCurveHorizontalScale(BitmapBuffer * dc)
{
  for (int i=0; i<=20; i++) {
    dc->drawSolidVerticalLine(CURVE_CENTER_X-CURVE_SIDE_WIDTH+i*CURVE_SIDE_WIDTH/10, CURVE_CENTER_Y+CURVE_SIDE_WIDTH+5, 10, COLOR_THEME_SECONDARY1);
  }
}

void drawCurveCoord(BitmapBuffer * dc, int x, int y, const char * text, bool active)
{
  dc->drawSolidFilledRect(x, y, CURVE_COORD_WIDTH, CURVE_COORD_HEIGHT, COLOR_THEME_WARNING);
  dc->drawText(x+3+(CURVE_COORD_WIDTH - 1 - getTextWidth(text, 0, FONT(XS))) / 2, y + 1, text, LEFT|FONT(XS)|COLOR_THEME_SECONDARY3);
  if (active) {
    dc->drawBitmapPattern(x, y, LBM_CURVE_COORD_SHADOW, COLOR_THEME_SECONDARY1);
  }
}

void drawCurvePoint(BitmapBuffer * dc, int x, int y, LcdFlags color)
{
  dc->drawBitmapPattern(x, y, LBM_CURVE_POINT, color);
  dc->drawBitmapPattern(x, y, LBM_CURVE_POINT_CENTER, COLOR_THEME_SECONDARY3);
}
