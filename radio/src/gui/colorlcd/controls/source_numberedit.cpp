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
                                   ValueOrSource* vos,
                                   std::function<void()> onChanged,
                                   LcdFlags textFlags,
                                   int32_t vdefault) :
    Window(parent, {0, 0, EdgeTxStyles::EDIT_FLD_WIDTH_NARROW + SRC_BTN_W + PAD_TINY * 3, EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY * 2}),
    vmin(vmin),
    vmax(vmax),
    vos(vos)
{
  setTextFlag(textFlags);

  padAll(PAD_TINY);
  lv_obj_set_flex_flow(lvobj, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_style_flex_cross_place(lvobj, LV_FLEX_ALIGN_CENTER, 0);
  lv_obj_set_size(lvobj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

  // Source field — reads/writes ValueOrSource directly
  source_field = new SourceChoice(
      this, {0, 0, EdgeTxStyles::EDIT_FLD_WIDTH_NARROW, 0},
      [=]() -> SourceRef {
        return vos->toSourceRef();
      },
      [=](SourceRef ref) {
        vos->setSource(ref);
        if (onChanged) onChanged();
      }, true);

  num_field = new NumberEdit(
      this, {0, 0, EdgeTxStyles::EDIT_FLD_WIDTH_NARROW, 0}, vmin, vmax,
      [=]() { return vos->value; },
      [=](int newValue) {
        vos->setNumeric(newValue);
        if (onChanged) onChanged();
      },
      textFlags);
  num_field->setDefault(vdefault);

  // The Source button
  m_srcBtn = new TextButton(this, {EdgeTxStyles::EDIT_FLD_WIDTH_NARROW + PAD_TINY, 0, SRC_BTN_W, 0}, "SRC", [=]() {
    // Toggle source mode
    if (vos->isSource) {
      vos->setNumeric(0);
    } else {
      vos->isSource = 1;
      vos->srcType = SOURCE_TYPE_GVAR;
      vos->value = 0;
    }
    if (onChanged) onChanged();
    update();
    return vos->isSource != 0;
  });
  m_srcBtn->check(vos->isSource);

  // update field type based on value
  update();
}

void SourceNumberEdit::setSuffix(const std::string& value)
{
  num_field->setSuffix(value);
}

void SourceNumberEdit::update()
{
  bool has_focus = act_field && act_field->hasFocus();

  source_field->hide();
  num_field->hide();

  if (vos->isSource) {
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

  m_srcBtn->check(vos->isSource);

  if (has_focus) {
    lv_group_focus_obj(act_field->getLvObj());
  }
}
