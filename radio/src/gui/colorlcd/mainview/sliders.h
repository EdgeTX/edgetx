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

  static LAYOUT_VAL(SLIDER_TICK_SPACING, 4, 4)
  static LAYOUT_VAL(SLIDER_VTICKS_COUNT, 40, 30)
#if defined(RADIO_PL18) || defined(RADIO_PL18EV)
  static constexpr coord_t SLIDER_HTICKS_COUNT = 34;
#else
  static LAYOUT_VAL(SLIDER_HTICKS_COUNT, 40, 30)
#endif
  static constexpr coord_t HORIZONTAL_SLIDERS_WIDTH =
      SLIDER_HTICKS_COUNT * SLIDER_TICK_SPACING + LayoutFactory::TRIM_SQUARE_SIZE;
  static constexpr coord_t VERTICAL_SLIDERS_HEIGHT =
      SLIDER_VTICKS_COUNT * SLIDER_TICK_SPACING + LayoutFactory::TRIM_SQUARE_SIZE;

  static LAYOUT_VAL(SL_SZ, 15, 15)

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

  static LAYOUT_VAL(MULTIPOS_W_SPACING, 12, 12)
  static LAYOUT_VAL(MULTIPOS_SZ, 12, 12)
  static LAYOUT_VAL(MULTIPOS_XO, 3, 3)
  static constexpr coord_t MULTIPOS_W = (XPOTS_MULTIPOS_COUNT + 1) * MULTIPOS_W_SPACING;

 protected:
  uint8_t idx;
  int16_t value = -10000;
  SliderIcon* posIcon = nullptr;
  lv_obj_t* posVal = nullptr;
};
