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

#include "window.h"

class MainViewTrim;

class ViewMainDecoration
{
  public:
    ViewMainDecoration(Window* parent, bool showTrims = true, bool showSliders = true, bool showFM = true);

    // Set decoration visibility
    void setTrimsVisible(bool visible);
    void setSlidersVisible(bool visible);
    void setFlightModeVisible(bool visible);

    // Get the available space in the middle of the screen
    // (without decoration)
    rect_t getMainZone() const;

  protected:

    enum {
      SLIDERS_POT1 = 0,
      SLIDERS_POT2,
      SLIDERS_POT3,
      SLIDERS_REAR_LEFT,
      SLIDERS_EXT1,
      SLIDERS_REAR_RIGHT,
      SLIDERS_EXT2,
      SLIDERS_MAX
    };
  
    enum {
      TRIMS_LH = 0,
      TRIMS_LV,
      TRIMS_RV,
      TRIMS_RH,
      TRIMS_MAX
    };

    Window* parent;
    Window* w_ml;
    Window* w_mr;
    Window* w_bl;
    Window* w_bc;
    Window* w_br;
  
    Window* sliders[SLIDERS_MAX] = { 0 };
    MainViewTrim* trims[TRIMS_MAX] = { 0 };
    Window* flightMode = nullptr;

    Window* layoutBox(Window* parent, lv_align_t align, lv_flex_flow_t flow);

    void createSliders(Window* ml, Window* mr, Window* bl, Window* bc, Window* br);
    void createTrims(Window* ml, Window* mr, Window* bl, Window* br);
    void createFlightMode(Window* bc);
};
