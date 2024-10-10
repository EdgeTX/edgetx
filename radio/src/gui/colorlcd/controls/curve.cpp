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

#include "curve.h"

#include "bitmaps.h"
#include "edgetx.h"
#include "strhelpers.h"

//-----------------------------------------------------------------------------

// Base curve rendering class to draw the background and curve.
CurveRenderer::CurveRenderer(Window* parent, const rect_t& rect,
                             std::function<int(int)> function) :
    valueFunc(std::move(function))
{
  dx = rect.x;
  dy = rect.y;
  dw = rect.w;
  dh = rect.h;

  lv_obj_t* bgBox = lv_line_create(parent->getLvObj());
  etx_obj_add_style(bgBox, styles->graph_border, LV_PART_MAIN);

  lv_obj_t* axis1 = lv_line_create(parent->getLvObj());
  etx_obj_add_style(axis1, styles->graph_border, LV_PART_MAIN);
  lv_obj_t* axis2 = lv_line_create(parent->getLvObj());
  etx_obj_add_style(axis2, styles->graph_border, LV_PART_MAIN);

  lv_obj_t* extra1 = lv_line_create(parent->getLvObj());
  etx_obj_add_style(extra1, styles->graph_dashed, LV_PART_MAIN);
  lv_obj_t* extra2 = lv_line_create(parent->getLvObj());
  etx_obj_add_style(extra2, styles->graph_dashed, LV_PART_MAIN);
  lv_obj_t* extra3 = lv_line_create(parent->getLvObj());
  etx_obj_add_style(extra3, styles->graph_dashed, LV_PART_MAIN);
  lv_obj_t* extra4 = lv_line_create(parent->getLvObj());
  etx_obj_add_style(extra4, styles->graph_dashed, LV_PART_MAIN);

  // Outer box
  bgPoints[0] = {dx, dy};
  bgPoints[1] = {(lv_coord_t)(dx + dw - 1), dy};
  bgPoints[2] = {(lv_coord_t)(dx + dw - 1), (lv_coord_t)(dy + dh - 1)};
  bgPoints[3] = {dx, (lv_coord_t)(dy + dh - 1)};
  bgPoints[4] = {dx, dy};

  lv_line_set_points(bgBox, bgPoints, 5);

  // Axis lines
  bgPoints[5] = {(lv_coord_t)(dx + dw / 2), dy};
  bgPoints[6] = {(lv_coord_t)(dx + dw / 2), (lv_coord_t)(dy + dh - 1)};
  bgPoints[7] = {dx, (lv_coord_t)(dy + dh / 2)};
  bgPoints[8] = {(lv_coord_t)(dx + dw - 1), (lv_coord_t)(dy + dh / 2)};

  lv_line_set_points(axis1, &bgPoints[5], 2);
  lv_line_set_points(axis2, &bgPoints[7], 2);

  // Extra lines
  bgPoints[9] = {(lv_coord_t)(dx + dw / 4), dy};
  bgPoints[10] = {(lv_coord_t)(dx + dw / 4), (lv_coord_t)(dy + dh - 1)};
  bgPoints[11] = {(lv_coord_t)(dx + dw * 3 / 4), dy};
  bgPoints[12] = {(lv_coord_t)(dx + dw * 3 / 4), (lv_coord_t)(dy + dh - 1)};
  bgPoints[13] = {dx, (lv_coord_t)(dy + dh / 4)};
  bgPoints[14] = {(lv_coord_t)(dx + dw - 1), (lv_coord_t)(dy + dh / 4)};
  bgPoints[15] = {dx, (lv_coord_t)(dy + dh * 3 / 4)};
  bgPoints[16] = {(lv_coord_t)(dx + dw - 1), (lv_coord_t)(dy + dh * 3 / 4)};

  lv_line_set_points(extra1, &bgPoints[9], 2);
  lv_line_set_points(extra2, &bgPoints[11], 2);
  lv_line_set_points(extra3, &bgPoints[13], 2);
  lv_line_set_points(extra4, &bgPoints[15], 2);

  // Curve points
  lnPoints = new lv_point_t[dw];

  ptLine = lv_line_create(parent->getLvObj());
  etx_obj_add_style(ptLine, styles->graph_line, LV_PART_MAIN);

  update();
}

CurveRenderer::~CurveRenderer()
{
  if (lnPoints) delete lnPoints;
}

void CurveRenderer::update()
{
  for (lv_coord_t x = 0; x < dw; x += 1) {
    lv_coord_t y =
        getPointY(valueFunc(divRoundClosest((x - dw / 2) * RESX, dw / 2)));
    lnPoints[x] = {(lv_coord_t)(x + dx), y};
  }

  lv_line_set_points(ptLine, lnPoints, dw);
}

coord_t CurveRenderer::getPointY(int y) const
{
  return dy +
         limit<coord_t>(0, dh / 2 - divRoundClosest(y * dh / 2, RESX), dh - 1);
}

//-----------------------------------------------------------------------------

