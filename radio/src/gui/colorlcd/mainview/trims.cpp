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

#include "bitmaps.h"
#include "input_mapping.h"
#include "edgetx.h"
#include "sliders.h"

class TrimIcon : public SliderIcon
{
 public:
  TrimIcon(Window* parent, bool isVertical) : SliderIcon(parent)
  {
    if (isVertical) {
      barPoints[0] = {3, 4};
      barPoints[1] = {12, 4};
      barPoints[2] = {3, 10};
      barPoints[3] = {12, 10};
    } else {
      barPoints[0] = {10, 3};
      barPoints[1] = {10, 12};
      barPoints[2] = {4, 3};
      barPoints[3] = {4, 12};
    }

    bar1 = lv_line_create(lvobj);
    etx_obj_add_style(bar1, styles->div_line_white, LV_PART_MAIN);
    etx_obj_add_style(bar1, styles->div_line_black, LV_STATE_USER_1);
    lv_line_set_points(bar1, &barPoints[0], 2);
    bar2 = lv_line_create(lvobj);
    etx_obj_add_style(bar2, styles->div_line_white, LV_PART_MAIN);
    etx_obj_add_style(bar2, styles->div_line_black, LV_STATE_USER_1);
    lv_line_set_points(bar2, &barPoints[2], 2);

    etx_bg_color(fill, COLOR_THEME_ACTIVE_INDEX, LV_STATE_USER_1);
  }

  void setState(int value)
  {
    if (value < TRIM_MIN || value > TRIM_MAX) {
      lv_obj_add_state(fill, LV_STATE_USER_1);
      lv_obj_add_state(bar1, LV_STATE_USER_1);
      lv_obj_add_state(bar2, LV_STATE_USER_1);
    } else {
      lv_obj_clear_state(fill, LV_STATE_USER_1);
      lv_obj_clear_state(bar1, LV_STATE_USER_1);
      lv_obj_clear_state(bar2, LV_STATE_USER_1);
    }

    if (value >= 0)
      lv_obj_clear_flag(bar1, LV_OBJ_FLAG_HIDDEN);
    else
      lv_obj_add_flag(bar1, LV_OBJ_FLAG_HIDDEN);

    if (value <= 0)
      lv_obj_clear_flag(bar2, LV_OBJ_FLAG_HIDDEN);
    else
      lv_obj_add_flag(bar2, LV_OBJ_FLAG_HIDDEN);
  }

 protected:
  lv_obj_t* bar1 = nullptr;
  lv_obj_t* bar2 = nullptr;
  lv_point_t barPoints[4];
};

MainViewTrim::MainViewTrim(Window* parent, const rect_t& rect, uint8_t idx,
                           bool isVertical) :
    Window(parent, rect), idx(idx), isVertical(isVertical)
{
  trimBar = lv_obj_create(lvobj);
  etx_solid_bg(trimBar, COLOR_THEME_SECONDARY1_INDEX);
  etx_obj_add_style(trimBar, styles->rounded, LV_PART_MAIN);
  if (isVertical) {
    lv_obj_set_pos(trimBar, (LayoutFactory::TRIM_SQUARE_SIZE - LayoutFactory::TRIM_LINE_WIDTH) / 2,
                   LayoutFactory::TRIM_SQUARE_SIZE / 2);
    lv_obj_set_size(trimBar, LayoutFactory::TRIM_LINE_WIDTH,
                    MainViewSlider::VERTICAL_SLIDERS_HEIGHT - LayoutFactory::TRIM_SQUARE_SIZE + 1);
  } else {
    lv_obj_set_pos(trimBar, LayoutFactory::TRIM_SQUARE_SIZE / 2,
                   (LayoutFactory::TRIM_SQUARE_SIZE - LayoutFactory::TRIM_LINE_WIDTH - 1) / 2);
    lv_obj_set_size(trimBar, MainViewSlider::HORIZONTAL_SLIDERS_WIDTH - LayoutFactory::TRIM_SQUARE_SIZE + 1,
                    LayoutFactory::TRIM_LINE_WIDTH);
  }

  trimIcon = new TrimIcon(this, isVertical);

  trimValue = new DynamicNumber<int16_t>(
      this, {0, 0, LayoutFactory::TRIM_SQUARE_SIZE, 12},
      [=]() { return divRoundClosest(abs(value) * 100, trimMax); },
      COLOR_THEME_PRIMARY2_INDEX, FONT(XXS) | CENTERED);
  etx_solid_bg(trimValue->getLvObj(), COLOR_THEME_SECONDARY1_INDEX);
  trimValue->hide();

  setRange();
  setPos();
}

