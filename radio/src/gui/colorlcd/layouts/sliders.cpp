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

#include "sliders.h"
#include "opentx.h"

enum slider_type {
  SLIDER_VERT,
  SLIDER_HORIZ,
  SLIDER_6POS,
};

static void slider_self_size(lv_event_t* e)
{
  lv_point_t* s = (lv_point_t*)lv_event_get_param(e);
  slider_type t = (slider_type)(intptr_t)lv_event_get_user_data(e);
  switch(t) {
  case SLIDER_VERT:
    s->y = VERTICAL_SLIDERS_HEIGHT;
    s->x = TRIM_SQUARE_SIZE;
    break;
  case SLIDER_HORIZ:
    s->x = HORIZONTAL_SLIDERS_WIDTH;
    s->y = TRIM_SQUARE_SIZE;
    break;
  case SLIDER_6POS:
    s->x = MULTIPOS_W;
    s->y = MULTIPOS_H;
    break;
  }
}

MainViewHorizontalSlider::MainViewHorizontalSlider(Window* parent,
                                                   uint8_t idx) :
    MainViewSlider(parent, rect_t{}, idx)
{
  void* user_data = (void*)SLIDER_HORIZ;
  lv_obj_add_event_cb(lvobj, slider_self_size, LV_EVENT_GET_SELF_SIZE,
                      user_data);

  setWidth(HORIZONTAL_SLIDERS_WIDTH);
  setHeight(TRIM_SQUARE_SIZE);
}

void MainViewHorizontalSlider::paint(BitmapBuffer * dc)
{
  // The ticks
  int delta = (width() - TRIM_SQUARE_SIZE) / SLIDER_TICKS_COUNT;
  coord_t x = TRIM_SQUARE_SIZE / 2;
  for (uint8_t i = 0; i <= SLIDER_TICKS_COUNT; i++) {
    if (i == 0 || i == SLIDER_TICKS_COUNT / 2 || i == SLIDER_TICKS_COUNT)
      dc->drawSolidVerticalLine(x, 2, 13, COLOR_THEME_SECONDARY1);
    else
      dc->drawSolidVerticalLine(x, 4, 9, COLOR_THEME_SECONDARY1);
    x += delta;
  }

  // The square
  x = divRoundClosest((width() - TRIM_SQUARE_SIZE) * (value + RESX), 2 * RESX);
  drawTrimSquare(dc, x, 0, COLOR_THEME_FOCUS);
}

MainView6POS::MainView6POS(Window* parent, uint8_t idx) :
    MainViewSlider(parent, rect_t{}, idx)
{
  void* user_data = (void*)SLIDER_6POS;
  lv_obj_add_event_cb(lvobj, slider_self_size, LV_EVENT_GET_SELF_SIZE,
                      user_data);

  setWidth(MULTIPOS_W);
  setHeight(MULTIPOS_H);
}

void MainView6POS::paint(BitmapBuffer * dc)
{
#if NUM_XPOTS > 0 // prevent compiler warning
  // The ticks
  int delta = (width() - TRIM_SQUARE_SIZE) / (XPOTS_MULTIPOS_COUNT - 1);
  coord_t x = TRIM_SQUARE_SIZE / 2;
  for (uint8_t i = 0; i <= XPOTS_MULTIPOS_COUNT; i++) {
    dc->drawSolidVerticalLine(x, 4, 9, COLOR_THEME_SECONDARY1);
    x += delta;
  }

  // The square
  auto value = 1 + (potsPos[idx] & 0x0f);
  x = TRIM_SQUARE_SIZE / 2 + divRoundClosest((width() - TRIM_SQUARE_SIZE) * (value -1) , 6);
  drawTrimSquare(dc, x, 0, COLOR_THEME_FOCUS);
  dc->drawNumber(x + 1, 0, value, FONT(XS) | COLOR_THEME_PRIMARY2);
#endif
}

MainViewVerticalSlider::MainViewVerticalSlider(Window* parent, uint8_t idx) :
    MainViewSlider(parent, rect_t{}, idx)
{
  void* user_data = (void*)SLIDER_VERT;
  lv_obj_add_event_cb(lvobj, slider_self_size, LV_EVENT_GET_SELF_SIZE,
                      user_data);

  setHeight(VERTICAL_SLIDERS_HEIGHT);
  setWidth(TRIM_SQUARE_SIZE);
}

void MainViewVerticalSlider::paint(BitmapBuffer * dc)
{
  // The ticks
  int delta = 4;
  int sliderTicksCount = (height() - TRIM_SQUARE_SIZE) / delta;
  coord_t y = TRIM_SQUARE_SIZE / 2;
  for (uint8_t i = 0; i <= sliderTicksCount; i++) {
    if (i == 0 || i == sliderTicksCount / 2 || i == sliderTicksCount)
       dc->drawSolidHorizontalLine(2, y, 13, COLOR_THEME_SECONDARY1);
    else
      dc->drawSolidHorizontalLine(4, y, 9, COLOR_THEME_SECONDARY1);
    y += delta;
  }

  // The square
  y = divRoundClosest((height() - TRIM_SQUARE_SIZE) * (-value + RESX), 2 * RESX);
  drawTrimSquare(dc, 0, y, COLOR_THEME_FOCUS);
}
