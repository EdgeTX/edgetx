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

#include "curveedit.h"
#include "keyboard_curve.h"
#include "opentx.h" // TODO for applyCustomCurve
#include "libopenui.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

CurveDataEdit::CurveDataEdit(Window * parent, const rect_t & rect, uint8_t index, CurveEdit * curveEdit) :
  FormGroup(parent, rect, FORM_FORWARD_FOCUS),
  index(index),
  curveEdit(curveEdit)
{
  update();
}

void CurveDataEdit::update()
{
  clear();

  FormGridLayout grid;
  grid.setLabelWidth(0);
  grid.setMarginRight(parent->width() - rect.w + 5);

  coord_t boxWidth = rect.w / 5;
  coord_t boxHeight = PAGE_LINE_HEIGHT;

  CurveHeader & curve = g_model.curves[index];
  uint8_t curvePointsCount = 5 + curve.points;

  // Point number
  for (int i = 0; i < curvePointsCount; i++) {
    auto txt = new StaticText(
        this,
        {i * boxWidth + PAGE_PADDING, 10, boxWidth - PAGE_PADDING, boxHeight},
        std::to_string(i + 1) + " ", 0, RIGHT | COLOR_THEME_SECONDARY1);
    txt->setBackgroundColor(COLOR_THEME_SECONDARY2);
  }
  grid.spacer(boxHeight + 10 + PAGE_PADDING);

  // x value
  if (curve.type == CURVE_TYPE_CUSTOM) {
    // Adjustable points for custom curves
    for (int i = 0; i < curvePointsCount; i++) {
      int8_t* points = curveAddress(index);
      auto pointEdit = new NumberEdit(
          this,
          {coord_t(PAGE_LINE_SPACING + 1 + i * boxWidth),
           grid.getWindowHeight(), coord_t(boxWidth - PAGE_LINE_SPACING),
           boxHeight},
          i <= 1 ? -100 : points[curvePointsCount + i - 2],
          i >= curvePointsCount - 2 ? 100 : points[curvePointsCount + i],
          GET_VALUE(i == 0 ? -100
                    : i == curvePointsCount - 1
                        ? 100
                        : points[curvePointsCount + i - 1]),
          [=](int32_t newValue) {
            points[curvePointsCount + i - 1] = newValue;
            SET_DIRTY();
            curveEdit->updatePreview();
          },
          0, RIGHT);

      if (i == 0 || i == curvePointsCount - 1) {
        pointEdit->disable();
      }
    }
  } else {
    for (uint8_t i = 0; i < curvePointsCount; i++) {
      new StaticText(this, {i * boxWidth, grid.getWindowHeight(), boxWidth, boxHeight},
                     std::to_string(-100 + 200 * i / (5 + curve.points - 1)), 0,
                     RIGHT | COLOR_THEME_SECONDARY1);
    }
  }
  grid.spacer(boxHeight + PAGE_PADDING);

  // y value
  for (uint8_t i = 0; i < curvePointsCount; i++) {
    int8_t* points = curveAddress(index);
    new NumberEdit(
        this,
        {coord_t(PAGE_LINE_SPACING + 1 + i * boxWidth),
         grid.getWindowHeight(),
         coord_t(boxWidth - PAGE_LINE_SPACING),
         boxHeight},
        -100, 100, GET_VALUE(points[i]),
        [=](int32_t newValue) {
          points[i] = newValue;
          SET_DIRTY();
          curveEdit->updatePreview();
        },
        0, RIGHT);
  }

}

void CurveDataEdit::paint(BitmapBuffer * dc)
{
  dc->clear(COLOR_THEME_SECONDARY3);
  // dc->drawSolidHorizontalLine(0, rect.h / 3, getInnerWidth(), 0);
  // dc->drawSolidHorizontalLine(0, 2 * rect.h / 3, getInnerWidth(), 0);
}

