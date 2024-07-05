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

#include "layout.h"
#include "layout_factory_impl.h"

const ZoneOption OPTIONS_LAYOUT_APPMODE[] = {LAYOUT_OPTIONS_END};

class LayoutAppMode : public Layout
{
 public:
  LayoutAppMode(Window* parent, const LayoutFactory* factory,
            Layout::PersistentData* persistentData, uint8_t zoneCount,
            uint8_t* zoneMap) :
      Layout(parent, factory, persistentData, zoneCount, zoneMap)
  {
  }

  bool hasTopbar() const override { return false; }
  bool hasFlightMode() const override { return false; }
  bool hasSliders() const override { return false; }
  bool hasTrims() const override { return false; }
  bool isMirrored() const override { return false; }
  bool isAppMode() const override { return true; }

 protected:
};

static const uint8_t zmap[] = {
    LAYOUT_MAP_0, LAYOUT_MAP_0, LAYOUT_MAP_FULL, LAYOUT_MAP_FULL,
};

BaseLayoutFactory<LayoutAppMode> layoutAppMode("Layout1x1AM", "App mode", OPTIONS_LAYOUT_APPMODE,
                                       1, (uint8_t*)zmap);
