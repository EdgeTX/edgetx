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

void MainViewTrim::checkEvents()
{
  Window::checkEvents();
  int8_t stickIndex = CONVERT_MODE(idx);
  int newValue = getTrimValue(mixerCurrentFlightMode, stickIndex);
  if (value != newValue) {
    value = newValue;
    invalidate();
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

void MainViewHorizontalTrim::paint(BitmapBuffer * dc)
{
  int32_t trimMin, trimMax;
  if (g_model.extendedTrims) {
    trimMin = TRIM_EXTENDED_MIN;
    trimMax = TRIM_EXTENDED_MAX;
  }
  else {
    trimMin = TRIM_MIN;
    trimMax = TRIM_MAX;
  }

  int16_t dispVal = value;
  if (dispVal < trimMin) dispVal = trimMin;
  if (dispVal > trimMax) dispVal = trimMax;
  
  // Trim line
  dc->drawSolidFilledRect(
      TRIM_SQUARE_SIZE / 2, (height() - TRIM_LINE_WIDTH - 1) / 2,
      width() - TRIM_SQUARE_SIZE + 1, TRIM_LINE_WIDTH, COLOR_THEME_SECONDARY1);

  // Trim square
  coord_t x = divRoundClosest((width() - TRIM_SQUARE_SIZE) * (dispVal - trimMin),
                              trimMax - trimMin);
  drawTrimSquare(dc, x, 0,
                 (value < TRIM_MIN || value > TRIM_MAX)
                     ? COLOR_THEME_ACTIVE /* TODO add a color */
                     : COLOR_THEME_FOCUS);

  // Trim value / small lines on the square
  if (g_model.displayTrims == DISPLAY_TRIMS_ALWAYS) {
    // TODO DISPLAY_TRIMS_CHANGE
    dc->drawNumber(x + (TRIM_SQUARE_SIZE + 1) / 2, 3,
                   divRoundClosest(value * 100, trimMax),
                   FONT(XXS) | COLOR_THEME_PRIMARY2 | CENTERED);
  } else {
    if (value >= 0) {
      dc->drawSolidVerticalLine(x + 4, 3, 9, COLOR_THEME_PRIMARY2);
    }
    if (value <= 0) {
      dc->drawSolidVerticalLine(x + 10, 3, 9, COLOR_THEME_PRIMARY2);
    }
  }
}

MainViewVerticalTrim::MainViewVerticalTrim(Window* parent, uint8_t idx) :
  MainViewTrim(parent, rect_t{}, idx)
{
  void* user_data = (void*)TRIM_VERT;
  lv_obj_add_event_cb(lvobj, trim_self_size, LV_EVENT_GET_SELF_SIZE, user_data);

  setHeight(VERTICAL_SLIDERS_HEIGHT);
  setWidth(TRIM_SQUARE_SIZE);
}

void MainViewVerticalTrim::paint(BitmapBuffer * dc)
{
  int32_t trimMin, trimMax;
  if (g_model.extendedTrims) {
    trimMin = TRIM_EXTENDED_MIN;
    trimMax = TRIM_EXTENDED_MAX;
  }
  else {
    trimMin = TRIM_MIN;
    trimMax = TRIM_MAX;
  }

  int16_t dispVal = value;
  if (dispVal < trimMin) dispVal = trimMin;
  if (dispVal > trimMax) dispVal = trimMax;

  // Trim line
  dc->drawSolidFilledRect((width() - TRIM_LINE_WIDTH) / 2, TRIM_SQUARE_SIZE / 2,
                          TRIM_LINE_WIDTH, height() - TRIM_SQUARE_SIZE + 1,
                          COLOR_THEME_SECONDARY1);

  // Trim square
  coord_t y =
      height() - TRIM_SQUARE_SIZE -
      divRoundClosest((height() - TRIM_SQUARE_SIZE) * (dispVal - trimMin),
                      trimMax - trimMin);
  drawTrimSquare(dc, 0, y,
                 (value < TRIM_MIN || value > TRIM_MAX)
                     ? COLOR_THEME_ACTIVE /* TODO add a color */
                     : COLOR_THEME_FOCUS);

  // Trim value / small lines on the square
  if (g_model.displayTrims == DISPLAY_TRIMS_ALWAYS) {
    // TODO DISPLAY_TRIMS_CHANGE
    dc->drawNumber((TRIM_SQUARE_SIZE + 1) / 2, y + 3,
                   divRoundClosest(value * 100, trimMax),
                   FONT(XXS) | COLOR_THEME_PRIMARY2 | CENTERED);
  } else {
    if (value >= 0) {
      dc->drawSolidHorizontalLine(3, y + 4, 9, COLOR_THEME_PRIMARY2);
    }
    if (value <= 0) {
      dc->drawSolidHorizontalLine(3, y + 10, 9, COLOR_THEME_PRIMARY2);
    }
  }
}
