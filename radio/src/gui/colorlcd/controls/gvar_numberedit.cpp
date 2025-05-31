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
#include "edgetx.h"

GVarNumberEdit::GVarNumberEdit(Window* parent, int32_t vmin,
                               int32_t vmax, std::function<int32_t()> getValue,
                               std::function<void(int32_t)> setValue,
                               LcdFlags textFlags, int32_t voffset, int32_t vdefault) :
    Window(parent, {0, 0, EdgeTxStyles::EDIT_FLD_WIDTH_NARROW + GV_BTN_W + PAD_TINY * 3, EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY * 2}),
    vmin(vmin),
    vmax(vmax),
    getValue(getValue),
    setValue(setValue),
    textFlags(textFlags),
    voffset(voffset)
{
  padAll(PAD_TINY);
  
  // GVAR field
  gvar_field = new Choice(
      this, {0, 0, EdgeTxStyles::EDIT_FLD_WIDTH_NARROW, 0}, -MAX_GVARS, MAX_GVARS - 1,
      [=]() {
        return GV_INDEX_FROM_VALUE(getValue());
      },
      [=](int idx) {
        setValue(GV_VALUE_FROM_INDEX(idx));
      });
  gvar_field->setTextHandler(
      [=](int32_t value) { return getGVarString(value); });

  num_field = new NumberEdit(
      this, {0, 0, EdgeTxStyles::EDIT_FLD_WIDTH_NARROW, 0}, vmin, vmax, [=]() { return getValue() + voffset; },
      nullptr, textFlags);
  num_field->setDefault(vdefault);

#if defined(GVARS)
  // The GVAR button
  if (modelGVEnabled()) {
    m_gvBtn = new TextButton(this, {EdgeTxStyles::EDIT_FLD_WIDTH_NARROW + PAD_TINY, 0, GV_BTN_W, 0}, STR_GV, [=]() {
      switchGVarMode();
      return GV_IS_GV_VALUE(getValue());
    });
    m_gvBtn->check(GV_IS_GV_VALUE(getValue()));
  }
#endif

  // update field type based on value
  update();
}

void GVarNumberEdit::switchGVarMode()
{
#if defined(GVARS)
  if (modelGVEnabled()) {
    int32_t value = getValue();
    setValue(
        GV_IS_GV_VALUE(value)
            ? ((textFlags & PREC1)
                   ? GET_GVAR_PREC1(value, vmin, vmax, mixerCurrentFlightMode)
                   : GET_GVAR(value, vmin, vmax, mixerCurrentFlightMode))
            : GV_VALUE_FROM_INDEX(0));

    m_gvBtn->check(GV_IS_GV_VALUE(value));

    // update field type based on value
    update();
  }
#endif
}

void GVarNumberEdit::setSuffix(std::string value)
{
  num_field->setSuffix(value);
}

void GVarNumberEdit::onEvent(event_t event)
{
  if (event == EVT_KEY_LONG(KEY_ENTER)) {
    switchGVarMode();
  } else {
    Window::onEvent(event);
  }
}

void GVarNumberEdit::update()
{
  bool has_focus = act_field && act_field->hasFocus();

  gvar_field->hide();
  num_field->hide();
  
  int32_t value = getValue();
  if (GV_IS_GV_VALUE(value)) {
    // GVAR mode
    act_field = gvar_field;
    num_field->setSetValueHandler(nullptr);
    gvar_field->update();
    gvar_field->show();
  } else {
    // number edit mode
    act_field = num_field;
    num_field->setSetValueHandler(
        [=](int32_t newValue) { return setValue(newValue - voffset); });
    num_field->setValue(value + voffset);
    num_field->show();
  }

  if (has_focus) {
    auto act_obj = act_field->getLvObj();
    lv_group_focus_obj(act_obj);
  }
}

void GVarNumberEdit::setDisplayHandler(std::function<std::string(int value)> function)
{
  num_field->setDisplayHandler(function);
}
