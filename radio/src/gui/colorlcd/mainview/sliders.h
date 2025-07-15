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

#include "libopenui.h"

class SliderIcon : public Window
{
 public:
  SliderIcon(Window* parent);

 protected:
  lv_obj_t* fill = nullptr;
};

class MainViewSlider : public Window
{
 public:
  MainViewSlider(Window* parent, const rect_t& rect, uint8_t idx,
                 bool isVertical);

#if defined(RADIO_PL18) || defined(RADIO_PL18EV) || defined(RADIO_PL18U)
  static constexpr coord_t SLIDER_SIZE = 136; // to fit 3 across bottom row
#else
  static LAYOUT_SIZE_SCALED_EVEN(SLIDER_SIZE, 160, 120)
#endif
  static LAYOUT_VAL_SCALED_ODD(SLIDER_ICON_SIZE, 15)
  static constexpr coord_t SLIDER_BAR_SIZE = SLIDER_ICON_SIZE + 2;
  static LAYOUT_VAL_SCALED(SLIDER_TICK_SPACING, 4)
  static constexpr coord_t HORIZONTAL_SLIDERS_WIDTH = SLIDER_SIZE + SLIDER_BAR_SIZE;
  static constexpr coord_t VERTICAL_SLIDERS_HEIGHT = SLIDER_SIZE + SLIDER_BAR_SIZE;

 protected:
  uint8_t potIdx;
  int16_t value = 0;
  bool isVertical;
  SliderIcon* sliderIcon = nullptr;
  lv_point_t* tickPoints = nullptr;

  void setPos();

  void checkEvents() override;

  void deleteLater(bool detach = true, bool trash = true) override;
};

class MainViewHorizontalSlider : public MainViewSlider
{
 public:
  MainViewHorizontalSlider(Window* parent, uint8_t idx);
};

class MainViewVerticalSlider : public MainViewSlider
{
 public:
  MainViewVerticalSlider(Window* parent, const rect_t& rect, uint8_t idx);
};

class MainView6POS : public Window
{
 public:
  MainView6POS(Window* parent, uint8_t idx);

  void checkEvents() override;

  static LAYOUT_VAL_SCALED(MULTIPOS_W_SPACING, 12)
  static LAYOUT_VAL_SCALED(MULTIPOS_SZ, 12)
  static constexpr coord_t MULTIPOS_W = (XPOTS_MULTIPOS_COUNT + 1) * MULTIPOS_W_SPACING;

 protected:
  uint8_t idx;
  int16_t value = -10000;
  SliderIcon* posIcon = nullptr;
  lv_obj_t* posVal = nullptr;
};
