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

#include "gvar_numberedit.h"
#include "opentx.h"

void GVarNumberEdit::value_changed(lv_event_t* e)
{
  auto obj = lv_event_get_target(e);
  auto edit = (GVarNumberEdit*)lv_obj_get_user_data(obj);
  if (!edit) return;

  edit->update();
}

GVarNumberEdit::GVarNumberEdit(Window* parent, const rect_t& rect, int32_t vmin,
                               int32_t vmax, std::function<int32_t()> getValue,
                               std::function<void(int32_t)> setValue,
                               LcdFlags textFlags, int32_t voffset) :
    Window(parent, rect),
    vmin(vmin),
    vmax(vmax),
    getValue(getValue),
    setValue(setValue),
    textFlags(textFlags),
    voffset(voffset)
{
  lv_obj_set_flex_flow(lvobj, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_pad_column(lvobj, lv_dpx(4), 0);
  lv_obj_set_style_flex_cross_place(lvobj, LV_FLEX_ALIGN_CENTER, 0);
  lv_obj_set_size(lvobj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  
  // GVAR field
  gvar_field = new Choice(
      this, rect_t{}, -MAX_GVARS, MAX_GVARS - 1,
      [=]() {
        uint16_t gvar1 = GV_GET_GV1_VALUE(vmin, vmax);
        return GV_INDEX_CALC_DELTA(getValue(), gvar1);
      },
      [=](int idx) {
        uint16_t gvar1 = GV_GET_GV1_VALUE(vmin, vmax);
        if (idx < 0)
          setValue(GV_CALC_VALUE_IDX_NEG(idx, gvar1));
        else
          setValue(GV_CALC_VALUE_IDX_POS(idx, gvar1));
      });
  gvar_field->setTextHandler(
      [=](int32_t value) { return getGVarString(value); });

  num_field = new NumberEdit(
      this, rect_t{}, vmin, vmax, [=]() { return getValue() + voffset; },
      [=](int32_t newValue) { return setValue(newValue - voffset); },
      windowFlags, textFlags);

#if defined(GVARS)
  // The GVAR button
  auto btn = new TextButton(this, rect_t{}, STR_GV, [=]() {
    switchGVarMode();
    return 0;
  });
  lv_obj_set_height(btn->getLvObj(), lv_obj_get_height(gvar_field->getLvObj()));
#endif

  lv_obj_add_event_cb(lvobj, GVarNumberEdit::value_changed,
                      LV_EVENT_VALUE_CHANGED, nullptr);

  // update field type based on value
  update();
}

void GVarNumberEdit::switchGVarMode()
{
  int32_t value = getValue();
  setValue(
      GV_IS_GV_VALUE(value, vmin, vmax)
          ? ((textFlags & PREC1)
                 ? GET_GVAR_PREC1(value, vmin, vmax, mixerCurrentFlightMode)
                 : GET_GVAR(value, vmin, vmax, mixerCurrentFlightMode))
          : GV_GET_GV1_VALUE(vmin, vmax));

  // update field type based on value
  update();
}

void GVarNumberEdit::setSuffix(std::string value)
{
  num_field->setSuffix(value);
}

void GVarNumberEdit::onEvent(event_t event)
{
  if (event == EVT_KEY_LONG(KEY_ENTER)) {
    // TODO: check this...
    // killEvents(event);
    switchGVarMode();
  } else {
    Window::onEvent(event);
  }
}

void GVarNumberEdit::update()
{
  bool has_focus = act_field && act_field->hasFocus();

  auto gvar_obj = gvar_field->getLvObj();
  auto num_obj = num_field->getLvObj();

  lv_obj_add_flag(gvar_obj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(num_obj, LV_OBJ_FLAG_HIDDEN);
  
  int32_t value = getValue();
  if (GV_IS_GV_VALUE(value, vmin, vmax)) {
    // GVAR mode
    act_field = gvar_field;
    lv_obj_clear_flag(gvar_obj, LV_OBJ_FLAG_HIDDEN);
    lv_event_send(gvar_obj, LV_EVENT_VALUE_CHANGED, nullptr);
  } else {
    // number edit mode
    act_field = num_field;
    lv_obj_clear_flag(num_obj, LV_OBJ_FLAG_HIDDEN);
    num_field->setValue(value + voffset);
  }

  if (has_focus) {
    auto act_obj = act_field->getLvObj();
    lv_group_focus_obj(act_obj);
  }
}
