/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "model_inputs.h"
#include "opentx.h"
#include "gvar_numberedit.h"
#include "libopenui.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

#define PASTE_BEFORE    -2
#define PASTE_AFTER     -1

uint8_t getExposCount()
{
  uint8_t count = 0;
  uint8_t ch ;

  for (int i=MAX_EXPOS-1 ; i>=0; i--) {
    ch = EXPO_VALID(expoAddress(i));
    if (ch != 0) {
      count++;
    }
  }
  return count;
}

bool reachExposLimit()
{
  if (getExposCount() >= MAX_EXPOS) {
    POPUP_WARNING(STR_NOFREEEXPO);
    return true;
  }
  return false;
}

void copyExpo(uint8_t source, uint8_t dest, int8_t input)
{
  pauseMixerCalculations();
  ExpoData sourceExpo;
  memcpy(&sourceExpo, expoAddress(source), sizeof(ExpoData));
  ExpoData * expo = expoAddress(dest);

  if(input == PASTE_AFTER) {
    memmove(expo+2, expo+1, (MAX_EXPOS-(source+1))*sizeof(ExpoData));
    memcpy(expo+1, &sourceExpo, sizeof(ExpoData));
    (expo+1)->chn = (expo)->chn;
  }
  else if(input == PASTE_BEFORE) {
    memmove(expo+1, expo, (MAX_EXPOS-(source+1))*sizeof(ExpoData));
    memcpy(expo, &sourceExpo, sizeof(ExpoData));
    expo->chn = (expo+1)->chn;
  }
  else {
    memmove(expo+1, expo, (MAX_EXPOS-(source+1))*sizeof(ExpoData));
    memcpy(expo, &sourceExpo, sizeof(ExpoData));
    expo->chn = input;
  }
  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}

void deleteExpo(uint8_t idx)
{
  pauseMixerCalculations();
  ExpoData * expo = expoAddress(idx);
  int input = expo->chn;
  memmove(expo, expo+1, (MAX_EXPOS-(idx+1))*sizeof(ExpoData));
  memclear(&g_model.expoData[MAX_EXPOS-1], sizeof(ExpoData));
  if (!isInputAvailable(input)) {
    memclear(&g_model.inputNames[input], LEN_INPUT_NAME);
  }
  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}

