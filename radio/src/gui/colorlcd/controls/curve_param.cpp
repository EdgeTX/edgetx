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
#include "source_numberedit.h"
#include "model_curves.h"
#include "edgetx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

CurveChoice::CurveChoice(Window* parent, std::function<int()> getRefValue, 
        std::function<void(int32_t)> setRefValue, std::function<void(void)> refreshView, mixsrc_t source) :
  Choice(parent, rect_t{}, -MAX_CURVES, MAX_CURVES, getRefValue, setRefValue),
  source(source), refreshView(std::move(refreshView))
  {
    setTextHandler([](int value) { return getCurveString(value); });
  }

bool CurveChoice::onLongPress()
{
  if (modelCurvesEnabled()) {
    if (_getValue()) {
      lv_obj_clear_state(lvobj, LV_STATE_PRESSED);
      ModelCurvesPage::pushEditCurve(abs(_getValue()) - 1, refreshView, source);
    }
  }
  return true;
}

CurveParam::CurveParam(Window* parent, const rect_t& rect, CurveRef* ref,
                       std::function<void(int32_t)> setRefValue, int16_t sourceMin, mixsrc_t source,
                       std::function<void(void)> refreshView) :
    Window(parent, rect), ref(ref), refreshView(std::move(refreshView))
{
  padAll(PAD_TINY);
  lv_obj_set_flex_flow(lvobj, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_flex_cross_place(lvobj, LV_FLEX_ALIGN_CENTER, 0);
  lv_obj_set_size(lvobj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

  new Choice(this, rect_t{}, STR_VCURVETYPE, 0, modelCurvesEnabled() ? CURVE_REF_CUSTOM : CURVE_REF_FUNC,
             GET_DEFAULT(ref->type), [=](int32_t newValue) {
               ref->type = newValue;
               ref->value = 0;
               SET_DIRTY();
               update();
             });

  // CURVE_REF_DIFF
  // CURVE_REF_EXPO
  auto gv = new SourceNumberEdit(this, -100, 100, GET_DEFAULT(ref->value), setRefValue, sourceMin);
  gv->setSuffix("%");
  value_edit = gv;

  // CURVE_REF_FUNC
  func_choice = new Choice(this, rect_t{}, STR_VCURVEFUNC, 0, CURVE_BASE - 1,
                           [=]() {
                             SourceNumVal v;
                             v.rawValue = ref->value;
                             return v.value;
                           },
                           [=](int32_t newValue) {
                             SourceNumVal v;
                             v.isSource = false;
                             v.value = newValue;
                             setRefValue(v.rawValue);
                           });

  // CURVE_REF_CUSTOM
  cust_choice = new CurveChoice(this, 
                                [=]() {
                                  SourceNumVal v;
                                  v.rawValue = ref->value;
                                  return v.value;
                                },
                                [=](int32_t newValue) {
                                  SourceNumVal v;
                                  v.isSource = false;
                                  v.value = newValue;
                                  setRefValue(v.rawValue);
                                }, refreshView, source);

  update();
}

void CurveParam::update()
{
  bool has_focus = act_field && act_field->hasFocus();

  value_edit->hide();
  func_choice->hide();
  cust_choice->hide();

  switch (ref->type) {
    case CURVE_REF_DIFF:
    case CURVE_REF_EXPO:
      value_edit->update();
      act_field = value_edit;
      break;

    case CURVE_REF_FUNC:
      func_choice->update();
      act_field = func_choice;
      break;

    case CURVE_REF_CUSTOM:
      cust_choice->update();
      act_field = cust_choice;
      break;

    default:
      return;
  }

  act_field->show();
  if (refreshView)
    refreshView();

  auto act_obj = act_field->getLvObj();
  if (has_focus) {
    lv_group_focus_obj(act_obj);
  }
}
