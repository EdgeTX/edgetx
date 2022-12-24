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
#include "fm_matrix.h"

#include "curve_param.h"
#include "gvar_numberedit.h"
#include "input_source.h"
#include "curveedit.h"

#include "opentx.h"
#include "switches.h"
#include "switchchoice.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

// Grid description for inner and outer grids
static const lv_coord_t col_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

InputEditWindow::InputEditWindow(int8_t input, uint8_t index) :
    Page(ICON_MODEL_INPUTS),
    input(input),
    index(index)
{
  ExpoData* inputData = expoAddress(index);

  body->padAll(PAD_ZERO);

  header->setTitle(STR_MENUINPUTS);
  headerSwitchName = header->setTitle2("");

  lv_obj_set_style_text_color(headerSwitchName->getLvObj(), makeLvColor(COLOR_THEME_ACTIVE), LV_STATE_USER_1);
  lv_obj_set_style_text_font(headerSwitchName->getLvObj(), getFont(FONT(BOLD)), LV_STATE_USER_1);

  active = !isActive(inputData);

  setTitle();

  // Outer grid form
  auto form = new Window(body, rect_t{});
  form->padAll(PAD_ZERO);
  form->setFlexLayout();

  FlexGridLayout grid(col_dsc, row_dsc);

  auto line = form->newLine(grid);
  line->padAll(PAD_ZERO);

#if LCD_H > LCD_W // portrait
  lv_obj_set_flex_flow(line->getLvObj(), LV_FLEX_FLOW_COLUMN);
#else // landscape
  lv_obj_set_flex_flow(line->getLvObj(), LV_FLEX_FLOW_ROW);
#endif

#if LCD_W > LCD_H // landscape (preview on left)
  // Preview grid box - force width and height
  auto box = new Window(line, rect_t{0, 0, INPUT_EDIT_CURVE_WIDTH + 8, body->height()});
  box->padAll(PAD_ZERO);

  // Add preview and buttons
  buildPreview(box, inputData);
#endif

  // Inner box for main controls - force width and height
#if LCD_H > LCD_W // portrait
  auto box = new Window(line, rect_t{0, 0, body->width(), body->height() - INPUT_EDIT_CURVE_HEIGHT - 68});
#else
  box = new Window(line, rect_t{0, 0, body->width() - INPUT_EDIT_CURVE_WIDTH - 12, body->height()});
#endif
  box->padAll(PAD_ZERO);
  etx_scrollbar(box->getLvObj());

  // Add main controls
  buildBody(box, inputData);

#if LCD_H > LCD_W // portrait (preview below)
  // Preview grid box - force width and height
  box = new Window(line, rect_t{0, 0, body->width(), INPUT_EDIT_CURVE_HEIGHT + 62});
  box->padAll(PAD_ZERO);

  // Add preview and buttons
  buildPreview(box, inputData);
#endif

  CurveEdit::SetCurrentSource(expoAddress(index)->srcRaw);
}

bool InputEditWindow::isActive(ExpoData* inputData)
{
  return getSwitch(inputData->swtch);
}

void InputEditWindow::checkEvents() 
{
  Page::checkEvents();

  ExpoData* inputData = expoAddress(index);

  if (active != isActive(inputData)) {
    active = isActive(inputData);
    if (active) {
      lv_obj_add_state(headerSwitchName->getLvObj(), LV_STATE_USER_1);
    } else {
      lv_obj_clear_state(headerSwitchName->getLvObj(), LV_STATE_USER_1);
    }
  }

  static uint8_t lastActiveIdx = MAX_EXPOS;

  for (uint8_t i=0; i<MAX_EXPOS; i++) {
    ExpoData * ed = expoAddress(i);
    if (!EXPO_VALID(ed)) break; // end of list
    if ((ed->srcRaw == inputData->srcRaw) && getSwitch(ed->swtch)) {
      if (lastActiveIdx != i) {
        lastActiveIdx = i;
        preview->update();
        break;
      }
    }
  }
}

int16_t InputEditWindow::getExpo(ExpoData* ed, int16_t val)
{
  int32_t v = 0;
  if (EXPO_VALID(ed)) {
    v = val;
    if (EXPO_MODE_ENABLE(ed, v)) {

      //========== CURVE=================
      if (ed->curve.value) {
        v = applyCurve(v, ed->curve);
      }

      //========== WEIGHT ===============
      int32_t weight = GET_GVAR_PREC1(ed->weight, -100, 100, mixerCurrentFlightMode);
      v = divRoundClosest((int32_t)v * weight, 1000);

      //========== OFFSET ===============
      int32_t offset = GET_GVAR_PREC1(ed->offset, -100, 100, mixerCurrentFlightMode);
      if (offset) v += divRoundClosest(calc100toRESX(offset), 10);
    }
  }
  return v;
}

void InputEditWindow::setTitle()
{
  headerSwitchName->setText(getSourceString(MIXSRC_FIRST_INPUT + input));
}

static const lv_coord_t b_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};