class InputEditWindow : public Page
{
 public:
  InputEditWindow(int8_t input, uint8_t index) :
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
            applyExpos(anas, e_perout_mode_inactive_flight_mode, line->srcRaw,
                       x);
            return anas[line->chn];
          },
          [=]() -> int { return getValue(expoAddress(index)->srcRaw); })
  {
#if LCD_W > LCD_H
    body.setWidth(LCD_W - preview.width() - PAGE_PADDING);
    body.setLeft(preview.width() + PAGE_PADDING);
#else
    body.setRect({0, INPUT_EDIT_CURVE_TOP + INPUT_EDIT_CURVE_HEIGHT, LCD_W,
                  LCD_H - INPUT_EDIT_CURVE_TOP - INPUT_EDIT_CURVE_HEIGHT});
#endif
    buildBody(&body);
    buildHeader(&header);
  }

  void deleteLater(bool detach = true, bool trash = true) override
  {
    if (_deleted) return;

    preview.deleteLater(true, false);

    Page::deleteLater(detach, trash);
  }

 protected:
  uint8_t input;
  uint8_t index;
  Curve preview;
  Choice *trimChoice = nullptr;
  FormGroup *curveParamField = nullptr;

  void buildHeader(Window *window)
  {
    new StaticText(window,
                   {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                    PAGE_LINE_HEIGHT},
                   STR_MENUINPUTS, 0, FOCUS_COLOR);
    new StaticText(window,
                   {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                    LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                   getSourceString(MIXSRC_FIRST_INPUT + input), 0, FOCUS_COLOR);
  }

  // TODO share this code with MIXER
  void updateCurveParamField(ExpoData *line)
  {
    curveParamField->clear();

    const rect_t rect = {0, 0, curveParamField->width(),
                         curveParamField->height()};

    switch (line->curve.type) {
      case CURVE_REF_DIFF:
      case CURVE_REF_EXPO: {
        GVarNumberEdit *edit =
            new GVarNumberEdit(curveParamField, rect, -100, 100,
                               GET_SET_DEFAULT(line->curve.value));
        edit->setSuffix("%");
        break;
      }

      case CURVE_REF_FUNC:
        new Choice(curveParamField, rect, STR_VCURVEFUNC, 0, CURVE_BASE - 1,
                   GET_SET_DEFAULT(line->curve.value));
        break;

      case CURVE_REF_CUSTOM: {
        auto choice = new Choice(curveParamField, rect, -MAX_CURVES, MAX_CURVES,
                                 GET_SET_DEFAULT(line->curve.value));
        choice->setTextHandler([](int value) { return getCurveString(value); });
        break;
      }
    }
  }

  void buildBody(FormWindow *window)
  {
    FormGridLayout grid;
    grid.setLabelWidth(INPUT_EDIT_LABELS_WIDTH);
    grid.spacer(PAGE_PADDING);

    ExpoData *line = expoAddress(index);

#if LCD_W > LCD_H
      grid.setMarginRight(180);
#endif

      // Input Name
      new StaticText(window, grid.getLabelSlot(), STR_INPUTNAME);
      new ModelTextEdit(window, grid.getFieldSlot(), g_model.inputNames[line->chn], sizeof(g_model.inputNames[line->chn]));
      grid.nextLine();

      // Switch
      new StaticText(window, grid.getLabelSlot(), STR_SWITCH);
      new SwitchChoice(window, grid.getFieldSlot(), SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES, GET_SET_DEFAULT(line->swtch));
      grid.nextLine();

      // Side
      new StaticText(window, grid.getLabelSlot(), STR_SIDE);
      new Choice(window, grid.getFieldSlot(), STR_VCURVEFUNC, 1, 3,
                 [=]() -> int16_t {
                   return 4 - line->mode;
                 },
                 [=](int16_t newValue) {
                   line->mode = 4 - newValue;
                   SET_DIRTY();
                 });
      grid.nextLine();

      // Name
      new StaticText(window, grid.getLabelSlot(), STR_EXPONAME);
      new ModelTextEdit(window, grid.getFieldSlot(), line->name, sizeof(line->name));
      grid.nextLine();

      // Source
      new StaticText(window, grid.getLabelSlot(), STR_SOURCE);
      new SourceChoice(window, grid.getFieldSlot(), INPUTSRC_FIRST, INPUTSRC_LAST,
                       GET_DEFAULT(line->srcRaw),
                       [=] (int32_t newValue) {
                         line->srcRaw = newValue;
                         if (line->srcRaw > MIXSRC_Ail && line->carryTrim == TRIM_ON) {
                           line->carryTrim = TRIM_OFF;
                           trimChoice->invalidate();
                         }
                         SET_DIRTY();
                       }
      );
      /* TODO telemetry current value
      if (ed->srcRaw >= MIXSRC_FIRST_TELEM) {
        drawSensorCustomValue(EXPO_ONE_2ND_COLUMN+75, y, (ed->srcRaw - MIXSRC_FIRST_TELEM)/3, convertTelemValue(ed->srcRaw - MIXSRC_FIRST_TELEM + 1, ed->scale), LEFT|(menuHorizontalPosition==1?attr:0));
      } */
      grid.nextLine();

      // Scale
      // TODO only displayed when source is telemetry + unfinished
      new StaticText(window, grid.getLabelSlot(), STR_SCALE);
      new NumberEdit(window, grid.getFieldSlot(), -100, 100, GET_SET_DEFAULT(line->scale));
      grid.nextLine();

      // Weight
      new StaticText(window, grid.getLabelSlot(), STR_WEIGHT);
      auto gvar = new GVarNumberEdit(window, grid.getFieldSlot(), -100, 100, GET_SET_DEFAULT(line->weight));
      gvar->setSuffix("%");
      grid.nextLine();

      // Offset
      new StaticText(window, grid.getLabelSlot(), STR_OFFSET);
      gvar = new GVarNumberEdit(window, grid.getFieldSlot(), -100, 100, GET_SET_DEFAULT(line->offset));
      gvar->setSuffix("%");
      grid.nextLine();

      // Trim
      new StaticText(window, grid.getLabelSlot(), STR_TRIM);
      trimChoice = new Choice(window, grid.getFieldSlot(), STR_VMIXTRIMS, -TRIM_OFF, -TRIM_LAST,
                              GET_VALUE(-line->carryTrim),
                              SET_VALUE(line->carryTrim, -newValue));
      trimChoice->setAvailableHandler([=](int value) {
        return value != TRIM_ON || line->srcRaw <= MIXSRC_Ail;
      });
      grid.nextLine();

      // Curve
      new StaticText(&body, grid.getLabelSlot(), STR_CURVE);
      new Choice(&body, grid.getFieldSlot(2, 0), "\004DiffExpoFuncCstm", 0, CURVE_REF_CUSTOM,
                 GET_DEFAULT(line->curve.type),
                 [=](int32_t newValue) {
                     line->curve.type = newValue;
                     line->curve.value = 0;
                     SET_DIRTY();
                     updateCurveParamField(line);
                 });
      curveParamField = new FormGroup(&body, grid.getFieldSlot(2, 1), FORM_FORWARD_FOCUS);
      updateCurveParamField(line);
      grid.nextLine();

      // Flight modes
      new StaticText(window, grid.getLabelSlot(), STR_FLMODE);
      for (uint8_t i=0; i<MAX_FLIGHT_MODES; i++) {
        char fm[2] = { char('0' + i), '\0'};
        if (i > 0 && (i % 4) == 0)
          grid.nextLine();
        new TextButton(window, grid.getFieldSlot(4, i % 4), fm,
                       [=]() -> uint8_t {
                           BFBIT_FLIP(line->flightModes, bfBit<uint32_t>(i));
                           SET_DIRTY();
                           return !(bfSingleBitGet(line->flightModes, i));
                       },
                       OPAQUE | (bfSingleBitGet(line->flightModes, i) ? 0 : BUTTON_CHECKED));
      }
      grid.nextLine();

      window->setInnerHeight(grid.getWindowHeight());
    }
};

