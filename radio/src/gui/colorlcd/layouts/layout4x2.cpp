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

// 4x2 layout: 4 rows, 2 columns
// Suitable for portrait screens and vertical screen usage

const LayoutOption OPTIONS_LAYOUT_4x2[] = {
    LAYOUT_COMMON_OPTIONS,
    {STR_DEF(STR_PANEL1_BACKGROUND), LayoutOption::Bool, true},
    {STR_DEF(STR_PANEL_COLOR), LayoutOption::Color, RGB2FLAGS(77, 112, 203)},
    {STR_DEF(STR_PANEL2_BACKGROUND), LayoutOption::Bool, true},
    {STR_DEF(STR_PANEL_COLOR), LayoutOption::Color, RGB2FLAGS(77, 112, 203)},
    LAYOUT_OPTIONS_END};

class Layout4x2 : public Layout
{
 public:
  enum {
    OPTION_PANEL1_BACKGROUND = LAYOUT_OPTION_LAST_DEFAULT + 1,
    OPTION_PANEL1_COLOR,
    OPTION_PANEL2_BACKGROUND,
    OPTION_PANEL2_COLOR
  };

  Layout4x2(Window* parent, const LayoutFactory* factory,
                   int screenNum, uint8_t zoneCount,
                   uint8_t* zoneMap) :
      Layout(parent, factory, screenNum, zoneCount, zoneMap)
  {
    panel1 = lv_obj_create(lvobj);
    lv_obj_set_style_bg_opa(panel1, LV_OPA_COVER, LV_PART_MAIN);
    panel2 = lv_obj_create(lvobj);
    lv_obj_set_style_bg_opa(panel2, LV_OPA_COVER, LV_PART_MAIN);
    setPanels();
  }

 protected:
  rect_t mainZone = {0, 0, 0, 0};
  lv_obj_t* panel1 = nullptr;
  lv_obj_t* panel2 = nullptr;

  void checkEvents() override
  {
    setPanels();
    Layout::checkEvents();
  }

  void setPanels()
  {
    rect_t zone = Layout::getMainZone();
    if (mainZone.x != zone.x || mainZone.y != zone.y || mainZone.w != zone.w ||
        mainZone.h != zone.h) {
      mainZone = zone;
      // Left column panel
      lv_obj_set_pos(panel1, mainZone.x, mainZone.y);
      lv_obj_set_size(panel1, mainZone.w / 2, mainZone.h);
      // Right column panel
      lv_obj_set_pos(panel2, mainZone.x + mainZone.w / 2, mainZone.y);
      lv_obj_set_size(panel2, mainZone.w / 2, mainZone.h);
    }

    bool vis = getOptionValue(OPTION_PANEL1_BACKGROUND)->boolValue;
    if (vis == lv_obj_has_flag(panel1, LV_OBJ_FLAG_HIDDEN)) {
      if (vis)
        lv_obj_clear_flag(panel1, LV_OBJ_FLAG_HIDDEN);
      else
        lv_obj_add_flag(panel1, LV_OBJ_FLAG_HIDDEN);
    }
    vis = getOptionValue(OPTION_PANEL2_BACKGROUND)->boolValue;
    if (vis == lv_obj_has_flag(panel2, LV_OBJ_FLAG_HIDDEN)) {
      if (vis)
        lv_obj_clear_flag(panel2, LV_OBJ_FLAG_HIDDEN);
      else
        lv_obj_add_flag(panel2, LV_OBJ_FLAG_HIDDEN);
    }

    etx_bg_color_from_flags(panel1, getOptionValue(OPTION_PANEL1_COLOR)->unsignedValue);
    etx_bg_color_from_flags(panel2, getOptionValue(OPTION_PANEL2_COLOR)->unsignedValue);
  }
};

// Zone map: 4x2 (4 rows, 2 columns)
// Each zone is 1/4 height, 1/2 width
static const uint8_t zmap[] = {
    // Zone positions: x, y, w, h (using LAYOUT_MAP constants)
    LAYOUT_MAP_0,    LAYOUT_MAP_0,    LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,  // Top-left
    LAYOUT_MAP_HALF, LAYOUT_MAP_0,    LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,  // Top-right
    LAYOUT_MAP_0,    LAYOUT_MAP_1QTR, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,  // 2nd row left
    LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,  // 2nd row right
    LAYOUT_MAP_0,    LAYOUT_MAP_HALF, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,  // 3rd row left
    LAYOUT_MAP_HALF, LAYOUT_MAP_HALF, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,  // 3rd row right
    LAYOUT_MAP_0,    LAYOUT_MAP_3QTR, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,  // Bottom-left
    LAYOUT_MAP_HALF, LAYOUT_MAP_3QTR, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,  // Bottom-right
};

BaseLayoutFactory<Layout4x2> layout4x2("Layout4x2", "4 x 2",
                                        OPTIONS_LAYOUT_4x2,
                                        8, (uint8_t*)zmap);
