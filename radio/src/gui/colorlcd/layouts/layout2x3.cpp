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

const uint8_t _LBM_LAYOUT_2x3[] = {
#include "mask_layout2x3.lbm"
};
STATIC_LZ4_BITMAP(LBM_LAYOUT_2x3);

const ZoneOption OPTIONS_LAYOUT_2x3[] = {LAYOUT_COMMON_OPTIONS,
                                         LAYOUT_OPTIONS_END};

class Layout2x3 : public Layout
{
 public:
  Layout2x3(Window* parent, const LayoutFactory* factory,
            Layout::PersistentData* persistentData) :
      Layout(parent, factory, persistentData)
  {
  }

  unsigned int getZonesCount() const override { return 6; }

  rect_t getZone(unsigned int index) const override
  {
    rect_t zone = getMainZone();

    zone.w /= 2;
    zone.h /= 3;

    if (index == 1 || index == 3)
      zone.y += zone.h;
    else if (index == 2 || index == 4)
      zone.y += zone.h * 2;

    if ((!isMirrored() && index > 2) || (isMirrored() && index < 3))
      zone.x += zone.w;

    return zone;
  }
};

BaseLayoutFactory<Layout2x3> layout2x3("Layout2x3", "2 x 3", LBM_LAYOUT_2x3,
                                       OPTIONS_LAYOUT_2x3);
