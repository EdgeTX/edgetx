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

const uint8_t _LBM_LAYOUT_1x1[] = {
#include "mask_layout1x1.lbm"
};
STATIC_LZ4_BITMAP(LBM_LAYOUT_1x1);

const ZoneOption OPTIONS_LAYOUT_1x1[] = {LAYOUT_COMMON_OPTIONS,
                                         LAYOUT_OPTIONS_END};

class Layout1x1 : public Layout
{
 public:
  Layout1x1(Window* parent, const LayoutFactory* factory,
            Layout::PersistentData* persistentData) :
      Layout(parent, factory, persistentData)
  {
  }

  unsigned int getZonesCount() const override { return 1; }

  rect_t getZone(unsigned int index) const override { return getMainZone(); }
};

BaseLayoutFactory<Layout1x1> layout1x1("Layout1x1", "Fullscreen",
                                       LBM_LAYOUT_1x1, OPTIONS_LAYOUT_1x1);
