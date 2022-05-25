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

const uint8_t _LBM_LAYOUT_2x4[] = {
#include "mask_layout2x4.lbm"
};
STATIC_LZ4_BITMAP(LBM_LAYOUT_2x4);

const ZoneOption OPTIONS_LAYOUT_2x4[] = {
    LAYOUT_COMMON_OPTIONS,
    {"Panel1 background", ZoneOption::Bool},
    {"  Color", ZoneOption::Color},
    {"Panel2 background", ZoneOption::Bool},
    {"  Color", ZoneOption::Color},
    LAYOUT_OPTIONS_END
};

class Layout2x4 : public Layout
{
 public:
  enum {
    OPTION_PANEL1_BACKGROUND = LAYOUT_OPTION_LAST_DEFAULT + 1,
    OPTION_PANEL1_COLOR,
    OPTION_PANEL2_BACKGROUND,
    OPTION_PANEL2_COLOR
  };

  Layout2x4(Window* parent, const LayoutFactory* factory,
            Layout::PersistentData* persistentData) :
      Layout(parent, factory, persistentData)
  {
  }

  void create() override
  {
    Layout::create();
    getOptionValue(OPTION_PANEL1_BACKGROUND)->boolValue = true;
    getOptionValue(OPTION_PANEL1_COLOR)->unsignedValue = RGB(77, 112, 203);
    getOptionValue(OPTION_PANEL2_BACKGROUND)->boolValue = true;
    getOptionValue(OPTION_PANEL2_COLOR)->unsignedValue = RGB(77, 112, 203);
  }

  unsigned int getZonesCount() const override { return 8; }

  rect_t getZone(unsigned int index) const override
  {
    rect_t zone = getMainZone();

    zone.w /= 2;
    zone.h /= 4;

    if ((!isMirrored() && index > 3) || (isMirrored() && index < 4)) {
      zone.x += zone.w;
    }

    zone.y += (index % 4) * zone.h;

    return zone;
  }

  void paint(BitmapBuffer* dc) override;
};

void Layout2x4::paint(BitmapBuffer* dc)
{
  Layout::paint(dc);
  rect_t fullScreen = Layout::getMainZone();
  fullScreen.w /= 2;
  if (getOptionValue(OPTION_PANEL1_BACKGROUND)->boolValue) {
    dc->drawSolidFilledRect(
        fullScreen.x, fullScreen.y, fullScreen.w, fullScreen.h,
        COLOR2FLAGS(getOptionValue(OPTION_PANEL1_COLOR)->unsignedValue));
  }

  if (getOptionValue(OPTION_PANEL2_BACKGROUND)->boolValue) {
    fullScreen.x += fullScreen.w;
    dc->drawSolidFilledRect(
        fullScreen.x, fullScreen.y, fullScreen.w, fullScreen.h,
        COLOR2FLAGS(getOptionValue(OPTION_PANEL2_COLOR)->unsignedValue));
  }
}

BaseLayoutFactory<Layout2x4> layout2x4("Layout2x4", "2 x 4", LBM_LAYOUT_2x4,
                                       OPTIONS_LAYOUT_2x4);
