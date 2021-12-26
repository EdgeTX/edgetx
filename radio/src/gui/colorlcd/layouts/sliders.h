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
#include "trims.h"

#if defined(PCBNV14) || defined(PCBPL18)
constexpr uint8_t SLIDER_TICKS_COUNT = 30;
#else
constexpr uint8_t SLIDER_TICKS_COUNT = 40;
#endif
constexpr coord_t HMARGIN = 5;
constexpr coord_t HORIZONTAL_SLIDERS_WIDTH = SLIDER_TICKS_COUNT * 4 + TRIM_SQUARE_SIZE;
constexpr coord_t MULTIPOS_H = 20;
constexpr coord_t MULTIPOS_W = 50;
constexpr coord_t VERTICAL_SLIDERS_HEIGHT = SLIDER_TICKS_COUNT * 4 + TRIM_SQUARE_SIZE;

class MainViewSlider : public Window
{
  public:
    MainViewSlider(Window * parent, const rect_t & rect, uint8_t idx):
      Window(parent, rect),
      idx(idx)
    {
    }

    void checkEvents() override
    {
      Window::checkEvents();
      int16_t newValue = calibratedAnalogs[idx];
      if (value != newValue) {
        value = newValue;
        invalidate();
      }
    }

  protected:
    uint8_t idx;
    int16_t value = 0;
};

class MainViewHorizontalSlider : public MainViewSlider
{
  public:
    using MainViewSlider::MainViewSlider;
    void paint(BitmapBuffer * dc) override;
};

class MainView6POS : public MainViewSlider
{
  public:
    using MainViewSlider::MainViewSlider;
    void paint(BitmapBuffer * dc) override;
};

class MainViewVerticalSlider : public MainViewSlider
{
  public:
    using MainViewSlider::MainViewSlider;
    void paint(BitmapBuffer * dc) override;
};
