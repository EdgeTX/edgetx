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

#include "window.h"

struct CurvePoint {
  point_t coords;
  LcdFlags flags;
};

class Curve: public Window
{
  friend class CurveEdit;

  public:
    Curve(Window * parent, const rect_t & rect, std::function<int(int)> function, std::function<int()> position=nullptr):
      Window(parent, rect, OPAQUE),
      function(std::move(function)),
      position(std::move(position))
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "Curve";
    }
#endif

    void checkEvents() override
    {
      // will always force a full window refresh
      if (position) {
        // int pos = position();
        // if (pos != lastPos) {
        //   lastPos = pos;
          invalidate();
        // }
      }

      Window::checkEvents();
    }

    void addPoint(const point_t & point, LcdFlags flags);

    void clearPoints();

    void paint(BitmapBuffer * dc) override;

  protected:
    // int lastPos = 0;
    std::function<int(int)> function;
    std::function<int()> position;
    std::list<CurvePoint> points;
    void drawBackground(BitmapBuffer * dc);
    void drawCurve(BitmapBuffer * dc);
    void drawPosition(BitmapBuffer * dc);
    void drawPoint(BitmapBuffer * dc, const CurvePoint & point);
    coord_t getPointX(int x) const;
    coord_t getPointY(int y) const;
};

#include "lz4_bitmaps.h"

DEFINE_LZ4_BITMAP(LBM_CURVE_POINT);
DEFINE_LZ4_BITMAP(LBM_CURVE_POINT_CENTER);
DEFINE_LZ4_BITMAP(LBM_CURVE_COORD_SHADOW);
