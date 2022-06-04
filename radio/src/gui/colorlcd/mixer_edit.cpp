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

#include "mixer_edit.h"
#include "channel_bar.h"
#include "gvar_numberedit.h"
#include "model_curves.h"

#include "opentx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

#if (LCD_W > LCD_H)
  #define MIX_STATUS_BAR_WIDTH 250
  #define MIX_STATUS_BAR_MARGIN 3
  #define MIX_RIGHT_MARGIN 0
#else
  #define MIX_STATUS_BAR_WIDTH 180
  #define MIX_STATUS_BAR_MARGIN 0
  #define MIX_RIGHT_MARGIN 3
#endif

class MixerEditStatusBar : public Window
{
  public:
    MixerEditStatusBar(Window *parent, const rect_t &rect, int8_t channel) :
      Window(parent, rect),
      _channel(channel)
    {
      channelBar = new ComboChannelBar(this, {MIX_STATUS_BAR_MARGIN, 0, rect.w - (MIX_STATUS_BAR_MARGIN * 2), rect.h}, channel);
      channelBar->setLeftMargin(0);
      channelBar->setTextColor(COLOR_THEME_PRIMARY2);
      channelBar->setOutputChannelBarLimitColor(COLOR_THEME_EDIT);
    }

    void paint(BitmapBuffer *dc) override
    {
      // dc->clear(COLOR_THEME_SECONDARY2);
    }

  protected:
    ComboChannelBar *channelBar;
    int8_t _channel;
};

MixEditWindow::MixEditWindow(int8_t channel, uint8_t mixIndex) :
    Page(ICON_MODEL_MIXER), channel(channel), mixIndex(mixIndex)
{
  buildBody(&body);
  buildHeader(&header);
}

void MixEditWindow::buildHeader(Window *window)
{
  new StaticText(window,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                  PAGE_LINE_HEIGHT},
                 STR_MIXES, 0, COLOR_THEME_PRIMARY2);
  new StaticText(window,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                  LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                 getSourceString(MIXSRC_CH1 + channel), 0,
                 COLOR_THEME_PRIMARY2);

  statusBar = new MixerEditStatusBar(
      window,
      {window->getRect().w - MIX_STATUS_BAR_WIDTH - MIX_RIGHT_MARGIN, 0,
       MIX_STATUS_BAR_WIDTH, MENU_HEADER_HEIGHT + 3},
      channel);
}

void MixEditWindow::buildBody(FormWindow *window)
{
  FormGridLayout grid;
  grid.spacer(8);

  MixData *mix = mixAddress(mixIndex);

  // Mix name
  new StaticText(window, grid.getLabelSlot(), STR_MIXNAME, 0,
                 COLOR_THEME_PRIMARY1);
  new ModelTextEdit(window, grid.getFieldSlot(), mix->name, sizeof(mix->name));
  grid.nextLine();

  // Source
  new StaticText(window, grid.getLabelSlot(), STR_SOURCE, 0,
                 COLOR_THEME_PRIMARY1);
  new SourceChoice(window, grid.getFieldSlot(), 0, MIXSRC_LAST,
                   GET_SET_DEFAULT(mix->srcRaw));
  grid.nextLine();

  // Weight
  new StaticText(window, grid.getLabelSlot(), STR_WEIGHT, 0,
                 COLOR_THEME_PRIMARY1);
  auto gvar = new GVarNumberEdit(window, grid.getFieldSlot(), MIX_WEIGHT_MIN,
                                 MIX_WEIGHT_MAX, GET_SET_DEFAULT(mix->weight));
  gvar->setSuffix("%");
  grid.nextLine();

  // Offset
  new StaticText(window, grid.getLabelSlot(), STR_OFFSET, 0,
                 COLOR_THEME_PRIMARY1);
  gvar = new GVarNumberEdit(window, grid.getFieldSlot(), MIX_OFFSET_MIN,
                            MIX_OFFSET_MAX, GET_SET_DEFAULT(mix->offset));
  gvar->setSuffix("%");
  grid.nextLine();

  // Trim
  new StaticText(window, grid.getLabelSlot(), STR_TRIM, 0,
                 COLOR_THEME_PRIMARY1);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_INVERTED(mix->carryTrim));
  grid.nextLine();

  // Curve
  new StaticText(&body, grid.getLabelSlot(), STR_CURVE, 0,
                 COLOR_THEME_PRIMARY1);
  new Choice(&body, grid.getFieldSlot(2, 0), "\004DiffExpoFuncCstm", 0,
             CURVE_REF_CUSTOM, GET_DEFAULT(mix->curve.type),
             [=](int32_t newValue) {
               mix->curve.type = newValue;
               mix->curve.value = 0;
               SET_DIRTY();
               updateCurveParamField(mix);
             });
  curveParamField =
      new FormGroup(&body, grid.getFieldSlot(2, 1)  // , FORM_FORWARD_FOCUS
      );
  updateCurveParamField(mix);
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
          BFBIT_FLIP(mix->flightModes, bfBit<uint32_t>(i));
          SET_DIRTY();
          return !(bfSingleBitGet(mix->flightModes, i));
        },
        OPAQUE | (bfSingleBitGet(mix->flightModes, i) ? 0 : BUTTON_CHECKED));
  }
  grid.nextLine();
