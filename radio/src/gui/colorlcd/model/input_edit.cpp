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
#include "edgetx.h"
#include "fm_matrix.h"
#include "gvar_numberedit.h"
#include "input_source.h"
#include "source_numberedit.h"
#include "switchchoice.h"
#include "switches.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

extern int32_t getSourceNumFieldValue(int16_t val, int16_t min, int16_t max);

InputEditWindow::InputEditWindow(int8_t input, uint8_t index) :
    Page(ICON_MODEL_INPUTS), input(input), index(index)
{
  ExpoData* inputData = expoAddress(index);

  body->padAll(PAD_ZERO);

  header->setTitle(STR_MENUINPUTS);
  headerSwitchName = header->setTitle2("");

  etx_txt_color(headerSwitchName->getLvObj(), COLOR_THEME_ACTIVE_INDEX,
                LV_STATE_USER_1);
  etx_font(headerSwitchName->getLvObj(), FONT_BOLD_INDEX, LV_STATE_USER_1);

  setTitle();

  // Outer grid form
  auto form = new Window(body, rect_t{});
  form->padAll(PAD_ZERO);

#if PORTRAIT_LCD
  form->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);
#else  // landscape
  form->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_TINY);
#endif

  Window* box;

#if !PORTRAIT_LCD  // landscape (preview on left)
  // Preview grid box - force width and height
  box = new Window(form,
                   rect_t{0, 0, INPUT_EDIT_CURVE_WIDTH + 8, body->height()});
  box->padAll(PAD_ZERO);

  // Add preview and buttons
  buildPreview(box, inputData);
#endif

  // Inner box for main controls - force width and height
#if PORTRAIT_LCD
  box = new Window(form, rect_t{0, 0, body->width(),
                                body->height() - INPUT_EDIT_CURVE_HEIGHT - 68});
#else
  box =
      new Window(form, rect_t{0, 0, body->width() - INPUT_EDIT_CURVE_WIDTH - 12,
                              body->height()});
#endif
  box->padAll(PAD_ZERO);
  etx_scrollbar(box->getLvObj());

  // Add main controls
  buildBody(box, inputData);

#if PORTRAIT_LCD  // portrait (preview below)
  // Preview grid box - force width and height
  box = new Window(form,
                   rect_t{0, 0, body->width(), INPUT_EDIT_CURVE_HEIGHT + 62});
  box->padAll(PAD_ZERO);

  // Add preview and buttons
  buildPreview(box, inputData);
#endif
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
      int32_t weight = getSourceNumFieldValue(ed->weight, -100, 100);
      v = divRoundClosest((int32_t)v * weight, 1000);

      //========== OFFSET ===============
      int32_t offset = getSourceNumFieldValue(ed->offset, -100, 100);
      if (offset) v += divRoundClosest(calc100toRESX(offset), 10);
    }
  }
  return v;
}

void InputEditWindow::setTitle()
{
  headerSwitchName->setText(getSourceString(MIXSRC_FIRST_INPUT + input));
}

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

