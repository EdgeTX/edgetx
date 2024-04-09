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

#include "bitmaps.h"
#include "hal/adc_driver.h"
#include "opentx.h"
#include "switches.h"

static const lv_style_const_prop_t shadow1_props[] = {
    // LV_STYLE_CONST_SHADOW_COLOR does not compile in GitHub ???
    {.prop = LV_STYLE_SHADOW_COLOR, .value = {.color = {.full = 0}}},
    LV_STYLE_CONST_SHADOW_OPA(LV_OPA_20),
    LV_STYLE_CONST_SHADOW_OFS_X(1),
    LV_STYLE_CONST_SHADOW_OFS_Y(1),
    LV_STYLE_CONST_SHADOW_WIDTH(1),
    LV_STYLE_PROP_INV,
};
static LV_STYLE_CONST_MULTI_INIT(shadow1_style, shadow1_props);

static const lv_style_const_prop_t shadow2_props[] = {
    // LV_STYLE_CONST_SHADOW_COLOR does not compile in GitHub ???
    {.prop = LV_STYLE_SHADOW_COLOR, .value = {.color = {.full = 0}}},
    LV_STYLE_CONST_SHADOW_OPA(LV_OPA_40),
    LV_STYLE_CONST_SHADOW_OFS_X(1),
    LV_STYLE_CONST_SHADOW_OFS_Y(1),
    LV_STYLE_CONST_SHADOW_WIDTH(1),
    LV_STYLE_PROP_INV,
};
static LV_STYLE_CONST_MULTI_INIT(shadow2_style, shadow2_props);

SliderIcon::SliderIcon(Window* parent) :
    Window(parent, rect_t{0, 0, 17, 17})
{
  setWindowFlag(NO_FOCUS);

  auto shad = lv_obj_create(lvobj);
  etx_obj_add_style(shad, shadow1_style, LV_PART_MAIN);
  lv_obj_set_pos(shad, 1, 1);
  lv_obj_set_size(shad, 15, 15);

  fill = lv_obj_create(lvobj);
  etx_obj_add_style(fill, shadow2_style, LV_PART_MAIN);
  lv_obj_set_pos(fill, 0, 0);
  lv_obj_set_size(fill, 15, 15);
  etx_solid_bg(fill, COLOR_THEME_FOCUS_INDEX);
}

MainViewSlider::MainViewSlider(Window* parent, const rect_t& rect, uint8_t idx,
                               bool isVertical) :
    Window(parent, rect), idx(idx), isVertical(isVertical)
{
  if (isVertical) {
    int sliderTicksCount = (height() - TRIM_SQUARE_SIZE) / SLIDER_TICK_SPACING;
    tickPoints = new lv_point_t[(sliderTicksCount + 1) * 2];

    lv_coord_t y = TRIM_SQUARE_SIZE / 2;
    for (uint8_t i = 0; i <= sliderTicksCount; i++) {
      if (i == 0 || i == sliderTicksCount / 2 || i == sliderTicksCount) {
        tickPoints[i * 2] = {2, y};
        tickPoints[i * 2 + 1] = {15, y};
      } else {
        tickPoints[i * 2] = {4, y};
        tickPoints[i * 2 + 1] = {13, y};
      }
      auto line = lv_line_create(lvobj);
      etx_obj_add_style(line, styles->div_line, LV_PART_MAIN);
      lv_line_set_points(line, &tickPoints[i * 2], 2);
      y += SLIDER_TICK_SPACING;
    }
  } else {
    int sliderTicksCount = (width() - TRIM_SQUARE_SIZE) / SLIDER_TICK_SPACING;
    tickPoints = new lv_point_t[(sliderTicksCount + 1) * 2];

    lv_coord_t x = TRIM_SQUARE_SIZE / 2;
    for (uint8_t i = 0; i <= sliderTicksCount; i++) {
      if (i == 0 || i == sliderTicksCount / 2 || i == SLIDER_TICKS_COUNT) {
        tickPoints[i * 2] = {x, 2};
        tickPoints[i * 2 + 1] = {x, 15};
      } else {
        tickPoints[i * 2] = {x, 4};
        tickPoints[i * 2 + 1] = {x, 13};
      }
      auto line = lv_line_create(lvobj);
      etx_obj_add_style(line, styles->div_line, LV_PART_MAIN);
      lv_line_set_points(line, &tickPoints[i * 2], 2);
      x += SLIDER_TICK_SPACING;
    }
  }

  sliderIcon = new SliderIcon(this);
  coord_t x = 0, y = 0;
  if (isVertical)
    y = (height() - TRIM_SQUARE_SIZE) / 2;
  else
    y = (width() - TRIM_SQUARE_SIZE) / 2;
  lv_obj_set_pos(sliderIcon->getLvObj(), x, y);

  checkEvents();
}

