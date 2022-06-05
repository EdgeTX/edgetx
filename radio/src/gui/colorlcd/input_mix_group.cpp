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

#include "input_mix_group.h"
#include "channel_bar.h"

#include "lvgl_widgets/input_mix_group.h"

#include "opentx.h"

#include <algorithm>

static const lv_coord_t col_dsc[] = {
  LV_DPI_DEF / 2, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST,
};

static const lv_coord_t row_dsc[] = {
  LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST,
};

void InputMixGroup::value_changed(lv_event_t* e)
{
  auto obj = lv_event_get_target(e);
  auto group = (InputMixGroup*)lv_obj_get_user_data(obj);
  if (!group) return;
  
  lv_label_set_text(group->label, getSourceString(group->idx));
}

InputMixGroup::InputMixGroup(Window* parent, mixsrc_t idx) :
    Window(parent, rect_t{}, 0, 0, input_mix_group_create), idx(idx)
{
  lv_obj_set_layout(lvobj, LV_LAYOUT_GRID);
  lv_obj_set_grid_dsc_array(lvobj, col_dsc, row_dsc);
  lv_obj_add_event_cb(lvobj, InputMixGroup::value_changed,
                      LV_EVENT_VALUE_CHANGED, nullptr);

  label = lv_label_create(lvobj);
  lv_label_set_text(label, getSourceString(idx));
  lv_obj_set_grid_cell(label,
                       LV_GRID_ALIGN_START, 0, 1,
                       LV_GRID_ALIGN_START, 0, 1);

  auto box = window_create(lvobj);
  lv_obj_set_size(box, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_grid_cell(box,
                       LV_GRID_ALIGN_STRETCH, 1, 1,
                       LV_GRID_ALIGN_START, 0, 1);
  
  line_container = window_create(box);
  lv_obj_set_size(line_container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(line_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_flex_cross_place(line_container, LV_FLEX_ALIGN_END, 0);
  lv_obj_set_style_pad_row(line_container, lv_dpx(8), LV_PART_MAIN);
}

void InputMixGroup::addMixerMonitor(uint8_t channel)
{
  rect_t r{ 0, 0, 100, 14 };
  auto mon = new MixerChannelBar(this, r, channel);
  mon->setDrawMiddleBar(false);

  lv_obj_t* mon_obj = mon->getLvObj();
  lv_obj_set_parent(mon_obj, line_container);
  lv_obj_set_style_translate_x(mon_obj, -lv_dpx(8), 0);
}

void InputMixGroup::addLine(Window* line, const uint8_t* symbol)
{
  lines.emplace_back(line, symbol);
  lv_obj_t* line_obj = line->getLvObj();
  lv_obj_set_parent(line_obj, line_container);
}

bool InputMixGroup::removeLine(Window* line)
{
  auto l = std::find_if(lines.begin(), lines.begin(), [=](const Line& l) -> bool {
      return l.win == line;
    });

  if (l != lines.end()) {
    lines.erase(l);
    return true;
  }

  return false;
}
