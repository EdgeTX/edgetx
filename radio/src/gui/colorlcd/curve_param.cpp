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

#include "curve_param.h"
#include "gvar_numberedit.h"
#include "model_curves.h"

#include "opentx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

static const char *_curve_type[] = {"Diff", "Expo", "Func", "Cstm"};

void CurveParam::value_changed(lv_event_t* e)
{
  auto obj = lv_event_get_target(e);
  auto param = (CurveParam*)lv_obj_get_user_data(obj);
  if (!param) return;

  param->update();
}

CurveParam::CurveParam(Window* parent, const rect_t& rect, CurveRef* ref) :
    Window(parent, rect),
    ref(ref)
{
  lv_obj_set_flex_flow(lvobj, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_pad_column(lvobj, lv_dpx(4), 0);
  lv_obj_set_style_flex_cross_place(lvobj, LV_FLEX_ALIGN_CENTER, 0);
  lv_obj_set_size(lvobj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  
  new Choice(this, rect_t{}, _curve_type, 0,
             CURVE_REF_CUSTOM, GET_DEFAULT(ref->type),
             [=](int32_t newValue) {
               ref->type = newValue;
               ref->value = 0;
               SET_DIRTY();
               lv_event_send(lvobj, LV_EVENT_VALUE_CHANGED, nullptr);
             });
  lv_obj_add_event_cb(lvobj, CurveParam::value_changed, LV_EVENT_VALUE_CHANGED,
                      nullptr);

  // CURVE_REF_DIFF
  // CURVE_REF_EXPO
  value_edit = new GVarNumberEdit(this, rect_t{}, -100, 100,
                                  GET_SET_DEFAULT(ref->value));
  value_edit->setSuffix("%");

  // CURVE_REF_FUNC
  func_choice = new Choice(this, rect_t{}, STR_VCURVEFUNC, 0, CURVE_BASE - 1,
                           GET_SET_DEFAULT(ref->value));

  
  // CURVE_REF_CUSTOM
  cust_choice = new ChoiceEx(this, rect_t{}, -MAX_CURVES, MAX_CURVES,
                                  GET_SET_DEFAULT(ref->value));
  cust_choice->setTextHandler([](int value) { return getCurveString(value); });
  cust_choice->setLongPressHandler([ref](event_t event) {
    // if no curve is specified then dont link to curve page
    if (ref->value != 0) ModelCurvesPage::pushEditCurve(abs(ref->value) - 1);
  });

  update();
}

void CurveParam::update()
{
  bool has_focus = act_field && act_field->hasFocus();

  auto value_obj = value_edit->getLvObj();
  auto func_obj = func_choice->getLvObj();
  auto cust_obj = cust_choice->getLvObj();
  
  lv_obj_add_flag(value_obj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(func_obj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(cust_obj, LV_OBJ_FLAG_HIDDEN);
  
  switch(ref->type) {
  case CURVE_REF_DIFF:
  case CURVE_REF_EXPO:
    lv_obj_clear_flag(value_obj, LV_OBJ_FLAG_HIDDEN);
    act_field = value_edit;
    break;

  case CURVE_REF_FUNC:
    lv_obj_clear_flag(func_obj, LV_OBJ_FLAG_HIDDEN);
    act_field = func_choice;
    break;
    
  case CURVE_REF_CUSTOM:
    lv_obj_clear_flag(cust_obj, LV_OBJ_FLAG_HIDDEN);
    act_field = cust_choice;
    break;

  default:
    return;
  }

  auto act_obj = act_field->getLvObj();
  if (has_focus) {
    lv_group_focus_obj(act_obj);
  }
  lv_event_send(act_obj, LV_EVENT_VALUE_CHANGED, nullptr);
}

