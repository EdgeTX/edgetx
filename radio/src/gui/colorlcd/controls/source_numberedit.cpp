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

#include "source_numberedit.h"

#include "edgetx.h"
#include "sourcechoice.h"

SourceNumberEdit::SourceNumberEdit(Window* parent,
                                   int32_t vmin, int32_t vmax,
                                   std::function<int32_t()> getValue,
                                   std::function<void(int32_t)> setValue,
                                   int16_t sourceMin,
                                   LcdFlags textFlags, int32_t voffset,
                                   int32_t vdefault) :
    Window(parent, {0, 0, EdgeTxStyles::EDIT_FLD_WIDTH_NARROW + SRC_BTN_W + PAD_TINY * 3, EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY * 2}),
    vmin(vmin),
    vmax(vmax),
    sourceMin(sourceMin),
    sourceDefault(sourceMin),
    getValue(getValue),
    setValue(setValue),
    voffset(voffset)
{
  setTextFlag(textFlags);

  padAll(PAD_TINY);
  lv_obj_set_flex_flow(lvobj, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_flex_cross_place(lvobj, LV_FLEX_ALIGN_CENTER, 0);
  lv_obj_set_size(lvobj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

  // Source field
  source_field = new SourceChoice(
      this, {0, 0, EdgeTxStyles::EDIT_FLD_WIDTH_NARROW, 0}, sourceMin, INPUTSRC_LAST,
      [=]() { return decode(); },
      [=](int newValue) { setValue(encode(newValue, true)); }, true);

  num_field = new NumberEdit(
      this, {0, 0, EdgeTxStyles::EDIT_FLD_WIDTH_NARROW, 0}, vmin, vmax,
      [=]() { return decode() + voffset; },
      [=](int newValue) { setValue(encode(newValue - voffset, false)); },
      textFlags);
  num_field->setDefault(vdefault);

  // The Source button
  m_srcBtn = new TextButton(this, {EdgeTxStyles::EDIT_FLD_WIDTH_NARROW + PAD_TINY, 0, SRC_BTN_W, 0}, "SRC", [=]() {
    switchSourceMode();
    return isSource();
  });

  delayLoad();
}

void SourceNumberEdit::delayedInit()
{
  m_srcBtn->check(isSource());

  // update field type based on value
  update();
}

bool SourceNumberEdit::isSource()
{
  SourceNumVal v;
  v.rawValue = getValue();
  return v.isSource;
}

int16_t SourceNumberEdit::decode()
{
  SourceNumVal v;
  v.rawValue = getValue();
  return v.value;
}

int32_t SourceNumberEdit::encode(int16_t value, bool isSource)
{
  return makeSourceNumVal(value, isSource);
}

void SourceNumberEdit::switchSourceMode()
{
  // TODO: convert value???
  setValue(encode(isSource() ? 0 : sourceDefault, !isSource()));

  // update field type based on value
  update();
}

void SourceNumberEdit::setSuffix(const std::string& value)
{
  num_field->setSuffix(value);
}

void SourceNumberEdit::setDisplayHandler(
    std::function<std::string(int value)> function)
{
  num_field->setDisplayHandler(function);
}

void SourceNumberEdit::update()
{
  bool has_focus = act_field && act_field->hasFocus();

  source_field->hide();
  num_field->hide();

  if (isSource()) {
    // Source mode
    act_field = source_field;
    source_field->show();
    source_field->update();
  } else {
    // number edit mode
    act_field = num_field;
    num_field->show();
    num_field->update();
  }

  m_srcBtn->check(isSource());

  if (has_focus) {
    lv_group_focus_obj(act_field->getLvObj());
  }
}

bool LimitNumberEdit::isSource()
{
  LimitNumVal v;
  v.rawValue = getValue();
  return v.isSource;
}

int16_t LimitNumberEdit::decode()
{
  LimitNumVal v;
  v.rawValue = getValue();
  return v.value;
}

int32_t LimitNumberEdit::encode(int16_t value, bool isSource)
{
  return makeLimitNumVal(value, isSource);
}
