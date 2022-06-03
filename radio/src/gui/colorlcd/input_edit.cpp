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
#include "model_curves.h"

#include "opentx.h"
#include "gvar_numberedit.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

class SensorValue : Window
{
 public:
  SensorValue(Window *parent, const rect_t &rect, ExpoData *expoData) :
      Window(parent, rect), expoData(expoData)
  {
  }
  void paint(BitmapBuffer *dc) override
  {
    if (isTelemetryValue()) {
      uint8_t sensorIndex = (expoData->srcRaw - MIXSRC_FIRST_TELEM) / 3;
#if defined(SIMU)
      if (true) {
#else
      TelemetryItem &telemetryItem = telemetryItems[sensorIndex];
      if (telemetryItem.isAvailable()) {
#endif
        LcdFlags flags = LEFT | COLOR_THEME_PRIMARY1;
        drawSensorCustomValue(dc, 3, 2, sensorIndex, lastSensorVal, flags);
      } else {
        dc->drawText(3, 2, "---", COLOR_THEME_PRIMARY1);
      }
    }
  }
  bool isTelemetryValue()
  {
    return expoData->srcRaw >= MIXSRC_FIRST_TELEM &&
           expoData->srcRaw <= MIXSRC_LAST_TELEM;
  }
  void checkEvents() override
  {
    getvalue_t sensorVal = -1;
    if (isTelemetryValue()) {
      sensorVal = getValue(expoData->srcRaw);
    }
    if (lastSensorVal != sensorVal) {
      lastSensorVal = sensorVal;
      invalidate();
    }
  }

  LcdFlags getSensorPrec()
  {
    LcdFlags prec = 0;
    if (isTelemetryValue()) {
      uint8_t sensorIndex = (expoData->srcRaw - MIXSRC_FIRST_TELEM) / 3;
      TelemetrySensor sensor = g_model.telemetrySensors[sensorIndex];
      if (sensor.prec > 0) {
        prec |= (sensor.prec == 1 ? PREC1 : PREC2);
      }
    }
    return prec;
  }

 protected:
  getvalue_t lastSensorVal;
  ExpoData *expoData;
};

InputEditWindow::InputEditWindow(int8_t input, uint8_t index) :
    Page(ICON_MODEL_INPUTS),
    input(input),
    index(index),
    preview(
        this,
        {INPUT_EDIT_CURVE_LEFT, INPUT_EDIT_CURVE_TOP, INPUT_EDIT_CURVE_WIDTH,
         INPUT_EDIT_CURVE_HEIGHT},
        [=](int x) -> int {
          ExpoData *line = expoAddress(index);
          int16_t anas[MAX_INPUTS] = {0};
          applyExpos(anas, e_perout_mode_inactive_flight_mode, line->srcRaw, x);
          return anas[line->chn];
        },
        [=]() -> int { return getValue(expoAddress(index)->srcRaw); })
{
#if LCD_W > LCD_H
  auto BODY_WIDTH = LCD_W - preview.width() - PAGE_PADDING;
  body.setWidth(BODY_WIDTH);
  body.setLeft(preview.width() + PAGE_PADDING);
#else
  body.setRect({0, INPUT_EDIT_CURVE_TOP + INPUT_EDIT_CURVE_HEIGHT, LCD_W,
                LCD_H - INPUT_EDIT_CURVE_TOP - INPUT_EDIT_CURVE_HEIGHT});
#endif
  buildBody(&body);
  buildHeader(&header);
}

void InputEditWindow::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;

  preview.deleteLater(true, false);

  Page::deleteLater(detach, trash);
}

void InputEditWindow::buildHeader(Window *window)
{
  new StaticText(window,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                  PAGE_LINE_HEIGHT},
                 STR_MENUINPUTS, 0, COLOR_THEME_PRIMARY2);
  new StaticText(window,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                  LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                 getSourceString(MIXSRC_FIRST_INPUT + input), 0,
                 COLOR_THEME_PRIMARY2);
}

// TODO share this code with MIXER
void InputEditWindow::updateCurveParamField(ExpoData *line)
{
  curveParamField->clear();

  const rect_t rect = {0, 0, curveParamField->width(),
                       curveParamField->height()};

  switch (line->curve.type) {
    case CURVE_REF_DIFF:
    case CURVE_REF_EXPO: {
      GVarNumberEdit *edit = new GVarNumberEdit(
          curveParamField, rect, -100, 100, GET_SET_DEFAULT(line->curve.value));
      edit->setSuffix("%");
      break;
    }

    case CURVE_REF_FUNC:
      new Choice(curveParamField, rect, STR_VCURVEFUNC, 0, CURVE_BASE - 1,
                 GET_SET_DEFAULT(line->curve.value));
      break;

    case CURVE_REF_CUSTOM: {
      auto choice = new ChoiceEx(curveParamField, rect, -MAX_CURVES, MAX_CURVES,
                                 GET_SET_DEFAULT(line->curve.value));
      choice->setTextHandler([](int value) { return getCurveString(value); });
      choice->setLongPressHandler([this](event_t event) {
        ExpoData *line = expoAddress(index);

        // if no curve is specified then dont link to curve page
        if (line->curve.value != 0)
          ModelCurvesPage::pushEditCurve(abs(line->curve.value) - 1);
      });
      break;
    }
  }
}

