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
  lv_coord_t(LV_DPI_DEF * 0.55), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST,
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

  lv_obj_t* chText = nullptr;
  if (idx >= MIXSRC_FIRST_CH && idx <= MIXSRC_LAST_CH
      && g_model.limitData[idx - MIXSRC_FIRST_CH].name[0] != '\0') {
    chText = lv_label_create(lvobj);
    lv_label_set_text_fmt(chText, TR_CH "%zu", (size_t)(idx - MIXSRC_FIRST_CH + 1));
    lv_obj_set_style_text_font(chText, getFont(FONT(XS)), 0);
#if LCD_H > LCD_W
    lv_obj_set_style_pad_bottom(chText, -2, 0);
#endif
    lv_obj_set_grid_cell(chText,
                         LV_GRID_ALIGN_START, 0, 1,
                         LV_GRID_ALIGN_END, 0, 1);
  }

  label = lv_label_create(lvobj);
  lv_obj_set_style_text_font(label, getFont(FONT(STD)), 0);
#if LCD_H > LCD_W
  if(chText)
    lv_obj_set_style_pad_top(label, -1, 0);
#endif
  lv_label_set_text(label, getSourceString(idx));
  lv_obj_set_grid_cell(label,
                       LV_GRID_ALIGN_START, 0, 1,
                       chText?LV_GRID_ALIGN_START:LV_GRID_ALIGN_CENTER, 0, 1);

  auto box = window_create(lvobj);
  lv_obj_set_size(box, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_grid_cell(box,
                       LV_GRID_ALIGN_STRETCH, 1, 1,
                       LV_GRID_ALIGN_START, 0, 1);
  
  line_container = window_create(box);
  lv_obj_set_size(line_container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(line_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_flex_cross_place(line_container, LV_FLEX_ALIGN_END, 0);
  lv_obj_set_style_pad_row(line_container, lv_dpx(4), LV_PART_MAIN);
}

void InputMixGroup::enableMixerMonitor(uint8_t channel)
{
  if (monitor != nullptr) return;

  rect_t r{ 0, 0, 100, 14 };
  monitor = new MixerChannelBar(this, r, channel);

  lv_obj_t* mon_obj = monitor->getLvObj();
  lv_obj_set_parent(mon_obj, line_container);
  lv_obj_move_to_index(mon_obj, 0);
  lv_obj_set_style_translate_x(mon_obj, -lv_dpx(8), 0);
}

void InputMixGroup::disableMixerMonitor()
{
  if (!monitor) return;
  monitor->deleteLater();
  monitor = nullptr;
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