#endif

  // Switch
  new StaticText(window, grid.getLabelSlot(), STR_SWITCH, 0,
                 COLOR_THEME_PRIMARY1);
  new SwitchChoice(window, grid.getFieldSlot(), SWSRC_FIRST_IN_MIXES,
                   SWSRC_LAST_IN_MIXES, GET_SET_DEFAULT(mix->swtch));
  grid.nextLine();

  // Warning
  new StaticText(window, grid.getLabelSlot(), STR_MIXWARNING, 0,
                 COLOR_THEME_PRIMARY1);
  auto edit = new NumberEdit(window, grid.getFieldSlot(2, 0), 0, 3,
                             GET_SET_DEFAULT(mix->mixWarn));
  edit->setZeroText(STR_OFF);
  grid.nextLine();

  // Multiplex
  new StaticText(window, grid.getLabelSlot(), STR_MULTPX, 0,
                 COLOR_THEME_PRIMARY1);
  new Choice(window, grid.getFieldSlot(), STR_VMLTPX, 0, 2,
             GET_SET_DEFAULT(mix->mltpx));
  grid.nextLine();

  // Delay up
  new StaticText(window, grid.getLabelSlot(), STR_DELAYUP, 0,
                 COLOR_THEME_PRIMARY1);
  edit = new NumberEdit(window, grid.getFieldSlot(2, 0), 0, DELAY_MAX,
                        GET_DEFAULT(mix->delayUp),
                        SET_VALUE(mix->delayUp, newValue), 0, PREC1);
  edit->setSuffix("s");
  grid.nextLine();

  // Delay down
  new StaticText(window, grid.getLabelSlot(), STR_DELAYDOWN, 0,
                 COLOR_THEME_PRIMARY1);
  edit = new NumberEdit(window, grid.getFieldSlot(2, 0), 0, DELAY_MAX,
                        GET_DEFAULT(mix->delayDown),
                        SET_VALUE(mix->delayDown, newValue), 0, PREC1);
  edit->setSuffix("s");
  grid.nextLine();

  // Slow up
  new StaticText(window, grid.getLabelSlot(), STR_SLOWUP, 0,
                 COLOR_THEME_PRIMARY1);
  edit = new NumberEdit(window, grid.getFieldSlot(2, 0), 0, DELAY_MAX,
                        GET_DEFAULT(mix->speedUp),
                        SET_VALUE(mix->speedUp, newValue), 0, PREC1);
  edit->setSuffix("s");
  grid.nextLine();

  // Slow down
  new StaticText(window, grid.getLabelSlot(), STR_SLOWDOWN, 0,
                 COLOR_THEME_PRIMARY1);
  edit = new NumberEdit(window, grid.getFieldSlot(2, 0), 0, DELAY_MAX,
                        GET_DEFAULT(mix->speedDown),
                        SET_VALUE(mix->speedDown, newValue), 0, PREC1);
  edit->setSuffix("s");
  grid.nextLine();
}

// TODO share this code with INPUT
void MixEditWindow::updateCurveParamField(MixData *line)
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
        MixData *mix = mixAddress(mixIndex);
        // if no curve is specified then dont link to curve page
        if (mix->curve.value != 0)
          ModelCurvesPage::pushEditCurve(abs(mix->curve.value) - 1);
      });
      break;
    }
  }
}