void CommonInputOrMixButton::checkEvents()
{
  if (active != isActive()) {
    invalidate();
    active = !active;
  }

  Button::checkEvents();
}

void CommonInputOrMixButton::drawFlightModes(BitmapBuffer *dc,
                                             FlightModesType value,
                                             LcdFlags textColor)
{
  dc->drawMask(146, 2 + PAGE_LINE_HEIGHT + FIELD_PADDING_TOP,
               mixerSetupFlightmodeIcon, textColor);
  coord_t x = 166;
  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    char s[] = " ";
    s[0] = '0' + i;
    if (value & (1 << i)) {
      dc->drawText(x, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP + 2, s,
                   FONT(XS) | TEXT_DISABLE_COLOR);
    } else {
      dc->drawSolidFilledRect(x, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP + 2, 8, 3,
                              FOCUS_BGCOLOR);
      dc->drawText(x, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP + 2, s, FONT(XS) | textColor);
    }
    x += 8;
  }
}

void CommonInputOrMixButton::paint(BitmapBuffer * dc)
{
  dc->drawSolidFilledRect(0, 0, width(), height(),
                          /*hasFocus() ? FOCUS_BGCOLOR :*/ FIELD_BGCOLOR);
  paintBody(dc);

  if (!hasFocus())
    dc->drawSolidRect(0, 0, rect.w, rect.h, 1, FIELD_FRAME_COLOR);
  else
    dc->drawSolidRect(0, 0, rect.w, rect.h, 2, FOCUS_BGCOLOR);
}

class InputLineButton : public CommonInputOrMixButton
{
 public:
  InputLineButton(FormGroup *parent, const rect_t &rect, uint8_t index) :
      CommonInputOrMixButton(parent, rect, index)
  {
    const ExpoData &line = g_model.expoData[index];
    if (line.swtch || line.curve.value != 0 || line.flightModes) {
      setHeight(height() + PAGE_LINE_HEIGHT + FIELD_PADDING_TOP);
    }
  }

