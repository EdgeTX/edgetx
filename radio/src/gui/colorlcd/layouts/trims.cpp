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

#include "trims.h"
#include "sliders.h"

#include "opentx.h"

enum trim_type {
  TRIM_VERT,
  TRIM_HORIZ,
};

static void trim_self_size(lv_event_t* e)
{
  lv_point_t* s = (lv_point_t*)lv_event_get_param(e);
  trim_type t = (trim_type)(intptr_t)lv_event_get_user_data(e);
  switch(t) {
  case TRIM_VERT:
    s->y = VERTICAL_SLIDERS_HEIGHT;
    s->x = TRIM_SQUARE_SIZE;
    break;
  case TRIM_HORIZ:
    s->x = HORIZONTAL_SLIDERS_WIDTH;
    s->y = TRIM_SQUARE_SIZE;
    break;
  }
}

MainViewTrim::MainViewTrim(Window * parent, const rect_t & rect, uint8_t idx):
  Window(parent, rect),
  idx(idx)
{
  setRange();
}

void MainViewTrim::setRange()
{
  if (g_model.extendedTrims) {
    trimMin = TRIM_EXTENDED_MIN;
    trimMax = TRIM_EXTENDED_MAX;
  }
  else {
    trimMin = TRIM_MIN;
    trimMax = TRIM_MAX;
  }
}

void MainViewTrim::checkEvents()
{
  Window::checkEvents();
  int8_t stickIndex = CONVERT_MODE(idx);
  int newValue = getTrimValue(mixerCurrentFlightMode, stickIndex);

  setRange();
  newValue = min(max(newValue, trimMin), trimMax);

  if (value != newValue ||
      (g_model.displayTrims == DISPLAY_TRIMS_CHANGE && showChange && trimsDisplayTimer == 0)) {
    value = newValue;
    invalidate();
  }
}

void MainViewTrim::paint(BitmapBuffer * dc)
{
  // Trim line
  drawLine(dc);

  coord_t x = sx();
  coord_t y = sy();

  // Trim square
  drawTrimSquare(dc, x, y,
                 (value < TRIM_MIN || value > TRIM_MAX)
                     ? COLOR_THEME_ACTIVE /* TODO add a color */
                     : COLOR_THEME_FOCUS);
  drawMarkerLines(dc, x, y);

  // Trim value / small lines on the square
  if ((g_model.displayTrims == DISPLAY_TRIMS_ALWAYS) ||
      ((g_model.displayTrims == DISPLAY_TRIMS_CHANGE) && (trimsDisplayTimer > 0) && (trimsDisplayMask & (1 << idx)))) {
    if (value) {
      showChange = true;
      drawValue(dc);
    }
  } else {
    showChange = false;
  }
}

MainViewHorizontalTrim::MainViewHorizontalTrim(Window* parent, uint8_t idx) :
  MainViewTrim(parent, rect_t{}, idx)
{
  void* user_data = (void*)TRIM_HORIZ;
  lv_obj_add_event_cb(lvobj, trim_self_size, LV_EVENT_GET_SELF_SIZE, user_data);

  setWidth(HORIZONTAL_SLIDERS_WIDTH);
  setHeight(TRIM_SQUARE_SIZE);
}

coord_t MainViewHorizontalTrim::sx()
{
  return divRoundClosest((HORIZONTAL_SLIDERS_WIDTH - TRIM_SQUARE_SIZE) * (value - trimMin),
                         trimMax - trimMin);
}

void MainViewHorizontalTrim::drawLine(BitmapBuffer * dc)
{
  // Trim line
  dc->drawSolidFilledRect(
      TRIM_SQUARE_SIZE / 2, (TRIM_SQUARE_SIZE - TRIM_LINE_WIDTH - 1) / 2,
      HORIZONTAL_SLIDERS_WIDTH - TRIM_SQUARE_SIZE + 1, TRIM_LINE_WIDTH, COLOR_THEME_SECONDARY1);
}

void MainViewHorizontalTrim::drawMarkerLines(BitmapBuffer * dc, coord_t x, coord_t y)
{
  // Small lines on the square
  if (value >= 0) {
    dc->drawSolidVerticalLine(x + 4, 3, 9, COLOR_THEME_PRIMARY2);
  }
  if (value <= 0) {
    dc->drawSolidVerticalLine(x + 10, 3, 9, COLOR_THEME_PRIMARY2);
  }
}

void MainViewHorizontalTrim::drawValue(BitmapBuffer * dc)
{
  coord_t x = (value < 0) ? HORIZONTAL_SLIDERS_WIDTH * 4 / 5 : HORIZONTAL_SLIDERS_WIDTH / 5;
  dc->drawSolidFilledRect(x - TRIM_SQUARE_SIZE/2, (TRIM_SQUARE_SIZE-12)/2, TRIM_SQUARE_SIZE, 12, COLOR_THEME_SECONDARY1);
  dc->drawNumber(x, 2,
                 divRoundClosest(abs(value) * 100, trimMax),
                 FONT(XXS) | COLOR_THEME_PRIMARY2 | CENTERED);
}

MainViewVerticalTrim::MainViewVerticalTrim(Window* parent, uint8_t idx) :
  MainViewTrim(parent, rect_t{}, idx)
{
  void* user_data = (void*)TRIM_VERT;
  lv_obj_add_event_cb(lvobj, trim_self_size, LV_EVENT_GET_SELF_SIZE, user_data);

  setWidth(TRIM_SQUARE_SIZE);
  setHeight(VERTICAL_SLIDERS_HEIGHT);
}

coord_t MainViewVerticalTrim::sy()
{
  return VERTICAL_SLIDERS_HEIGHT - TRIM_SQUARE_SIZE -
         divRoundClosest((VERTICAL_SLIDERS_HEIGHT - TRIM_SQUARE_SIZE) * (value - trimMin),
                         trimMax - trimMin);
}

void MainViewVerticalTrim::drawLine(BitmapBuffer * dc)
{
  // Trim line
  dc->drawSolidFilledRect((TRIM_SQUARE_SIZE - TRIM_LINE_WIDTH) / 2, TRIM_SQUARE_SIZE / 2,
                          TRIM_LINE_WIDTH, VERTICAL_SLIDERS_HEIGHT - TRIM_SQUARE_SIZE + 1,
                          COLOR_THEME_SECONDARY1);
}

void MainViewVerticalTrim::drawMarkerLines(BitmapBuffer * dc, coord_t x, coord_t y)
{
  // Small lines on the square
  if (value >= 0) {
    dc->drawSolidHorizontalLine(3, y + 4, 9, COLOR_THEME_PRIMARY2);
  }
  if (value <= 0) {
    dc->drawSolidHorizontalLine(3, y + 10, 9, COLOR_THEME_PRIMARY2);
  }
}

void MainViewVerticalTrim::drawValue(BitmapBuffer * dc)
{
  coord_t y = (value > 0) ? VERTICAL_SLIDERS_HEIGHT * 4 / 5 : VERTICAL_SLIDERS_HEIGHT / 5 - 11;
  dc->drawSolidFilledRect(0, y, TRIM_SQUARE_SIZE, 11, COLOR_THEME_SECONDARY1);
  dc->drawNumber(TRIM_SQUARE_SIZE/2, y,
                 divRoundClosest(abs(value) * 100, trimMax),
                 FONT(XXS) | COLOR_THEME_PRIMARY2 | CENTERED);
}