void InputEditWindow::buildBody(Window* box, ExpoData* inputData)
{
  lv_obj_set_scrollbar_mode(box->getLvObj(), LV_SCROLLBAR_MODE_AUTO);
  auto form = new Window(box, rect_t{});
  form->padAll(PAD_ZERO);
  form->setFlexLayout();

  FlexGridLayout grid(b_col_dsc, row_dsc, PAD_SMALL);

  // Input Name
  auto line = form->newLine(grid);
  auto inputName = g_model.inputNames[inputData->chn];
  new StaticText(line, rect_t{}, STR_INPUTNAME);
  auto nameFld = new ModelTextEdit(line, rect_t{0, 0, LCD_W*3/10-8, 0}, inputName, LEN_INPUT_NAME);
  nameFld->setChangeHandler([=]() {
    setTitle();
    SET_DIRTY();
  });

  // Line Name
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_EXPONAME);
  new ModelTextEdit(line, rect_t{0, 0, LCD_W*3/10-8, 0}, inputData->name, LEN_EXPOMIX_NAME);

  // Source
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_SOURCE);
  new InputSource(line, inputData);

  // Switch
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_SWITCH);
  new SwitchChoice(line, rect_t{}, SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES, GET_SET_DEFAULT(inputData->swtch));

  // Weight
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_WEIGHT);
  auto gvar = new GVarNumberEdit(line, -100, 100,
                                 GET_DEFAULT(inputData->weight),
                                 [=](int32_t newValue) {
                                   inputData->weight = newValue;
                                   preview->update();
                                   SET_DIRTY();
                                 });
  gvar->setSuffix("%");

  // Offset
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_OFFSET);
  gvar = new GVarNumberEdit(line, -100, 100,
                            GET_DEFAULT(inputData->offset),
                            [=](int32_t newValue) {
                              inputData->offset = newValue;
                              preview->update();
                              SET_DIRTY();
                            });
  gvar->setSuffix("%");

  // Curve
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_CURVE);
  new CurveParam(line, rect_t{}, &inputData->curve,
                 [=](int32_t newValue) {
                   inputData->curve.value = newValue;
                   preview->update();
                   SET_DIRTY();
                 });

  // Trim
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_TRIM);
  const auto trimLast = TRIM_OFF + keysGetMaxTrims() - 1;
  auto c = new Choice(line, rect_t{}, -TRIM_OFF, trimLast,
                      GET_VALUE(-inputData->trimSource),
                      SET_VALUE(inputData->trimSource, -newValue));
  c->setAvailableHandler([=](int value) {
    return value != TRIM_ON || inputData->srcRaw <= MIXSRC_LAST_STICK;
  });
  c->setTextHandler([=](int value) -> std::string {
    return getTrimSourceLabel(inputData->srcRaw, -value);
  });

  // Flight modes
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_FLMODE);
  new FMMatrix<ExpoData>(line, rect_t{}, inputData, 3);

#if LCD_W > LCD_H
  line->padBottom(8);
#endif
}

void InputEditWindow::buildPreview(Window* box, ExpoData* inputData)
{
  lv_coord_t xo = (box->width() - INPUT_EDIT_CURVE_WIDTH) / 2;
  lv_coord_t yo = (box->height() - (INPUT_EDIT_CURVE_HEIGHT + 56)) / 2;

  static bool showActive = true;
  auto aBtn = new TextButton(box, rect_t{xo, yo, INPUT_EDIT_CURVE_WIDTH, 24}, STR_SHOW_ACTIVE);
  aBtn->padAll(PAD_ZERO);
  aBtn->check(showActive);
  aBtn->setPressHandler([=]() {
    showActive = !showActive;
    preview->update();
    return showActive;
  });

  preview = new Curve(box, rect_t{ xo, yo + 28, INPUT_EDIT_CURVE_WIDTH, INPUT_EDIT_CURVE_HEIGHT },
      [=](int x) -> int {
        if (showActive) {
          int16_t anas[MAX_INPUTS] = {0};
          applyExpos(anas, e_perout_mode_inactive_flight_mode, inputData->srcRaw, x);
          return anas[inputData->chn];
        } else {
          return getExpo(inputData, x);
        }
      },
      [=]() -> int { return getValue(expoAddress(index)->srcRaw); });

  auto sBtn1 = new TextButton(box, rect_t{xo, yo + INPUT_EDIT_CURVE_HEIGHT + 4 + 28, INPUT_EDIT_CURVE_WIDTH/2 - 4, 24}, STR_VCURVEFUNC[2]);
  sBtn1->padAll(PAD_ZERO);

  auto sBtn2 = new TextButton(box, rect_t{xo + INPUT_EDIT_CURVE_WIDTH / 2 + 4, yo + INPUT_EDIT_CURVE_HEIGHT + 4 + 28, INPUT_EDIT_CURVE_WIDTH/2 -4, 24}, STR_VCURVEFUNC[1]);
  sBtn2->padAll(PAD_ZERO);

  sBtn1->setPressHandler([=]() {
    if (sBtn1->checked()) {
      inputData->mode = inputData->mode & (~1);
      if ((inputData->mode & 3) == 0)
        inputData->mode = inputData->mode | 2;
    } else {
      inputData->mode = inputData->mode | 1;
    }
    SET_DIRTY();
    preview->update();
    sBtn2->check(inputData->mode & 2);
    return (inputData->mode & 1) != 0;
  });

  sBtn2->setPressHandler([=]() {
    if (sBtn2->checked()) {
      inputData->mode = inputData->mode & (~2);
      if ((inputData->mode & 3) == 0)
        inputData->mode = inputData->mode | 1;
    } else {
      inputData->mode = inputData->mode | 2;
    }
    SET_DIRTY();
    preview->update();
    sBtn1->check(inputData->mode & 1);
    return (inputData->mode & 2) != 0;
  });

  sBtn1->check(inputData->mode & 1);
  sBtn2->check(inputData->mode & 2);
}

void InputEditWindow::deleteLater(bool detach, bool trash)
{
  if (!deleted()) {
    CurveEdit::SetCurrentSource(0);
    Page::deleteLater(detach, trash);
  }
}
