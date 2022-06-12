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
#include "lz4_bitmaps.h"

const uint8_t _LBM_LAYOUT_1x4[] = {
#include "mask_layout1x4.lbm"
};
STATIC_LZ4_BITMAP(LBM_LAYOUT_1x4);

const ZoneOption OPTIONS_LAYOUT_1x4[] =  {
  LAYOUT_COMMON_OPTIONS,
  LAYOUT_OPTIONS_END
};

class Layout1x4 : public Layout
{
 public:
  Layout1x4(Window* parent, const LayoutFactory* factory,
            Layout::PersistentData* persistentData) :
      Layout(parent, factory, persistentData)
  {
  }

  unsigned int getZonesCount() const override { return 4; }

  rect_t getZone(unsigned int index) const override
  {
    rect_t zone = getMainZone();

    if (index > 3) {
      index = 3;
    }
    return {zone.x, zone.y + zone.h / 4 * (int)index, zone.w, zone.h / 4};

    return zone;
  }
};

BaseLayoutFactory<Layout1x4> Layout1x4("Layout1x4", "1 x 4", LBM_LAYOUT_1x4,
                                       OPTIONS_LAYOUT_1x4);