  bool isActive() const override { return isExpoActive(index); }

  void paintBody(BitmapBuffer *dc) override
  {
    const ExpoData &line = g_model.expoData[index];

    LcdFlags textColor = DEFAULT_COLOR;
    // if (hasFocus())
    //   textColor = FOCUS_COLOR;

    dc->drawSolidFilledRect(0, 0, width(), height(),
                            isActive() ? HIGHLIGHT_COLOR : FIELD_BGCOLOR);
    // first line ...
    drawValueOrGVar(dc, FIELD_PADDING_LEFT, FIELD_PADDING_TOP, line.weight,
                    -100, 100, textColor);
    drawSource(dc, 60, FIELD_PADDING_TOP, line.srcRaw, textColor);

    if (line.name[0]) {
      dc->drawMask(146, FIELD_PADDING_TOP, mixerSetupLabelIcon, textColor);
      dc->drawSizedText(166, FIELD_PADDING_TOP, line.name, sizeof(line.name),
                        textColor);
    }

    // second line ...
    if (line.swtch) {
      dc->drawMask(3, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP,
                   mixerSetupSwitchIcon, textColor);
      drawSwitch(dc, 21, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP, line.swtch,
                 textColor);
    }

    if (line.curve.value != 0) {
      dc->drawMask(60, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP,
                   mixerSetupCurveIcon, textColor);
      drawCurveRef(dc, 80, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP, line.curve,
                   textColor);
    }

    if (line.flightModes) {
      drawFlightModes(dc, line.flightModes, textColor);
    }
    
  }
};

ModelInputsPage::ModelInputsPage():
  PageTab(STR_MENUINPUTS, ICON_MODEL_INPUTS)
{
  setOnSetVisibleHandler([]() {
    // reset clipboard
    s_copyMode = 0;
  });
}

void ModelInputsPage::rebuild(FormWindow * window, int8_t focusIndex)
{
  coord_t scrollPosition = window->getScrollPositionY();
  window->clear();
  build(window, focusIndex);
  window->setScrollPositionY(scrollPosition);
}

void ModelInputsPage::editInput(FormWindow * window, uint8_t input, uint8_t index)
{
  Window::clearFocus();
  Window * editWindow = new InputEditWindow(input, index);
  editWindow->setCloseHandler([=]() {
    rebuild(window, index);
  });
}

