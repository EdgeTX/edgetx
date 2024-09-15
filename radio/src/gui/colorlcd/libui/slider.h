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

class Slider : public Window
{
 public:
  Slider(Window* parent, coord_t width, int32_t vmin, int32_t vmax,
         std::function<int()> getValue, std::function<void(int)> setValue);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "Slider"; }
#endif

  void setValue(int value)
  {
    if (_setValue != nullptr) _setValue(limit(vmin, value, vmax));
  }

  void update();

 protected:
  int vmin;
  int vmax;
  lv_obj_t* slider;
  lv_obj_t** tickPts = nullptr;
  bool loaded = false;

  std::function<int()> _getValue;
  std::function<void(int)> _setValue;

  static void slider_changed_cb(lv_event_t* e);
  static void on_draw(lv_event_t* e);
  void delayedInit();

  void deleteLater(bool detach, bool trash) override;
};
