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

#include "curve_param.h"
#include "curveedit.h"
#include "gvar_numberedit.h"
#include "source_numberedit.h"
#include "input_edit_adv.h"
#include "input_source.h"
#include "edgetx.h"
#include "etx_lv_theme.h"
#include "switchchoice.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

InputEditWindow::InputEditWindow(int8_t input, uint8_t index) :
    Page(ICON_MODEL_INPUTS), input(input), index(index)
{
  header->setTitle(STR_MENUINPUTS);
  headerSwitchName = header->setTitle2("");

  etx_txt_color(headerSwitchName->getLvObj(), COLOR_THEME_ACTIVE_INDEX,
                LV_STATE_USER_1);
  etx_font(headerSwitchName->getLvObj(), FONT_BOLD_INDEX, LV_STATE_USER_1);

  setTitle();

  auto body_obj = body->getLvObj();
#if PORTRAIT_LCD  // portrait
  lv_obj_set_flex_flow(body_obj, LV_FLEX_FLOW_COLUMN);
#else  // landscape
  lv_obj_set_flex_flow(body_obj, LV_FLEX_FLOW_ROW);
#endif
  lv_obj_set_style_flex_cross_place(body_obj, LV_FLEX_ALIGN_CENTER, 0);

  auto box = new Window(body, rect_t{});
  auto box_obj = box->getLvObj();
  lv_obj_set_flex_grow(box_obj, 2);
  etx_scrollbar(box_obj);

#if PORTRAIT_LCD  // portrait
  box->setWidth(body->width() - 2 * PAD_MEDIUM);
#else  // landscape
  box->setHeight(body->height() - 2 * PAD_MEDIUM);
#endif

  auto form = new Window(box, rect_t{});
  buildBody(form);

  preview = new Curve(
      body, rect_t{0, 0, INPUT_EDIT_CURVE_WIDTH, INPUT_EDIT_CURVE_HEIGHT},
      [=](int x) -> int {
        ExpoData* line = expoAddress(index);
        int16_t anas[MAX_INPUTS] = {0};
        applyExpos(anas, e_perout_mode_inactive_flight_mode, line->srcRaw, x);
        return anas[line->chn];
      },
      [=]() -> int { return getValue(expoAddress(index)->srcRaw); });
}

void InputEditWindow::setTitle()
{
  headerSwitchName->setText(getSourceString(MIXSRC_FIRST_INPUT + input));
}

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

void InputEditWindow::buildBody(Window* form)
{
  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);
  form->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

  ExpoData* input = expoAddress(index);

  // Input Name
  auto line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_INPUTNAME);
  new ModelTextEdit(line, rect_t{}, g_model.inputNames[input->chn],
                    LEN_INPUT_NAME,
                    [=]() {
                      setTitle();
                    });

  // Line Name
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_EXPONAME);
  new ModelTextEdit(line, rect_t{}, input->name, LEN_EXPOMIX_NAME);

  // Source
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_SOURCE);
  auto src = new InputSource(line, input);
  lv_obj_set_style_grid_cell_x_align(src->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

  // Weight
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_WEIGHT);
  auto gvar =
      new SourceNumberEdit(line, -100, 100, GET_DEFAULT(input->weight),
                           [=](int32_t newValue) {
                             input->weight = newValue;
                             updatePreview = true;
                             SET_DIRTY();
                           }, MIXSRC_FIRST);
  gvar->setSuffix("%");

  // Offset
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_OFFSET);
  gvar = new SourceNumberEdit(line, -100, 100,
                              GET_DEFAULT(input->offset), [=](int32_t newValue) {
                                input->offset = newValue;
                                updatePreview = true;
                                SET_DIRTY();
                              }, MIXSRC_FIRST);
  gvar->setSuffix("%");

  // Switch
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_SWITCH);
  new SwitchChoice(line, rect_t{}, SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES,
                   GET_DEFAULT(input->swtch),
                   [=](int newValue) {
                     input->swtch = newValue;
                     updatePreview = true;
                     SET_DIRTY();
                   });

  // Curve
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_CURVE);
  auto param =
      new CurveParam(line, rect_t{}, &input->curve,
        [=](int32_t newValue) {
          input->curve.value = newValue;
          updatePreview = true;
          SET_DIRTY();
        }, MIXSRC_FIRST, input->srcRaw,
        [=]() {
          updatePreview = true;
        });
  lv_obj_set_style_grid_cell_x_align(param->getLvObj(), LV_GRID_ALIGN_STRETCH,
                                     0);

  line = form->newLine(grid);
  line->padAll(PAD_LARGE);
  auto btn =
      new TextButton(line, rect_t{}, LV_SYMBOL_SETTINGS, [=]() -> uint8_t {
        new InputEditAdvanced(this->input, index);
        return 0;
      });
  lv_obj_set_width(btn->getLvObj(), lv_pct(100));
}

void InputEditWindow::checkEvents()
{
  ExpoData* input = expoAddress(index);

  getvalue_t val;
  SourceNumVal v;

  v.rawValue = input->weight;
  if (v.isSource) {
    val = getValue(v.value);
    if (val != lastWeightVal) {
      lastWeightVal = val;
      updatePreview = true;
    }
  }

  v.rawValue = input->offset;
  if (v.isSource) {
    val = getValue(v.value);
    if (val != lastOffsetVal) {
      lastOffsetVal = val;
      updatePreview = true;
    }
  }

  v.rawValue = input->curve.value;
  if (v.isSource) {
    val = getValue(v.value);
    if (val != lastCurveVal) {
      lastCurveVal = val;
      updatePreview = true;
    }
  }

  uint8_t activeIdx = 255;
  for (int i = 0; i < MAX_EXPOS; i += 1) {
    auto inp = expoAddress(i);
    if (inp->chn == input->chn) {
      if (getSwitch(inp->swtch)) {
        activeIdx = i;
        break;
      }
    }
  }
  if (activeIdx != lastActiveIndex) {
    updatePreview = true;
    lastActiveIndex = activeIdx;
  }

  if (lastActiveIndex == index) {
    lv_obj_add_state(headerSwitchName->getLvObj(), LV_STATE_USER_1);
  } else {
    lv_obj_clear_state(headerSwitchName->getLvObj(), LV_STATE_USER_1);
  }

  if (updatePreview) {
    updatePreview = false;
    if (preview) preview->update();
  }

  Page::checkEvents();
}
