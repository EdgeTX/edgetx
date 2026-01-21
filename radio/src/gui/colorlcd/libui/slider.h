/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#include "window.h"

class SliderBase : public Window
{
 public:
  SliderBase(Window* parent, coord_t width, coord_t height, int32_t vmin, int32_t vmax,
         std::function<int()> getValue, std::function<void(int)> setValue);

  void setValue(int value);
  void setColor(LcdFlags color);

  void update();

  void enable(bool enabled = true) override;

 protected:
  int vmin;
  int vmax;
  lv_obj_t* slider;
  lv_obj_t** tickPts = nullptr;

  std::function<int()> _getValue;
  std::function<void(int)> _setValue;

  void deleteLater() override;
  void checkEvents() override;

  static void slider_changed_cb(lv_event_t* e);
};

class Slider : public SliderBase
{
 public:
  Slider(Window* parent, coord_t width, int32_t vmin, int32_t vmax,
         std::function<int()> getValue, std::function<void(int)> setValue);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "Slider"; }
#endif

 protected:
  void delayedInit() override;
};

class VerticalSlider : public SliderBase
{
 public:
  VerticalSlider(Window* parent, coord_t height, int32_t vmin, int32_t vmax,
         std::function<int()> getValue, std::function<void(int)> setValue);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "VerticalSlider"; }
#endif

 protected:
  void delayedInit() override;
};