void InputEditWindow::buildBody(FormWindow *window)
{
  FormGridLayout grid;
  grid.setLabelWidth(INPUT_EDIT_LABELS_WIDTH);
  grid.spacer(PAGE_PADDING);

  ExpoData *line = expoAddress(index);

#if LCD_W > LCD_H
  grid.setMarginRight(180);
#endif

  // Input Name
  new StaticText(window, grid.getLabelSlot(), STR_INPUTNAME, 0,
                 COLOR_THEME_PRIMARY1);
  new ModelTextEdit(window, grid.getFieldSlot(), g_model.inputNames[line->chn],
                    sizeof(g_model.inputNames[line->chn]));
  grid.nextLine();

  // Switch
  new StaticText(window, grid.getLabelSlot(), STR_SWITCH, 0,
                 COLOR_THEME_PRIMARY1);
  new SwitchChoice(window, grid.getFieldSlot(), SWSRC_FIRST_IN_MIXES,
                   SWSRC_LAST_IN_MIXES, GET_SET_DEFAULT(line->swtch));
  grid.nextLine();

  // Side
  new StaticText(window, grid.getLabelSlot(), STR_SIDE, 0,
                 COLOR_THEME_PRIMARY1);
  new Choice(
      window, grid.getFieldSlot(), STR_VCURVEFUNC, 1, 3,
      [=]() -> int16_t { return 4 - line->mode; },
      [=](int16_t newValue) {
        line->mode = 4 - newValue;
        SET_DIRTY();
      });
  grid.nextLine();

  // Name
  new StaticText(window, grid.getLabelSlot(), STR_EXPONAME, 0,
                 COLOR_THEME_PRIMARY1);
  new ModelTextEdit(window, grid.getFieldSlot(), line->name,
                    sizeof(line->name));
  grid.nextLine();

  // Source
  new StaticText(window, grid.getLabelSlot(), STR_SOURCE, 0,
                 COLOR_THEME_PRIMARY1);
  new SourceChoice(
      window, grid.getFieldSlot(2, 0), INPUTSRC_FIRST, INPUTSRC_LAST,
      GET_DEFAULT(line->srcRaw), [=](int32_t newValue) {
        line->srcRaw = newValue;
        if (line->srcRaw > MIXSRC_Ail && line->carryTrim == TRIM_ON) {
          line->carryTrim = TRIM_OFF;
          trimChoice->invalidate();
        }
        window->clear();
        buildBody(window);
        SET_DIRTY();
      });

  SensorValue *sensor = nullptr;
  if (line->srcRaw >= MIXSRC_FIRST_TELEM) {
    sensor = new SensorValue(window, grid.getFieldSlot(2, 1), line);

    grid.nextLine();
    // Scale
    new StaticText(window, grid.getLabelSlot(), STR_SCALE, 0,
                   COLOR_THEME_PRIMARY1);
    new NumberEdit(window, grid.getFieldSlot(), 0,
                   maxTelemValue(line->srcRaw - MIXSRC_FIRST_TELEM + 1),
                   GET_SET_DEFAULT(line->scale), 0, sensor->getSensorPrec());
    adjustHeight();
  }
  grid.nextLine();

  // Weight
  new StaticText(window, grid.getLabelSlot(), STR_WEIGHT, 0,
                 COLOR_THEME_PRIMARY1);
  auto gvar = new GVarNumberEdit(window, grid.getFieldSlot(), -100, 100,
                                 GET_SET_DEFAULT(line->weight));
  gvar->setSuffix("%");
  grid.nextLine();

  // Offset
  new StaticText(window, grid.getLabelSlot(), STR_OFFSET, 0,
                 COLOR_THEME_PRIMARY1);
  gvar = new GVarNumberEdit(window, grid.getFieldSlot(), -100, 100,
                            GET_SET_DEFAULT(line->offset));
  gvar->setSuffix("%");
  grid.nextLine();

  // Trim
  new StaticText(window, grid.getLabelSlot(), STR_TRIM, 0,
                 COLOR_THEME_PRIMARY1);
  trimChoice = new Choice(window, grid.getFieldSlot(), STR_VMIXTRIMS, -TRIM_OFF,
                          -TRIM_LAST, GET_VALUE(-line->carryTrim),
                          SET_VALUE(line->carryTrim, -newValue));
  trimChoice->setAvailableHandler([=](int value) {
    return value != TRIM_ON || line->srcRaw <= MIXSRC_Ail;
  });
  grid.nextLine();

  // Curve
  new StaticText(&body, grid.getLabelSlot(), STR_CURVE, 0,
                 COLOR_THEME_PRIMARY1);
  new Choice(&body, grid.getFieldSlot(2, 0), "\004DiffExpoFuncCstm", 0,
             CURVE_REF_CUSTOM, GET_DEFAULT(line->curve.type),
             [=](int32_t newValue) {
               line->curve.type = newValue;
               line->curve.value = 0;
               SET_DIRTY();
               updateCurveParamField(line);
             });
  curveParamField = new FormGroup(&body, grid.getFieldSlot(2, 1)
                                  // , FORM_FORWARD_FOCUS
  );
  updateCurveParamField(line);
  grid.nextLine();

#if defined(FLIGHT_MODES)
  // Flight modes
  new StaticText(window, grid.getLabelSlot(), STR_FLMODE, 0,
                 COLOR_THEME_PRIMARY1);
  for (uint32_t i = 0; i < MAX_FLIGHT_MODES; i++) {
    char fm[2] = {char('0' + i), '\0'};
    if (i > 0 && (i % 4) == 0) grid.nextLine();
    new TextButton(
        window, grid.getFieldSlot(4, i % 4), fm,
        [=]() -> uint32_t {
          BFBIT_FLIP(line->flightModes, bfBit<uint32_t>(i));
          SET_DIRTY();
          return !(bfSingleBitGet(line->flightModes, i));
        },
        OPAQUE | (bfSingleBitGet(line->flightModes, i) ? 0 : BUTTON_CHECKED));
  }
  grid.nextLine();
#endif
}
