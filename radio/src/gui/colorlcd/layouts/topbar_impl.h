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

#include "topbar.h"
#include "layouts/layout_factory_impl.h"

typedef WidgetsContainerImpl<MAX_TOPBAR_ZONES, MAX_TOPBAR_OPTIONS> TopbarImplBase;

class TopbarImpl:
  public TopbarImplBase,
  public Topbar
{
  public:
    explicit TopbarImpl(Window * parent);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "TopBar";
    }
#endif

    void load() override
    {
        TopbarImplBase::load();
    }
    
    unsigned int getZonesCount() const override;

    rect_t getZone(unsigned int index) const override;

    void setVisible(float visible);
    coord_t getVisibleHeight(float visible) const; // 0.0 -> 1.0
  
    void paint(BitmapBuffer * dc) override;

    void checkEvents() override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif
  
  protected:
    uint32_t lastRefresh = 0;
};
