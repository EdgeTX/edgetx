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

#include "opentx.h"
#include "curve.h"
#include "lcd.h"
#include "bitmaps.h"
#include "strhelpers.h"
#include "font.h"

const uint8_t _LBM_CURVE_POINT[] = {
#include "mask_cvpoint.lbm"
};

const uint8_t _LBM_CURVE_POINT_CENTER[] = {
#include "mask_cvpoint_center.lbm"
};

const uint8_t _LBM_CURVE_COORD_SHADOW[] = {
#include "mask_coord_shadow.lbm"
};

IMPL_LZ4_BITMAP(LBM_CURVE_POINT);
IMPL_LZ4_BITMAP(LBM_CURVE_POINT_CENTER);
IMPL_LZ4_BITMAP(LBM_CURVE_COORD_SHADOW);

// Base curve rendering class to 'paint' the background and curve.
CurveRenderer::CurveRenderer(const rect_t & rect, std::function<int(int)> function):
                             rect(rect),
                             function(std::move(function))
{
}

coord_t CurveRenderer::getPointY(int y) const
{
  return dy + limit<coord_t>(0,
                             dh / 2 - divRoundClosest(y * dh / 2, RESX),
                             dh - 1);
}

void CurveRenderer::drawBackground(BitmapBuffer * dc)
{
  dc->drawSolidFilledRect(rect.x, rect.y, rect.w, rect.h, COLOR_THEME_PRIMARY2);

  // Axis
  dc->drawSolidHorizontalLine(dx, dy+dh/2, dw, COLOR_THEME_SECONDARY2);
  dc->drawSolidVerticalLine(dx+dw/2, dy, dh, COLOR_THEME_SECONDARY2);

  // Extra lines
  dc->drawVerticalLine(dx+dw/4, dy, dh, STASHED, COLOR_THEME_SECONDARY2);
  dc->drawVerticalLine(dx+dw*3/4, dy, dh, STASHED, COLOR_THEME_SECONDARY2);
  dc->drawHorizontalLine(dx, dy+dh/4, dw, STASHED, COLOR_THEME_SECONDARY2);
  dc->drawHorizontalLine(dx, dy+dh*3/4, dw, STASHED, COLOR_THEME_SECONDARY2);

  // Outside border
  dc->drawSolidRect(dx, dy, dw, dh, 1, COLOR_THEME_SECONDARY2);
}

void CurveRenderer::drawCurve(BitmapBuffer * dc)
{
  auto prev = (coord_t) -1;

  for (int x = 0; x < dw; x++) {
    coord_t y = getPointY(function(divRoundClosest((x - dw / 2) * RESX, dw / 2)));
    if (prev >= 0) {
      if (prev < y) {
        for (int tmp = prev; tmp <= y; tmp++) {
          dc->drawBitmapPattern(dx + x - 2, tmp - 2, LBM_POINT, COLOR_THEME_SECONDARY1);
        }
      }
      else {
        for (int tmp = y; tmp <= prev; tmp++) {
          dc->drawBitmapPattern(dx + x - 2, tmp - 2, LBM_POINT, COLOR_THEME_SECONDARY1);
        }
      }
    }
    prev = y;
  }
}

void CurveRenderer::paint(BitmapBuffer * dc, uint8_t ofst)
{
  dx = rect.x + 2 + ofst;
  dy = rect.y + 2 + ofst;
  dw = rect.w - 4 - ofst * 2;
  dh = rect.h - 4 - ofst * 2;

  drawBackground(dc);
  drawCurve(dc);
}

Curve::Curve(Window * parent, const rect_t & rect, std::function<int(int)> function, std::function<int()> position):
      Window(parent, rect, OPAQUE),
      base(rect_t{0, 0, rect.w, rect.h}, function),
      function(std::move(function)),
      position(std::move(position))
{
}

coord_t Curve::getPointX(int x) const
{
  return dx + limit<coord_t>(0,
                             dw / 2 + divRoundClosest(x * dw / 2, RESX),
                             dw - 1);
}

