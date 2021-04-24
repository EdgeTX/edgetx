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

#pragma once

#include "window.h"

constexpr coord_t FM_LABEL_HEIGHT = 20;

class ViewMainDecoration: public Window
{
  public:
    ViewMainDecoration(Window * parent, const rect_t & rect):
        Window(parent, rect)
    {
      createSliders();
      createTrims();
      createFlightMode();
    }

    // Set decoration visibility
    void setTrimsVisible(bool visible);
    void setSlidersVisible(bool visible);
    void setFlightModeVisible(bool visible);

    // Re-calculate positions
    void adjustDecoration();

    // Get the available space in the middle of the screen
    // (without decoration)
    rect_t getMainZone() const;

    enum VisibilityMask {
      VM_NONE =          0,
      VM_TRIMS =    1 << 0,
      VM_SLIDERS =  1 << 1,
      VM_FM =       1 << 2,
      VM_ALL = VM_TRIMS | VM_SLIDERS | VM_FM,
    };

    unsigned getVisibilityMask() const
    {
      return visibilityMask;
    }
  
  protected:

#if defined(DEBUG_WINDOWS)
    virtual std::string getName() const
    {
      return "ViewMainDecoration";
    }
#endif    

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

    Window* sliders[SLIDERS_MAX];
    Window* trims[TRIMS_MAX];
    Window* flightMode;
    unsigned visibilityMask = VM_NONE;

    void createSliders();
    void createTrims();
    void createFlightMode();
};