CurveEdit::CurveEdit(Window * parent, const rect_t & rect, uint8_t index) :
  FormField(parent, rect, NO_FOCUS),
  preview(this, {0, 0, width(), height()}, [=](int x) -> int {
    return applyCustomCurve(x, index);
  }),
  index(index),
  current(0)
{
  updatePreview();
}

void CurveEdit::updatePreview()
{
  preview.clearPoints();
  CurveHeader & curve = g_model.curves[index];
  for (uint8_t i = 0; i < 5 + curve.points; i++) {
    if (hasFocus() && current == i) {
      preview.position = [=] () -> int {
        return getPoint(index, i).x;
      };
    }
    else {
      preview.addPoint(getPoint(index, i), COLOR_THEME_SECONDARY1);
    }
  }
  invalidate();
}

// #if defined(HARDWARE_TOUCH)
// bool CurveEdit::onTouchEnd(coord_t x, coord_t y)
// {
//   if (!hasFocus()) {
//     setFocus(SET_FOCUS_DEFAULT);
//   }

//   CurveKeyboard::show(this, isCustomCurve());

//   CurveHeader & curve = g_model.curves[index];
//   for (int i=0; i<5 + curve.points; i++) {
//     if (i != current) {
//       point_t point = getPoint(index, i);
//       if (abs(preview.getPointX(point.x) - x) <= 10 && abs(preview.getPointY(point.y) - y) <= 10) {
//         current = i;
//         updatePreview();
//         break;
//       }
//     }
//   }

//   return true;
// }

// void CurveEdit::onFocusLost()
// {
//   CurveKeyboard::hide();
// }
// #endif

void CurveEdit::next()
{
  if (current++ == preview.points.size()) {
    current = 0;
  }
  updatePreview();
}

void CurveEdit::previous()
{
  if (current-- == 0) {
    current = preview.points.size();
  }
  updatePreview();
}

void CurveEdit::up()
{
  int8_t & point = curveAddress(index)[current];
  point = min<int8_t>(100, ++point);
  storageDirty(EE_MODEL);
  invalidate();
}

void CurveEdit::down()
{
  int8_t & point = curveAddress(index)[current];
  point = max<int8_t>(-100, --point);
  storageDirty(EE_MODEL);
  invalidate();
}

void CurveEdit::right()
{
  CurveHeader & curve = g_model.curves[index];
  if (curve.type == CURVE_TYPE_CUSTOM && current != 0 && current != curve.points + 5 - 1) {
    int8_t * points = curveAddress(index);
    int8_t * point = &points[5 + curve.points + current - 1];
    int8_t xmax = (current == (curve.points - 2) ? +100 : *(point + 1));
    *point = min<int8_t>(*point + 1, xmax-1);
    storageDirty(EE_MODEL);
    invalidate();
  }
}

void CurveEdit::left()
{
  CurveHeader & curve = g_model.curves[index];
  if (curve.type == CURVE_TYPE_CUSTOM && current != 0 && current != curve.points + 5 - 1) {
    int8_t * points = curveAddress(index);
    int8_t * point = &points[5 + curve.points + current - 1];
    int8_t xmin = (current == 1 ? -100 : *(point - 1));
    *point = max<int8_t>(xmin+1, *point - 1);
    storageDirty(EE_MODEL);
    invalidate();
  }
}

bool CurveEdit::isCustomCurve() const
{
  return g_model.curves[index].type == CURVE_TYPE_CUSTOM;
}

void CurveEdit::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  switch (event) {
#if defined(HARDWARE_TOUCH)
    case EVT_VIRTUAL_KEY_LEFT:
      left();
      break;

    case EVT_VIRTUAL_KEY_RIGHT:
      right();
      break;

    case EVT_VIRTUAL_KEY_UP:
      up();
      break;

    case EVT_VIRTUAL_KEY_DOWN:
      down();
      break;

    case EVT_VIRTUAL_KEY_PREVIOUS:
      previous();
      break;

    case EVT_VIRTUAL_KEY_NEXT:
      next();
      break;
#endif

    default:
      FormField::onEvent(event);
      break;
  }
}
