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
#include "curve.h"
#include "lcd.h"
#include "bitmaps.h"
#include "strhelpers.h"
#include "font.h"

void Curve::drawBackground(BitmapBuffer * dc)
{
  lcdSetColor(RGB(0xE0, 0xE0, 0xE0));
  dc->clear(CUSTOM_COLOR);

  // Axis
  dc->drawSolidHorizontalLine(0, height()/2, width(), DISABLE_COLOR);
  dc->drawSolidVerticalLine(width()/2, 0, height(), DISABLE_COLOR);

  // Extra lines
  dc->drawVerticalLine(width()/4, 0, height(), STASHED, DISABLE_COLOR);
  dc->drawVerticalLine(width()*3/4, 0, height(), STASHED, DISABLE_COLOR);
  dc->drawHorizontalLine(0, height()/4, width(), STASHED, DISABLE_COLOR);
  dc->drawHorizontalLine(0, height()*3/4, width(), STASHED, DISABLE_COLOR);

  // Outside border
  // dc->drawSolidRect(0, 0, width(), height(), 1, DEFAULT_COLOR);
}

coord_t Curve::getPointX(int x) const
{
  return limit<coord_t>(0,
                        width() / 2 + divRoundClosest(x * width() / 2, RESX),
                        width() - 1);
}

coord_t Curve::getPointY(int y) const
{
  return limit<coord_t>(0,
                        height() / 2 - divRoundClosest(y * height() / 2, RESX),
                        height() - 1);
}

void Curve::drawCurve(BitmapBuffer * dc)
{
  auto prev = (coord_t) -1;

  for (int x = 0; x < width(); x++) {
    coord_t y = getPointY(function(divRoundClosest((x - width() / 2) * RESX, width() / 2)));
    if (prev >= 0) {
      if (prev < y) {
        for (int tmp = prev; tmp <= y; tmp++) {
          dc->drawBitmapPattern(x - 2, tmp - 2, LBM_POINT, DEFAULT_COLOR);
        }
      }
      else {
        for (int tmp = y; tmp <= prev; tmp++) {
          dc->drawBitmapPattern(x - 2, tmp - 2, LBM_POINT, DEFAULT_COLOR);
        }
      }
    }
    prev = y;
  }
}

void Curve::drawPosition(BitmapBuffer * dc)
{
  int valueX = position();
  int valueY = function(valueX);

  coord_t x = getPointX(valueX);
  coord_t y = getPointY(valueY);

  // the 2 lines
  dc->drawSolidHorizontalLine(0, y, width(), CURVE_CURSOR_COLOR);
  dc->drawSolidVerticalLine(x, 0, height(), CURVE_CURSOR_COLOR);

  // the point (white inside)
  dc->drawBitmapPattern(x-4, y-4, LBM_CURVE_POINT, CURVE_CURSOR_COLOR);
  dc->drawBitmapPattern(x-4, y-4, LBM_CURVE_POINT_CENTER, DEFAULT_BGCOLOR);

  char coords[16];
  strAppendSigned(strAppend(strAppendSigned(coords, calcRESXto100(valueX)), ","), calcRESXto100(valueY));
  dc->drawSolidFilledRect(10, 11, 1 + getTextWidth(coords, 0, FONT(XS)), 17, CURVE_CURSOR_COLOR);
  dc->drawText(11, 10, coords, FONT(XS)|DEFAULT_BGCOLOR);
}

void Curve::drawPoint(BitmapBuffer * dc, const CurvePoint & point)
{
  coord_t x = getPointX(point.coords.x);
  coord_t y = getPointY(point.coords.y);

  dc->drawBitmapPattern(x-4, y-4, LBM_CURVE_POINT, point.flags);
  dc->drawBitmapPattern(x-4, y-4, LBM_CURVE_POINT_CENTER, DEFAULT_BGCOLOR);
}

void Curve::paint(BitmapBuffer * dc)
{
  drawBackground(dc);
  drawCurve(dc);
  for (auto point: points) {
    drawPoint(dc, point);
  }
  if (position) {
    drawPosition(dc);
  }
}

void Curve::addPoint(const point_t & point, LcdFlags flags)
{
  points.push_back({point, flags});
  invalidate();
}

void Curve::clearPoints()
{
  points.clear();
  invalidate();
}
