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

#include "button.h"

class InputMixButton : public Button
{
 public:
  InputMixButton(Window* parent, const rect_t& rect, uint8_t index);

  uint8_t getIndex() const { return index; }
  void setIndex(uint8_t i) { index = i; }

  void checkEvents() override;

  void drawFlightModes(BitmapBuffer *dc, FlightModesType value,
                       LcdFlags textColor, coord_t x, coord_t y);

  // TODO: generic "getHeight()" and recalc on changes

 protected:
  uint8_t index;
  bool active = false;

  virtual bool isActive() const = 0;
};
