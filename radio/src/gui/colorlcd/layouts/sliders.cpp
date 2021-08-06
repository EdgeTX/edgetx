/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

void MainView6POS::paint(BitmapBuffer * dc)
{
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
  dc->drawNumber(x + 1, 0, value, COLOR_THEME_PRIMARY2);
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
