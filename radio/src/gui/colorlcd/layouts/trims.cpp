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

#include "trims.h"
#include "opentx.h"

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
      width() - TRIM_SQUARE_SIZE + 1, TRIM_LINE_WIDTH, DEFAULT_COLOR);

  // Trim square
  coord_t x = divRoundClosest((width() - TRIM_SQUARE_SIZE) * (dispVal - trimMin),
                              trimMax - trimMin);
  drawTrimSquare(dc, x, 0,
                 (value < TRIM_MIN || value > TRIM_MAX)
                     ? HIGHLIGHT_COLOR /* TODO add a color */
                     : TRIM_BGCOLOR);

  // Trim value / small lines on the square
  if (g_model.displayTrims == DISPLAY_TRIMS_ALWAYS) {
    // TODO DISPLAY_TRIMS_CHANGE
    dc->drawNumber(x + (TRIM_SQUARE_SIZE + 1) / 2, 3,
                   divRoundClosest(value * 100, trimMax),
                   FONT(XXS) | FOCUS_COLOR | CENTERED);
  } else {
    if (value >= 0) {
      dc->drawSolidVerticalLine(x + 4, 3, 9, FOCUS_COLOR);
    }
    if (value <= 0) {
      dc->drawSolidVerticalLine(x + 10, 3, 9, FOCUS_COLOR);
    }
  }
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
                          DEFAULT_COLOR);

  // Trim square
  coord_t y =
      height() - TRIM_SQUARE_SIZE -
      divRoundClosest((height() - TRIM_SQUARE_SIZE) * (dispVal - trimMin),
                      trimMax - trimMin);
  drawTrimSquare(dc, 0, y,
                 (value < TRIM_MIN || value > TRIM_MAX)
                     ? HIGHLIGHT_COLOR /* TODO add a color */
                     : TRIM_BGCOLOR);

  // Trim value / small lines on the square
  if (g_model.displayTrims == DISPLAY_TRIMS_ALWAYS) {
    // TODO DISPLAY_TRIMS_CHANGE
    dc->drawNumber((TRIM_SQUARE_SIZE + 1) / 2, y + 3,
                   divRoundClosest(value * 100, trimMax),
                   FONT(XXS) | FOCUS_COLOR | CENTERED);
  } else {
    if (value >= 0) {
      dc->drawSolidHorizontalLine(3, y + 4, 9, FOCUS_COLOR);
    }
    if (value <= 0) {
      dc->drawSolidHorizontalLine(3, y + 10, 9, FOCUS_COLOR);
    }
  }
}
