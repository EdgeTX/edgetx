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

class StaticText;

//-----------------------------------------------------------------------------

class CurveRenderer
{
 public:
  CurveRenderer(Window* parent, const rect_t& rect,
                std::function<int(int)> function);
  ~CurveRenderer();

  void update();

 protected:
  // Drawing rectangle position & size
  lv_coord_t dx, dy, dw, dh;
  std::function<int(int)> valueFunc;
  lv_point_t bgPoints[17];
  lv_point_t* lnPoints = nullptr;
  lv_obj_t* ptLine = nullptr;

  coord_t getPointY(int y) const;
};

//-----------------------------------------------------------------------------

class Curve : public Window
{
 public:
  Curve(Window* parent, const rect_t& rect, std::function<int(int)> function,
        std::function<int()> position = nullptr);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "Curve"; }
#endif

  void addPoint(const point_t& point);
  void clearPoints();

  void update();

 protected:
  CurveRenderer base;
  // Drawing rectangle position & size
  uint8_t dx, dy, dw, dh;
  int lastPos = 0;
  std::function<int(int)> valueFunc;
  std::function<int()> positionFunc;
  std::list<point_t> points;
  StaticText* positionValue = nullptr;
  lv_point_t posLinePoints[4];
  lv_obj_t* posVLine = nullptr;
  lv_obj_t* posHLine = nullptr;
  lv_obj_t* posPoint = nullptr;
  lv_obj_t* pointDots[17] = { nullptr };

  void updatePosition();

  coord_t getPointX(int x) const;
  coord_t getPointY(int y) const;

  void checkEvents() override;
};
