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

const ZoneOption OPTIONS_LAYOUT_2x4[] = {
    LAYOUT_COMMON_OPTIONS,
    {"Panel1 background", ZoneOption::Bool, OPTION_VALUE_BOOL(true)},
    {"  Color", ZoneOption::Color, RGB2FLAGS(77, 112, 203)},
    {"Panel2 background", ZoneOption::Bool, OPTION_VALUE_BOOL(true)},
    {"  Color", ZoneOption::Color, RGB2FLAGS(77, 112, 203)},
    LAYOUT_OPTIONS_END};

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
            Layout::PersistentData* persistentData, uint8_t zoneCount,
            uint8_t* zoneMap) :
      Layout(parent, factory, persistentData, zoneCount, zoneMap)
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
  LcdFlags panel1Color = -1;
  LcdFlags panel2Color = -1;

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
      lv_obj_set_pos(panel1, mainZone.x, mainZone.y);
      lv_obj_set_size(panel1, mainZone.w / 2, mainZone.h);
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
    if (vis != lv_obj_has_flag(panel2, LV_OBJ_FLAG_HIDDEN)) {
      if (vis)
        lv_obj_clear_flag(panel2, LV_OBJ_FLAG_HIDDEN);
      else
        lv_obj_add_flag(panel2, LV_OBJ_FLAG_HIDDEN);
    }

    etx_bg_color_from_flags(panel1, getOptionValue(OPTION_PANEL1_COLOR)->unsignedValue);
    etx_bg_color_from_flags(panel2, getOptionValue(OPTION_PANEL2_COLOR)->unsignedValue);
  }
};

static const uint8_t zmap[] = {
    LAYOUT_MAP_0,    LAYOUT_MAP_0,    LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,
    LAYOUT_MAP_0,    LAYOUT_MAP_1QTR, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,
    LAYOUT_MAP_0,    LAYOUT_MAP_HALF, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,
    LAYOUT_MAP_0,    LAYOUT_MAP_3QTR, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,
    LAYOUT_MAP_HALF, LAYOUT_MAP_0,    LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,
    LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,
    LAYOUT_MAP_HALF, LAYOUT_MAP_HALF, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,
    LAYOUT_MAP_HALF, LAYOUT_MAP_3QTR, LAYOUT_MAP_HALF, LAYOUT_MAP_1QTR,
};

BaseLayoutFactory<Layout2x4> layout2x4("Layout2x4", "2 x 4", OPTIONS_LAYOUT_2x4,
                                       8, (uint8_t*)zmap);