void MainViewTrim::setRange()
{
  extendedTrims = g_model.extendedTrims;
  if (extendedTrims) {
    trimMin = TRIM_EXTENDED_MIN;
    trimMax = TRIM_EXTENDED_MAX;
  } else {
    trimMin = TRIM_MIN;
    trimMax = TRIM_MAX;
  }
}

void MainViewTrim::setPos()
{
  coord_t x = sx();
  coord_t y = sy();

  lv_obj_set_pos(trimIcon->getLvObj(), x, y);
  trimIcon->setState(value);

  if ((g_model.displayTrims == DISPLAY_TRIMS_ALWAYS) ||
      ((g_model.displayTrims == DISPLAY_TRIMS_CHANGE) &&
       (trimsDisplayTimer > 0) && (trimsDisplayMask & (1 << idx)))) {
    if (value) {
      if (isVertical) {
        x = 0;
        y = (value > 0) ? MainViewSlider::VERTICAL_SLIDERS_HEIGHT * 4 / 5
                        : MainViewSlider::VERTICAL_SLIDERS_HEIGHT / 5 - 11;
      } else {
        x = ((value < 0) ? MainViewSlider::HORIZONTAL_SLIDERS_WIDTH * 4 / 5
                         : MainViewSlider::HORIZONTAL_SLIDERS_WIDTH / 5) -
            LayoutFactory::TRIM_SQUARE_SIZE / 2;
        y = (LayoutFactory::TRIM_SQUARE_SIZE - 12) / 2;
      }
      lv_obj_set_pos(trimValue->getLvObj(), x, y);
      trimValue->show();
      showChange = true;
    } else {
      trimValue->hide();
    }
  } else {
    showChange = false;
    trimValue->hide();
  }
}

void MainViewTrim::setVisible(bool visible)
{
  hidden = !visible;
  setDisplayState();
}

bool MainViewTrim::setDisplayState()
{
  trim_t v = getRawTrimValue(mixerCurrentFlightMode, inputMappingConvertMode(idx));
  if (hidden || v.mode == TRIM_MODE_NONE || v.mode == TRIM_MODE_3POS) {
    // Hide trim if not being used
    hide();
    return false;
  } else {
    show();
  }

  return true;
}

void MainViewTrim::checkEvents()
{
  Window::checkEvents();

  // Do nothing if trims turned off
  if (hidden) return;

  // Don't update if not visible
  if (!setDisplayState()) return;

  int newValue = getTrimValue(mixerCurrentFlightMode, inputMappingConvertMode(idx));

  bool update = false;
  
  if (extendedTrims != g_model.extendedTrims) {
    update = true;
    setRange();
    newValue = limit(trimMin, newValue, trimMax);
  }

  if (update || value != newValue || (g_model.displayTrims == DISPLAY_TRIMS_CHANGE &&
                            showChange && trimsDisplayTimer == 0)) {
    value = newValue;
    setPos();
  }
}

coord_t MainViewTrim::sx()
{
  if (isVertical) return 0;

  return divRoundClosest(
      (MainViewSlider::HORIZONTAL_SLIDERS_WIDTH - LayoutFactory::TRIM_SQUARE_SIZE) * (value - trimMin),
      trimMax - trimMin);
}

coord_t MainViewTrim::sy()
{
  if (!isVertical) return 0;

  return divRoundClosest(
             (MainViewSlider::VERTICAL_SLIDERS_HEIGHT - LayoutFactory::TRIM_SQUARE_SIZE) * (trimMax - value),
             trimMax - trimMin);
}

MainViewHorizontalTrim::MainViewHorizontalTrim(Window* parent, uint8_t idx) :
    MainViewTrim(parent,
                 rect_t{0, 0, MainViewSlider::HORIZONTAL_SLIDERS_WIDTH, LayoutFactory::TRIM_SQUARE_SIZE}, idx,
                 false)
{
}

MainViewVerticalTrim::MainViewVerticalTrim(Window* parent, uint8_t idx) :
    MainViewTrim(parent,
                 rect_t{0, 0, LayoutFactory::TRIM_SQUARE_SIZE, MainViewSlider::VERTICAL_SLIDERS_HEIGHT}, idx,
                 true)
{
}