void ModelInputsPage::build(FormWindow *window, int8_t focusIndex)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);
  grid.setLabelWidth(66);

  int inputIndex = 0;
  ExpoData *line = g_model.expoData;
  for (uint8_t input = 0; input < MAX_INPUTS; input++) {
    if (inputIndex < MAX_EXPOS && line->chn == input && EXPO_VALID(line)) {
      coord_t h = grid.getWindowHeight();
      auto txt = new StaticText(window, grid.getLabelSlot(),
                                getSourceString(MIXSRC_FIRST_INPUT + input),
                                BUTTON_BACKGROUND, CENTERED);

      while (inputIndex < MAX_EXPOS && line->chn == input && EXPO_VALID(line)) {
        Button *button =
            new InputLineButton(window, grid.getFieldSlot(), inputIndex);
        button->setPressHandler([=]() -> uint8_t {
          button->bringToTop();
          Menu *menu = new Menu(window);
          menu->addLine(STR_EDIT,
                        [=]() { editInput(window, input, inputIndex); });
          if (!reachExposLimit()) {
            menu->addLine(STR_INSERT_BEFORE, [=]() {
              insertExpo(inputIndex, input);
              editInput(window, input, inputIndex);
            });
            menu->addLine(STR_INSERT_AFTER, [=]() {
              insertExpo(inputIndex + 1, input);
              editInput(window, input, inputIndex + 1);
            });
            menu->addLine(STR_COPY, [=]() {
              s_copyMode = COPY_MODE;
              s_copySrcIdx = inputIndex;
            });
            if (s_copyMode != 0) {
              menu->addLine(STR_PASTE_BEFORE, [=]() {
                copyExpo(s_copySrcIdx, inputIndex, PASTE_BEFORE);
                if (s_copyMode == MOVE_MODE) {
                  deleteExpo((s_copySrcIdx > inputIndex) ? s_copySrcIdx + 1
                                                         : s_copySrcIdx);
                  s_copyMode = 0;
                }
                rebuild(window, inputIndex);
              });
              menu->addLine(STR_PASTE_AFTER, [=]() {
                copyExpo(s_copySrcIdx, inputIndex, PASTE_AFTER);
                if (s_copyMode == MOVE_MODE) {
                  deleteExpo((s_copySrcIdx > inputIndex) ? s_copySrcIdx + 1
                                                         : s_copySrcIdx);
                  s_copyMode = 0;
                }
                rebuild(window, inputIndex + 1);
              });
            }
          }
          menu->addLine(STR_MOVE, [=]() {
            s_copyMode = MOVE_MODE;
            s_copySrcIdx = inputIndex;
          });
          menu->addLine(STR_DELETE, [=]() {
            deleteExpo(inputIndex);
            rebuild(window, -1);
          });
          return 0;
        });
        button->setFocusHandler([=](bool focus) {
          if (focus) {
            txt->setBackgroundColor(FOCUS_BGCOLOR);
            txt->setTextFlags(FOCUS_COLOR | CENTERED);
          } else {
            txt->setBackgroundColor(FIELD_FRAME_COLOR);
            txt->setTextFlags(CENTERED);
          }
          txt->invalidate();
          if (focus) button->bringToTop();
        });

        if (focusIndex == inputIndex) {
          button->setFocus(SET_FOCUS_DEFAULT);
          txt->setBackgroundColor(FOCUS_BGCOLOR);
          txt->setTextFlags(FOCUS_COLOR | CENTERED);
          txt->invalidate();
        }
        grid.spacer(button->height() - 1);
        ++inputIndex;
        ++line;
      }

      h = grid.getWindowHeight() - h + 1;
      txt->setHeight(h);

      grid.spacer(7);
    } else {
      auto button = new TextButton(window, grid.getLabelSlot(),
                                   getSourceString(MIXSRC_FIRST_INPUT + input));
      if (focusIndex == inputIndex) button->setFocus(SET_FOCUS_DEFAULT);
      button->setPressHandler([=]() -> uint8_t {
        button->bringToTop();
        Menu *menu = new Menu(window);
        menu->addLine(STR_EDIT, [=]() {
          insertExpo(inputIndex, input);
          editInput(window, input, inputIndex);
          return 0;
        });
        if (!reachExposLimit()) {
          if (s_copyMode != 0) {
            menu->addLine(STR_PASTE, [=]() {
              copyExpo(s_copySrcIdx, inputIndex, input);
              if (s_copyMode == MOVE_MODE) {
                deleteExpo((s_copySrcIdx >= inputIndex) ? s_copySrcIdx + 1
                                                        : s_copySrcIdx);
                s_copyMode = 0;
              }
              rebuild(window, -1);
              return 0;
            });
          }
        }
        return 0;
      });

      grid.spacer(button->height() + 5);
    }
  }

  Window *focus = Window::getFocus();
  if (focus) {
    focus->bringToTop();
  }

  grid.nextLine();

  window->setInnerHeight(grid.getWindowHeight());
}

// TODO port: avoid global s_currCh on ARM boards (as done here)...
int8_t s_currCh;
uint8_t s_copyMode;
int8_t s_copySrcRow;

void insertExpo(uint8_t idx, uint8_t input)
{
  pauseMixerCalculations();
  ExpoData * expo = expoAddress(idx);
  memmove(expo+1, expo, (MAX_EXPOS-(idx+1))*sizeof(ExpoData));
  memclear(expo, sizeof(ExpoData));
  expo->srcRaw = (input >= 4 ? MIXSRC_Rud + input : MIXSRC_Rud + channelOrder(input + 1) - 1);
  expo->curve.type = CURVE_REF_EXPO;
  expo->mode = 3; // pos+neg
  expo->chn = input;
  expo->weight = 100;
  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}