coord_t Curve::getPointY(int y) const
{
  return dy + limit<coord_t>(0,
                             dh / 2 - divRoundClosest(y * dh / 2, RESX),
                             dh - 1);
}

void Curve::drawBackground(BitmapBuffer * dc)
{
  dc->clear(COLOR_THEME_PRIMARY2);

  // Axis
  dc->drawSolidHorizontalLine(dx, dy+dh/2, dw, COLOR_THEME_SECONDARY2);
  dc->drawSolidVerticalLine(dx+dw/2, dy, dh, COLOR_THEME_SECONDARY2);

  // Extra lines
  dc->drawVerticalLine(dx+dw/4, dy, dh, STASHED, COLOR_THEME_SECONDARY2);
  dc->drawVerticalLine(dx+dw*3/4, dy, dh, STASHED, COLOR_THEME_SECONDARY2);
  dc->drawHorizontalLine(dx, dy+dh/4, dw, STASHED, COLOR_THEME_SECONDARY2);
  dc->drawHorizontalLine(dx, dy+dh*3/4, dw, STASHED, COLOR_THEME_SECONDARY2);

  // Outside border
  dc->drawSolidRect(dx, dy, dw, dh, 1, COLOR_THEME_SECONDARY2);
}

void Curve::drawCurve(BitmapBuffer * dc)
{
  auto prev = (coord_t) -1;

  for (int x = 0; x < dw; x++) {
    coord_t y = getPointY(function(divRoundClosest((x - dw / 2) * RESX, dw / 2)));
    if (prev >= 0) {
      if (prev < y) {
        for (int tmp = prev; tmp <= y; tmp++) {
          dc->drawBitmapPattern(dx + x - 2, tmp - 2, LBM_POINT, COLOR_THEME_SECONDARY1);
        }
      }
      else {
        for (int tmp = y; tmp <= prev; tmp++) {
          dc->drawBitmapPattern(dx + x - 2, tmp - 2, LBM_POINT, COLOR_THEME_SECONDARY1);
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
  dc->drawSolidHorizontalLine(dx, y, dw, COLOR_THEME_ACTIVE);
  dc->drawSolidVerticalLine(x, dy, dh, COLOR_THEME_ACTIVE);

  // the point (white inside)
  dc->drawBitmapPattern(x-4, y-4, LBM_CURVE_POINT, COLOR_THEME_ACTIVE);
  dc->drawBitmapPattern(x-4, y-4, LBM_CURVE_POINT_CENTER, COLOR_THEME_PRIMARY2);

  char coords[16];
  strAppendSigned(strAppend(strAppendSigned(coords, calcRESXto100(valueX)), ","), calcRESXto100(valueY));
  dc->drawSolidFilledRect(10, 11, 2 + getTextWidth(coords, 0, FONT(XS)), 17, COLOR_THEME_ACTIVE);
  dc->drawText(11, 10, coords, FONT(XS) | COLOR_THEME_PRIMARY1);
}

void Curve::drawPoint(BitmapBuffer * dc, const CurvePoint & point)
{
  coord_t x = getPointX(point.coords.x);
  coord_t y = getPointY(point.coords.y);

  dc->drawBitmapPattern(x-4, y-4, LBM_CURVE_POINT, point.flags);
  dc->drawBitmapPattern(x-4, y-4, LBM_CURVE_POINT_CENTER, COLOR_THEME_SECONDARY3);
}

void Curve::paint(BitmapBuffer * dc)
{
  // Adjust border - if drawing points leave more space to prevent clipping of end points.
  if (points.size() > 0) {
    dx = 4;
    dy = 4;
  } else {
    dx = 2;
    dy = 2;
  }
  dw = width() - dx * 2;
  dh = height() - dy * 2;

  base.paint(dc, dx-2);

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

void Curve::checkEvents()
{
  // Redraw if crosshair position has changed
  if (position) {
    int pos = position();
    if (pos != lastPos) {
      lastPos = pos;
      invalidate();
    }
  }

  Window::checkEvents();
}
