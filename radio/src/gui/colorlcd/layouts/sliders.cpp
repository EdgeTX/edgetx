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
#include "switches.h"

#include "hal/adc_driver.h"

constexpr coord_t MULTIPOS_H = 18;
constexpr coord_t MULTIPOS_W_SPACING = 12;
constexpr coord_t MULTIPOS_W = (6+1)*MULTIPOS_W_SPACING;

MainViewSlider::MainViewSlider(Window* parent, const rect_t& rect,
                               uint8_t idx) :
    Window(parent, rect), idx(idx)
{
}

void MainViewSlider::checkEvents()
{
  Window::checkEvents();
  auto pot_idx = adcGetInputOffset(ADC_INPUT_FLEX) + idx;
  int16_t newValue = calibratedAnalogs[pot_idx];
  if (value != newValue) {
  value = newValue;
  invalidate();
  }
}

MainViewHorizontalSlider::MainViewHorizontalSlider(Window* parent,
                                                   uint8_t idx) :
    MainViewSlider(parent, rect_t{0, 0, HORIZONTAL_SLIDERS_WIDTH, TRIM_SQUARE_SIZE}, idx)
{
}

void MainViewHorizontalSlider::paint(BitmapBuffer * dc)
{
  // The ticks
  int sliderTicksCount = (width() - TRIM_SQUARE_SIZE) / SLIDER_TICK_SPACING;
  coord_t x = TRIM_SQUARE_SIZE / 2;
  for (uint8_t i = 0; i <= sliderTicksCount; i++) {
    if (i == 0 || i == sliderTicksCount / 2 || i == SLIDER_TICKS_COUNT)
      dc->drawSolidVerticalLine(x, 2, 13, COLOR_THEME_SECONDARY1);
    else
      dc->drawSolidVerticalLine(x, 4, 9, COLOR_THEME_SECONDARY1);
    x += SLIDER_TICK_SPACING;
  }

  // The square
  x = divRoundClosest((width() - TRIM_SQUARE_SIZE) * (value + RESX), 2 * RESX);
  drawTrimSquare(dc, x, 0, COLOR_THEME_FOCUS);
}

MainView6POS::MainView6POS(Window* parent, uint8_t idx) :
    MainViewSlider(parent, rect_t{0, 0, MULTIPOS_W, MULTIPOS_H}, idx)
{
}

void MainView6POS::paint(BitmapBuffer * dc)
{
  coord_t x = MULTIPOS_W_SPACING/4;
  for (uint8_t value = 0; value < XPOTS_MULTIPOS_COUNT; value++) {
    dc->drawNumber(x+TRIM_SQUARE_SIZE/4, 0, value+1, FONT(XS) | COLOR_THEME_SECONDARY1);
    x += MULTIPOS_W_SPACING;
  }

  // The square
  value = getXPotPosition(idx);
  x = MULTIPOS_W_SPACING / 4 + MULTIPOS_W_SPACING * value;
  drawTrimSquare(dc, x, 0, COLOR_THEME_FOCUS);
  dc->drawNumber(x+MULTIPOS_W_SPACING/4, -2, value+1, FONT(BOLD) | COLOR_THEME_PRIMARY2);
}

void MainView6POS::checkEvents()
{
  Window::checkEvents();
  int16_t newValue = getXPotPosition(idx);
  if (value != newValue) {
    value = newValue;
    invalidate();
  }
}

MainViewVerticalSlider::MainViewVerticalSlider(Window* parent, const rect_t& rect, uint8_t idx) :
    MainViewSlider(parent, rect, idx)
{
}

void MainViewVerticalSlider::paint(BitmapBuffer * dc)
{
  // The ticks
  int sliderTicksCount = (height() - TRIM_SQUARE_SIZE) / SLIDER_TICK_SPACING;
  coord_t y = TRIM_SQUARE_SIZE / 2;
  for (uint8_t i = 0; i <= sliderTicksCount; i++) {
    if (i == 0 || i == sliderTicksCount / 2 || i == sliderTicksCount)
       dc->drawSolidHorizontalLine(2, y, 13, COLOR_THEME_SECONDARY1);
    else
      dc->drawSolidHorizontalLine(4, y, 9, COLOR_THEME_SECONDARY1);
    y += SLIDER_TICK_SPACING;
  }

  // The square
  y = divRoundClosest((height() - TRIM_SQUARE_SIZE) * (-value + RESX), 2 * RESX);
  drawTrimSquare(dc, 0, y, COLOR_THEME_FOCUS);
}