void MainViewSlider::deleteLater(bool detach, bool trash)
{
  if (!deleted()) {
    if (tickPoints) delete tickPoints;
    Window::deleteLater(detach, trash);
  }
}

void MainViewSlider::checkEvents()
{
  Window::checkEvents();

  auto pot_idx = adcGetInputOffset(ADC_INPUT_FLEX) + idx;
  int16_t newValue = calibratedAnalogs[pot_idx];
  if (value != newValue) {
    value = newValue;

    coord_t x = 0, y = 0;
    if (isVertical) {
      y = divRoundClosest((height() - TRIM_SQUARE_SIZE) * (-value + RESX),
                          2 * RESX);
    } else {
      x = divRoundClosest((width() - TRIM_SQUARE_SIZE) * (value + RESX),
                          2 * RESX);
    }
    lv_obj_set_pos(sliderIcon->getLvObj(), x, y);
  }
}

MainViewHorizontalSlider::MainViewHorizontalSlider(Window* parent,
                                                   uint8_t idx) :
    MainViewSlider(parent,
                   rect_t{0, 0, HORIZONTAL_SLIDERS_WIDTH, TRIM_SQUARE_SIZE},
                   idx, false)
{
}

MainViewVerticalSlider::MainViewVerticalSlider(Window* parent,
                                               const rect_t& rect,
                                               uint8_t idx) :
    MainViewSlider(parent, rect, idx, true)
{
}

constexpr coord_t MULTIPOS_H = 18;
constexpr coord_t MULTIPOS_W_SPACING = 12;
constexpr coord_t MULTIPOS_W = (6 + 1) * MULTIPOS_W_SPACING;

MainView6POS::MainView6POS(Window* parent, uint8_t idx) :
    Window(parent, rect_t{0, 0, MULTIPOS_W, MULTIPOS_H}), idx(idx)
{
  char num[] = " ";
  coord_t x = MULTIPOS_W_SPACING / 4 + TRIM_SQUARE_SIZE / 4;
  for (uint8_t value = 0; value < XPOTS_MULTIPOS_COUNT; value++) {
    num[0] = value + '1';
    auto p = lv_label_create(lvobj);
    lv_label_set_text(p, num);
    lv_obj_set_size(p, 12, 12);
    lv_obj_set_pos(p, x, 0);
    etx_txt_color(p, COLOR_THEME_SECONDARY1_INDEX, LV_PART_MAIN);
    etx_font(p, FONT_XS_INDEX, LV_PART_MAIN);
    x += MULTIPOS_W_SPACING;
  }

  posIcon = new SliderIcon(this);
  posVal = lv_label_create(posIcon->getLvObj());
  lv_obj_set_pos(posVal, 3, -2);
  lv_obj_set_size(posVal, 12, 12);
  etx_txt_color(posVal, COLOR_THEME_PRIMARY2_INDEX, LV_PART_MAIN);
  etx_font(posVal, FONT_BOLD_INDEX, LV_PART_MAIN);

  checkEvents();
}

void MainView6POS::checkEvents()
{
  Window::checkEvents();
  int16_t newValue = getXPotPosition(idx);
  if (value != newValue) {
    value = newValue;

    coord_t x = MULTIPOS_W_SPACING / 4 + MULTIPOS_W_SPACING * value;
    lv_obj_set_pos(posIcon->getLvObj(), x, 0);

    char num[] = " ";
    num[0] = value + '1';
    lv_label_set_text(posVal, num);
  }
}
