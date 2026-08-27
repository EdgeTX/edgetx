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
#include "translations/translations.h"

// 6x1 layout: 6 rows, 1 column
// Single column with 6 vertical zones

const LayoutOption OPTIONS_LAYOUT_6x1[] = {
    LAYOUT_COMMON_OPTIONS,
    {STR_DEF(STR_PANEL_BACKGROUND), LayoutOption::Bool, true},
    {STR_DEF(STR_PANEL_COLOR), LayoutOption::Color, RGB2FLAGS(50, 50, 50)},
    LAYOUT_OPTIONS_END};

class Layout6x1 : public Layout
{
 public:
  enum { OPTION_BACKGROUND = LAYOUT_OPTION_LAST_DEFAULT + 1, OPTION_COLOR };

  Layout6x1(Window* parent, const LayoutFactory* factory, int screenNum,
            uint8_t zoneCount, uint8_t* zoneMap) :
      Layout(parent, factory, screenNum, zoneCount, zoneMap)
  {
    panel = lv_obj_create(lvobj);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);
    setPanel();
  }

 protected:
  rect_t mainZone = {0, 0, 0, 0};
  lv_obj_t* panel = nullptr;

  void updateDecorations() override
  {
    Layout::updateDecorations();
    setPanel();
  }

  void setPanel()
  {
    rect_t zone = Layout::getWidgetsZone();
    if (mainZone.x != zone.x || mainZone.y != zone.y || mainZone.w != zone.w ||
        mainZone.h != zone.h) {
      mainZone = zone;
      lv_obj_set_pos(panel, mainZone.x, mainZone.y);
      lv_obj_set_size(panel, mainZone.w, mainZone.h);
    }

    bool vis = getOptionValue(OPTION_BACKGROUND)->boolValue;
    if (vis == lv_obj_has_flag(panel, LV_OBJ_FLAG_HIDDEN)) {
      if (vis)
        lv_obj_clear_flag(panel, LV_OBJ_FLAG_HIDDEN);
      else
        lv_obj_add_flag(panel, LV_OBJ_FLAG_HIDDEN);
    }

    etx_bg_color_from_flags(panel, getOptionValue(OPTION_COLOR)->unsignedValue);
  }
};

// Zone map: 6x1 (1 column, 6 rows)
// Each zone is 1/6 height, full width
// clang-format off
static const uint8_t zmap[] = {
    // Zone positions: x, y, w, h
    LAYOUT_MAP_0,     LAYOUT_MAP_0,       LAYOUT_MAP_FULL, LAYOUT_MAP_1SIXTH,   // Zone 1
    LAYOUT_MAP_0,     LAYOUT_MAP_1SIXTH,  LAYOUT_MAP_FULL, LAYOUT_MAP_1SIXTH,   // Zone 2
    LAYOUT_MAP_0,     LAYOUT_MAP_1THIRD,  LAYOUT_MAP_FULL, LAYOUT_MAP_1SIXTH,   // Zone 3
    LAYOUT_MAP_0,     LAYOUT_MAP_HALF,    LAYOUT_MAP_FULL, LAYOUT_MAP_1SIXTH,   // Zone 4
    LAYOUT_MAP_0,     LAYOUT_MAP_2THIRD,  LAYOUT_MAP_FULL, LAYOUT_MAP_1SIXTH,   // Zone 5
    LAYOUT_MAP_0,     LAYOUT_MAP_5SIXTH,  LAYOUT_MAP_FULL, LAYOUT_MAP_1SIXTH,   // Zone 6
};
// clang-format on

BaseLayoutFactory<Layout6x1> layout1x6("Layout1x6", "1 x 6", OPTIONS_LAYOUT_6x1,
                                       6, (uint8_t*)zmap);