Curve::Curve(Window* parent, const rect_t& rect,
             std::function<int(int)> function, std::function<int()> position) :
    Window(parent, rect),
    base(this,
         rect_t{(position ? 4 : 2), (position ? 4 : 2),
                rect.w - (position ? 8 : 4), rect.h - (position ? 8 : 4)},
         function),
    valueFunc(std::move(function)),
    positionFunc(std::move(position))
{
  setWindowFlag(NO_FOCUS);

  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);

  etx_solid_bg(lvobj, COLOR_THEME_PRIMARY2_INDEX);

  // Adjust border - if drawing points leave more space to prevent clipping of
  // end points.
  if (positionFunc) {
    dx = 4;
    dy = 4;
  } else {
    dx = 2;
    dy = 2;
  }
  dw = rect.w - dx * 2;
  dh = rect.h - dy * 2;

  for (int i = 0; i < 17; i += 1) {
    auto p = lv_obj_create(lvobj);
    etx_solid_bg(p, COLOR_THEME_PRIMARY2_INDEX);
    etx_obj_add_style(p, styles->circle, LV_PART_MAIN);
    etx_obj_add_style(p, styles->border, LV_PART_MAIN);
    etx_obj_add_style(p, styles->border_color[COLOR_THEME_SECONDARY1_INDEX], LV_PART_MAIN);
    lv_obj_set_size(p, 9, 9);
    lv_obj_add_flag(p, LV_OBJ_FLAG_HIDDEN);
    pointDots[i] = p;
  }

  if (positionFunc) {
    posVLine = lv_line_create(lvobj);
    etx_obj_add_style(posVLine, styles->graph_position_line, LV_PART_MAIN);
    posHLine = lv_line_create(lvobj);
    etx_obj_add_style(posHLine, styles->graph_position_line, LV_PART_MAIN);

    positionValue = new StaticText(this, {10, 10, LV_SIZE_CONTENT, 17}, "", COLOR_THEME_PRIMARY1_INDEX, FONT(XS));
    positionValue->padLeft(PAD_TINY);
    positionValue->padRight(PAD_TINY);
    etx_solid_bg(positionValue->getLvObj(), COLOR_THEME_ACTIVE_INDEX);

    posPoint = lv_obj_create(lvobj);
    etx_solid_bg(posPoint, COLOR_THEME_PRIMARY2_INDEX);
    etx_obj_add_style(posPoint, styles->circle, LV_PART_MAIN);
    etx_obj_add_style(posPoint, styles->border, LV_PART_MAIN);
    etx_obj_add_style(posPoint, styles->border_color[COLOR_THEME_ACTIVE_INDEX], LV_PART_MAIN);
    lv_obj_set_size(posPoint, 9, 9);

    updatePosition();
  }
}

coord_t Curve::getPointX(int x) const
{
  return dx +
         limit<coord_t>(0, dw / 2 + divRoundClosest(x * dw / 2, RESX), dw - 1);
}

coord_t Curve::getPointY(int y) const
{
  return dy +
         limit<coord_t>(0, dh / 2 - divRoundClosest(y * dh / 2, RESX), dh - 1);
}

void Curve::updatePosition()
{
  if (positionFunc) {
    int valueX = positionFunc();
    int valueY = valueFunc(valueX);

    char coords[16];
    strAppendSigned(
        strAppend(strAppendSigned(coords, calcRESXto100(valueX)), ","),
        calcRESXto100(valueY));

    positionValue->setText(coords);

    lv_coord_t x = getPointX(valueX);
    lv_coord_t y = getPointY(valueY);

    lv_obj_set_pos(posPoint, x - 4, y - 4);

    posLinePoints[0] = {x, dy};
    posLinePoints[1] = {x, (lv_coord_t)(dy + dh - 1)};
    posLinePoints[2] = {dx, y};
    posLinePoints[3] = {(lv_coord_t)(dx + dw - 1), y};

    lv_line_set_points(posVLine, &posLinePoints[0], 2);
    lv_line_set_points(posHLine, &posLinePoints[2], 2);
  }
}

void Curve::addPoint(const point_t& point)
{
  int i = points.size();
  coord_t x = getPointX(point.x);
  coord_t y = getPointY(point.y);
  lv_obj_set_pos(pointDots[i], x - 4, y - 4);
  lv_obj_clear_flag(pointDots[i], LV_OBJ_FLAG_HIDDEN);

  points.push_back(point);

  update();
}

void Curve::clearPoints()
{
  points.clear();
  for (int i = 0; i < 17; i += 1)
    lv_obj_add_flag(pointDots[i], LV_OBJ_FLAG_HIDDEN);

  update();
}

void Curve::update()
{
  base.update();
  updatePosition();
}

void Curve::checkEvents()
{
  // Redraw if crosshair position has changed
  if (positionFunc) {
    int pos = positionFunc();
    if (pos != lastPos) {
      lastPos = pos;
      updatePosition();
    }
  }

  Window::checkEvents();
}
