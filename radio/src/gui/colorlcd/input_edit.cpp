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

#include "input_edit.h"
#include "input_edit_adv.h"

#include "curve_param.h"
#include "gvar_numberedit.h"
#include "input_source.h"
#include "curveedit.h"

#include "opentx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

InputEditWindow::InputEditWindow(int8_t input, uint8_t index) :
    Page(ICON_MODEL_INPUTS),
    input(input),
    index(index)
{
  std::string title2(getSourceString(MIXSRC_FIRST_INPUT + input));
  header.setTitle(STR_MENUINPUTS);
  header.setTitle2(title2);

  auto body_obj = body.getLvObj();
#if LCD_H > LCD_W // portrait
  lv_obj_set_flex_flow(body_obj, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_pad_row(body_obj, lv_dpx(8), 0);
#else // landscape
  lv_obj_set_flex_flow(body_obj, LV_FLEX_FLOW_ROW);
  lv_obj_set_style_pad_column(body_obj, lv_dpx(8), 0);
#endif
  lv_obj_set_style_flex_cross_place(body_obj, LV_FLEX_ALIGN_CENTER, 0);
    
  // TODO: would be better to set the padding on the preview window...
  //       ...but it doesn't support it yet.
  lv_obj_set_style_pad_all(body_obj, lv_dpx(8), 0);

  auto box = new Window(&body, rect_t{});
  auto box_obj = box->getLvObj();
  lv_obj_set_flex_grow(box_obj, 2);
  lv_obj_set_scrollbar_mode(box->getLvObj(), LV_SCROLLBAR_MODE_AUTO);

#if LCD_H > LCD_W // portrait
  box->setWidth(body.width() - 2*lv_dpx(8));
#else // landscape
  box->setHeight(body.height() - 2*lv_dpx(8));
#endif

  auto form = new FormWindow(box, rect_t{});
  auto form_obj = form->getLvObj();
  lv_obj_set_style_pad_all(form_obj, lv_dpx(8), 0);
  buildBody(form);

  preview = new Curve(&body, rect_t{0, 0, INPUT_EDIT_CURVE_WIDTH, INPUT_EDIT_CURVE_HEIGHT},
      [=](int x) -> int {
        ExpoData* line = expoAddress(index);
        int16_t anas[MAX_INPUTS] = {0};
        applyExpos(anas, e_perout_mode_inactive_flight_mode, line->srcRaw, x);
        return anas[line->chn];
      },
      [=]() -> int { return getValue(expoAddress(index)->srcRaw); });

  CurveEdit::SetCurrentSource(expoAddress(index)->srcRaw);
}

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

void InputEditWindow::buildBody(FormWindow* form)
{
  FlexGridLayout grid(col_dsc, row_dsc, 2);
  form->setFlexLayout();

  ExpoData* input = expoAddress(index);

  // Input Name
  auto line = form->newLine(&grid);
  auto inputName = g_model.inputNames[input->chn];
  new StaticText(line, rect_t{}, STR_INPUTNAME, 0, COLOR_THEME_PRIMARY1);
  new ModelTextEdit(line, rect_t{}, inputName, LEN_INPUT_NAME);

  // Line Name
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_EXPONAME, 0, COLOR_THEME_PRIMARY1);
  new ModelTextEdit(line, rect_t{}, input->name, LEN_EXPOMIX_NAME);

  // Source
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_SOURCE, 0, COLOR_THEME_PRIMARY1);
  auto src = new InputSource(line, input);
  lv_obj_set_style_grid_cell_x_align(src->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

  // Weight
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_WEIGHT, 0, COLOR_THEME_PRIMARY1);
  auto gvar = new GVarNumberEdit(line, rect_t{}, -100, 100,
                                 GET_DEFAULT(input->weight),
                                 [=](int32_t newValue) {
                                   input->weight = newValue;
                                   preview->invalidate();
                                   SET_DIRTY();
                                 });
  gvar->setSuffix("%");

  // Offset
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_OFFSET, 0, COLOR_THEME_PRIMARY1);
  gvar = new GVarNumberEdit(line, rect_t{}, -100, 100,
                            GET_DEFAULT(input->offset),
                            [=](int32_t newValue) {
                              input->offset = newValue;
                              preview->invalidate();
                              SET_DIRTY();
                            });
  gvar->setSuffix("%");

  // Switch
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_SWITCH, 0, COLOR_THEME_PRIMARY1);
  new SwitchChoice(line, rect_t{}, SWSRC_FIRST_IN_MIXES,
                   SWSRC_LAST_IN_MIXES, GET_SET_DEFAULT(input->swtch));

  // Curve
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_CURVE, 0, COLOR_THEME_PRIMARY1);
  auto param = new CurveParam(line, rect_t{}, &input->curve,
                              [=](int32_t newValue) {
                                input->curve.value = newValue;
                                preview->invalidate();
                                SET_DIRTY();
                              });
  lv_obj_set_style_grid_cell_x_align(param->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

  line = form->newLine();
  lv_obj_set_style_pad_all(line->getLvObj(), lv_dpx(8), 0);
  auto btn = new TextButton(line, rect_t{}, LV_SYMBOL_SETTINGS, [=]() -> uint8_t {
    new InputEditAdvanced(this->input, index);
    return 0;
  });
  lv_obj_set_width(btn->getLvObj(), lv_pct(100));
}

void InputEditWindow::deleteLater(bool detach, bool trash)
{
  if (!deleted()) {
    CurveEdit::SetCurrentSource(0);
    Page::deleteLater(detach, trash);
  }
}
