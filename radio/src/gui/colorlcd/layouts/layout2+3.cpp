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

const uint8_t _LBM_LAYOUT_2P3[] = {
#include "mask_layout2+3.lbm"
};
STATIC_LZ4_BITMAP(LBM_LAYOUT_2P3);

const ZoneOption OPTIONS_LAYOUT_2P3[] =  {
  LAYOUT_COMMON_OPTIONS,
  LAYOUT_OPTIONS_END
};

class Layout2P3 : public Layout
{
 public:
  Layout2P3(Window* parent, const LayoutFactory* factory,
            Layout::PersistentData* persistentData) :
      Layout(parent, factory, persistentData)
  {
  }

  unsigned int getZonesCount() const override { return 5; }

  rect_t getZone(unsigned int index) const override
  {
    rect_t zone = getMainZone();

    zone.w /= 2;

    if (index == 0 || index == 1) {
      zone.h /= 2;
      if (index == 1) zone.y += zone.h;
      if (isMirrored()) {
        zone.x += zone.w;
      }
    } else {
      index -= 2;
      zone.h /= 3;
      zone.y += zone.h * (int)index;
      if (!isMirrored()) {
        zone.x += zone.w;
      }
    }

    return zone;
  }
};

BaseLayoutFactory<Layout2P3> Layout2P3("Layout2P3", "2 + 3", LBM_LAYOUT_2P3,
                                       OPTIONS_LAYOUT_2P3);
