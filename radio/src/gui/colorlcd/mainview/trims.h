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

class TrimIcon;

class MainViewTrim : public Window
{
 public:
  MainViewTrim(Window* parent, const rect_t& rect, uint8_t idx,
               bool isVertical);
    
  void setVisible(bool visible);

 protected:
  uint8_t idx;
  int value = 0;
  bool isVertical;
  bool showChange = false;
  int trimMin = 0, trimMax = 0;
  bool extendedTrims = false;
  bool hidden = false;
  TrimIcon* trimIcon = nullptr;
  DynamicNumber<int16_t>* trimValue = nullptr;
  lv_obj_t* trimBar = nullptr;

  void setRange();
  void setPos();
    
  bool setDisplayState();

  void checkEvents() override;

  coord_t sx();
  coord_t sy();
};

class MainViewHorizontalTrim : public MainViewTrim
{
 public:
  using MainViewTrim::MainViewTrim;
  MainViewHorizontalTrim(Window* parent, uint8_t idx);
};

class MainViewVerticalTrim : public MainViewTrim
{
 public:
  MainViewVerticalTrim(Window* parent, uint8_t idx);
};