void InputEditWindow::buildBody(Window* box, ExpoData* inputData)
{
  auto form = new Window(box, rect_t{});
  form->padAll(PAD_TINY);
  form->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

  // Input Name
  auto line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_INPUTNAME);
  new ModelTextEdit(line, {0, 0, LCD_W * 3 / 10 - 8, 0},
                    g_model.inputNames[inputData->chn], LEN_INPUT_NAME,
                    [=]() { setTitle(); });

  // Line Name
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_EXPONAME);
  new ModelTextEdit(line, {0, 0, LCD_W * 3 / 10 - 8, 0}, inputData->name,
                    LEN_EXPOMIX_NAME);

  // Source
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_SOURCE);
  new InputSource(line, inputData);

  // Switch
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_SWITCH);
  new SwitchChoice(line, rect_t{}, SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES,
                   GET_SET_DEFAULT(inputData->swtch));

  // Weight
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_WEIGHT);
  auto gvar = new SourceNumberEdit(
      line, -100, 100, GET_DEFAULT(inputData->weight),
      [=](int32_t newValue) {
        inputData->weight = newValue;
        updatePreview = true;
        SET_DIRTY();
      },
      MIXSRC_FIRST);
  gvar->setSuffix("%");

  // Offset
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_OFFSET);
  gvar = new SourceNumberEdit(
      line, -100, 100, GET_DEFAULT(inputData->offset),
      [=](int32_t newValue) {
        inputData->offset = newValue;
        updatePreview = true;
        SET_DIRTY();
      },
      MIXSRC_FIRST);
  gvar->setSuffix("%");

  // Curve
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_CURVE);
  new CurveParam(
      line, rect_t{}, &inputData->curve,
      [=](int32_t newValue) {
        inputData->curve.value = newValue;
        updatePreview = true;
        SET_DIRTY();
      },
      MIXSRC_FIRST, inputData->srcRaw,
      [=]() {
        updatePreview = true;
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

#if !PORTRAIT_LCD
  line->padBottom(PAD_LARGE);
#endif
}

void InputEditWindow::buildPreview(Window* box, ExpoData* inputData)
{
  lv_coord_t xo = (box->width() - INPUT_EDIT_CURVE_WIDTH) / 2;
  lv_coord_t yo = (box->height() - (INPUT_EDIT_CURVE_HEIGHT + 56)) / 2;

  static bool showActive = true;
  auto aBtn = new TextButton(box, rect_t{xo, yo, INPUT_EDIT_CURVE_WIDTH, 24},
                             STR_SHOW_ACTIVE);
  aBtn->padAll(PAD_ZERO);
  aBtn->check(showActive);
  aBtn->setPressHandler([=]() {
    showActive = !showActive;
    updatePreview = true;
    return showActive;
  });

  preview = new Curve(
      box, rect_t{xo, yo + 28, INPUT_EDIT_CURVE_WIDTH, INPUT_EDIT_CURVE_HEIGHT},
      [=](int x) -> int {
        if (showActive) {
          int16_t anas[MAX_INPUTS] = {0};
          applyExpos(anas, e_perout_mode_inactive_flight_mode,
                     inputData->srcRaw, x);
          return anas[inputData->chn];
        } else {
          return getExpo(inputData, x);
        }
      },
      [=]() -> int { return getValue(expoAddress(index)->srcRaw); });

  auto sBtn1 = new TextButton(box,
                              rect_t{xo, yo + INPUT_EDIT_CURVE_HEIGHT + 4 + 28,
                                     INPUT_EDIT_CURVE_WIDTH / 2 - 4, 24},
                              STR_VCURVEFUNC[2]);
  sBtn1->padAll(PAD_ZERO);

  auto sBtn2 = new TextButton(box,
                              rect_t{xo + INPUT_EDIT_CURVE_WIDTH / 2 + 4,
                                     yo + INPUT_EDIT_CURVE_HEIGHT + 4 + 28,
                                     INPUT_EDIT_CURVE_WIDTH / 2 - 4, 24},
                              STR_VCURVEFUNC[1]);
  sBtn2->padAll(PAD_ZERO);

  sBtn1->setPressHandler([=]() {
    if (sBtn1->checked()) {
      inputData->mode = inputData->mode & (~1);
      if ((inputData->mode & 3) == 0) inputData->mode = inputData->mode | 2;
    } else {
      inputData->mode = inputData->mode | 1;
    }
    SET_DIRTY();
    updatePreview = true;
    sBtn2->check(inputData->mode & 2);
    return (inputData->mode & 1) != 0;
  });

  sBtn2->setPressHandler([=]() {
    if (sBtn2->checked()) {
      inputData->mode = inputData->mode & (~2);
      if ((inputData->mode & 3) == 0) inputData->mode = inputData->mode | 1;
    } else {
      inputData->mode = inputData->mode | 2;
    }
    SET_DIRTY();
    updatePreview = true;
    sBtn1->check(inputData->mode & 1);
    return (inputData->mode & 2) != 0;
  });

  sBtn1->check(inputData->mode & 1);
  sBtn2->check(inputData->mode & 2);
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
