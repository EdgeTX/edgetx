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

#include "model_mixes.h"
#include "opentx.h"
#include "libopenui.h"
#include "choiceex.h"
#include "bitfield.h"
#include "model_inputs.h"
#include "gvar_numberedit.h"
#include "model_curves.h"
#include "dataconstants.h"
#include "channel_bar.h"


#define SET_DIRTY()     storageDirty(EE_MODEL)
#define PASTE_BEFORE    -2
#define PASTE_AFTER     -1

uint8_t getMixesCount()
{
  uint8_t count = 0;
  uint8_t ch;

  for (int i = MAX_MIXERS - 1; i >= 0; i--) {
    ch = mixAddress(i)->srcRaw;
    if (ch != 0) {
      count++;
    }
  }
  return count;
}

bool reachMixesLimit()
{
  if (getMixesCount() >= MAX_MIXERS) {
    POPUP_WARNING(STR_NOFREEMIXER);
    return true;
  }
  return false;
}


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

class MixEditWindow : public Page
{
 public:
  MixEditWindow(int8_t channel, uint8_t mixIndex) :
      Page(ICON_MODEL_MIXER), channel(channel), mixIndex(mixIndex)
  {
    buildBody(&body);
    buildHeader(&header);
  }

 protected:
  uint8_t channel;
  uint8_t mixIndex;
  FormGroup *curveParamField = nullptr;
  MixerEditStatusBar *statusBar = nullptr;

  void buildHeader(Window *window)
  {
    new StaticText(window,
                   {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                    PAGE_LINE_HEIGHT},
                   STR_MIXES, 0, COLOR_THEME_PRIMARY2);
    new StaticText(window,
                   {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                    LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                   getSourceString(MIXSRC_CH1 + channel), 0, COLOR_THEME_PRIMARY2);

    statusBar = new MixerEditStatusBar(
        window, {window->getRect().w - MIX_STATUS_BAR_WIDTH - MIX_RIGHT_MARGIN, 0, MIX_STATUS_BAR_WIDTH, MENU_HEADER_HEIGHT + 3},
        channel);
  }

  void buildBody(FormWindow *window)
  {
    FormGridLayout grid;
    grid.spacer(8);

    MixData *mix = mixAddress(mixIndex);

    // Mix name
    new StaticText(window, grid.getLabelSlot(), STR_MIXNAME, 0, COLOR_THEME_PRIMARY1);
    new ModelTextEdit(window, grid.getFieldSlot(), mix->name,
                      sizeof(mix->name));
    grid.nextLine();

    // Source
    new StaticText(window, grid.getLabelSlot(), STR_SOURCE, 0, COLOR_THEME_PRIMARY1);
    new SourceChoice(window, grid.getFieldSlot(), 0, MIXSRC_LAST,
                     GET_SET_DEFAULT(mix->srcRaw));
    grid.nextLine();

    // Weight
    new StaticText(window, grid.getLabelSlot(), STR_WEIGHT, 0, COLOR_THEME_PRIMARY1);
    auto gvar =
        new GVarNumberEdit(window, grid.getFieldSlot(), MIX_WEIGHT_MIN,
                           MIX_WEIGHT_MAX, GET_SET_DEFAULT(mix->weight));
    gvar->setSuffix("%");
    grid.nextLine();

    // Offset
    new StaticText(window, grid.getLabelSlot(), STR_OFFSET, 0, COLOR_THEME_PRIMARY1);
    gvar = new GVarNumberEdit(window, grid.getFieldSlot(), MIX_OFFSET_MIN,
                              MIX_OFFSET_MAX, GET_SET_DEFAULT(mix->offset));
    gvar->setSuffix("%");
    grid.nextLine();

    // Trim
    new StaticText(window, grid.getLabelSlot(), STR_TRIM, 0, COLOR_THEME_PRIMARY1);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_INVERTED(mix->carryTrim));
    grid.nextLine();

    // Curve
    new StaticText(&body, grid.getLabelSlot(), STR_CURVE, 0, COLOR_THEME_PRIMARY1);
    new Choice(&body, grid.getFieldSlot(2, 0), "\004DiffExpoFuncCstm", 0,
               CURVE_REF_CUSTOM, GET_DEFAULT(mix->curve.type),
               [=](int32_t newValue) {
                 mix->curve.type = newValue;
                 mix->curve.value = 0;
                 SET_DIRTY();
                 updateCurveParamField(mix);
               });
    curveParamField =
        new FormGroup(&body, grid.getFieldSlot(2, 1), FORM_FORWARD_FOCUS);
    updateCurveParamField(mix);
    grid.nextLine();

#if defined(FLIGHT_MODES)
    // Flight modes
    new StaticText(window, grid.getLabelSlot(), STR_FLMODE, 0, COLOR_THEME_PRIMARY1);
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
    new StaticText(window, grid.getLabelSlot(), STR_SWITCH, 0, COLOR_THEME_PRIMARY1);
    new SwitchChoice(window, grid.getFieldSlot(), SWSRC_FIRST_IN_MIXES,
                     SWSRC_LAST_IN_MIXES, GET_SET_DEFAULT(mix->swtch));
    grid.nextLine();

    // Warning
    new StaticText(window, grid.getLabelSlot(), STR_MIXWARNING, 0, COLOR_THEME_PRIMARY1);
    auto edit = new NumberEdit(window, grid.getFieldSlot(2, 0), 0, 3,
                               GET_SET_DEFAULT(mix->mixWarn));
    edit->setZeroText(STR_OFF);
    grid.nextLine();

    // Multiplex
    new StaticText(window, grid.getLabelSlot(), STR_MULTPX, 0, COLOR_THEME_PRIMARY1);
    new Choice(window, grid.getFieldSlot(), STR_VMLTPX, 0, 2,
               GET_SET_DEFAULT(mix->mltpx));
    grid.nextLine();

    // Delay up
    new StaticText(window, grid.getLabelSlot(), STR_DELAYUP, 0, COLOR_THEME_PRIMARY1);
    edit = new NumberEdit(window, grid.getFieldSlot(2, 0), 0, DELAY_MAX,
                          GET_DEFAULT(mix->delayUp),
                          SET_VALUE(mix->delayUp, newValue), 0, PREC1);
    edit->setSuffix("s");
    grid.nextLine();

    // Delay down
    new StaticText(window, grid.getLabelSlot(), STR_DELAYDOWN, 0, COLOR_THEME_PRIMARY1);
    edit = new NumberEdit(window, grid.getFieldSlot(2, 0), 0, DELAY_MAX,
                          GET_DEFAULT(mix->delayDown),
                          SET_VALUE(mix->delayDown, newValue), 0, PREC1);
    edit->setSuffix("s");
    grid.nextLine();

    // Slow up
    new StaticText(window, grid.getLabelSlot(), STR_SLOWUP, 0, COLOR_THEME_PRIMARY1);
    edit = new NumberEdit(window, grid.getFieldSlot(2, 0), 0, DELAY_MAX,
                          GET_DEFAULT(mix->speedUp),
                          SET_VALUE(mix->speedUp, newValue), 0, PREC1);
    edit->setSuffix("s");
    grid.nextLine();

    // Slow down
    new StaticText(window, grid.getLabelSlot(), STR_SLOWDOWN, 0, COLOR_THEME_PRIMARY1);
    edit = new NumberEdit(window, grid.getFieldSlot(2, 0), 0, DELAY_MAX,
                          GET_DEFAULT(mix->speedDown),
                          SET_VALUE(mix->speedDown, newValue), 0, PREC1);
    edit->setSuffix("s");
    grid.nextLine();

    window->setInnerHeight(grid.getWindowHeight());
  }

  // TODO share this code with INPUT
  void updateCurveParamField(MixData *line)
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
};

class MixLineButton : public CommonInputOrMixButton
{
 public:
  MixLineButton(FormGroup *parent, const rect_t &rect, uint8_t index) :
      CommonInputOrMixButton(parent, rect, index)
  {
    const MixData &mix = g_model.mixData[index];
    if (mix.swtch || mix.curve.value != 0 || mix.flightModes) {
      setHeight(height() + PAGE_LINE_HEIGHT + FIELD_PADDING_TOP);
    }
  }

  bool isActive() const override { return isMixActive(index); }

  void paintBody(BitmapBuffer *dc) override
  {
    const MixData &line = g_model.mixData[index];

    LcdFlags textColor = COLOR_THEME_SECONDARY1;

    // first line ...
    drawValueOrGVar(dc, FIELD_PADDING_LEFT, FIELD_PADDING_TOP, line.weight,
                    MIX_WEIGHT_MIN, MIX_WEIGHT_MAX, textColor);
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

    if (line.curve.value) {
      dc->drawMask(60, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP,
                   mixerSetupCurveIcon, textColor);
      drawCurveRef(dc, 80, PAGE_LINE_HEIGHT + FIELD_PADDING_TOP, line.curve,
                   textColor);
    }

    if (line.flightModes) {
      drawFlightModes(dc, line.flightModes, textColor);
    }

    // Put this icon on the first line, since we have more space there
    uint8_t delayslow = 0;
    if (line.speedDown || line.speedUp) delayslow = 1;
    if (line.delayUp || line.delayDown) delayslow += 2;
    if (delayslow) {
      BitmapBuffer *delayslowbmp[] = {mixerSetupSlowIcon, mixerSetupDelayIcon, mixerSetupDelaySlowIcon};
      dc->drawMask(width() - 16, FIELD_PADDING_TOP, delayslowbmp[delayslow - 1], textColor);
    }
  }
};

void insertMix(uint8_t idx, uint8_t channel)
{
  pauseMixerCalculations();
  MixData *mix = mixAddress(idx);
  memmove(mix + 1, mix, (MAX_MIXERS - (idx + 1)) * sizeof(MixData));
  memclear(mix, sizeof(MixData));
  mix->destCh = channel;
  mix->srcRaw = channel + 1;
  if (!isSourceAvailable(mix->srcRaw)) {
    mix->srcRaw = (channel > 3 ? MIXSRC_Rud - 1 + channel
                               : MIXSRC_Rud - 1 + channelOrder(channel));
    while (!isSourceAvailable(mix->srcRaw)) {
      mix->srcRaw += 1;
    }
  }
  mix->weight = 100;
  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}

ModelMixesPage::ModelMixesPage() :
  PageTab(STR_MIXES, ICON_MODEL_MIXER)
{
  setOnSetVisibleHandler([]() {
    // reset clipboard
    s_copyMode = 0;
  });
}

void ModelMixesPage::rebuild(FormWindow * window, int8_t focusMixIndex)
{
  coord_t scrollPosition = window->getScrollPositionY();
  window->clear();
  build(window, focusMixIndex);
  window->setScrollPositionY(scrollPosition);
}

void ModelMixesPage::editMix(FormWindow * window, uint8_t channel, uint8_t mixIndex)
{
  Window::clearFocus();
  Window *editWindow = new MixEditWindow(channel, mixIndex);
  editWindow->setCloseHandler([=]() {
    int8_t newIndex = mixIndex;
    if (is_memclear(&g_model.mixData[mixIndex], sizeof(MixData))) {
      deleteMix(mixIndex);
      newIndex--;
    }
    rebuild(window, newIndex);
  });
}

class MixLineTitle : public StaticText
{
 public:
  using StaticText::StaticText;

  void setTextFlags(LcdFlags flags)
  {
    textFlags = flags;
    for (auto child : children) {
      auto bitmap = dynamic_cast<StaticBitmap*>(child);
      if (bitmap) bitmap->setMaskColor(flags & 0xFFFF0000);
    }
  }
};

void ModelMixesPage::build(FormWindow * window, int8_t focusMixIndex)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);
  grid.setLabelWidth(66);

  const BitmapBuffer * const mixerMultiplexBitmap[] = {
    mixerSetupAddBitmap,
    mixerSetupMultiBitmap,
    mixerSetupReplaceBitmap
  };

  int mixIndex = 0;
  MixData * mix = g_model.mixData;
  for (uint8_t ch = 0; ch < MAX_OUTPUT_CHANNELS; ch++) {

    bool skip_mix = (ch == 0 && is_memclear(mix, sizeof(MixData)));
    
    if (mixIndex < MAX_MIXERS && mix->destCh == ch && !skip_mix) {

      coord_t h = grid.getWindowHeight();
      auto txt = new MixLineTitle(window, grid.getLabelSlot(),
                                  getSourceString(MIXSRC_CH1 + ch),
                                  BUTTON_BACKGROUND, COLOR_THEME_PRIMARY1 | CENTERED);

      uint8_t count = 0;
      while (mixIndex < MAX_MIXERS && mix->destCh == ch && !skip_mix) {

        Button * button = new MixLineButton(window, grid.getFieldSlot(), mixIndex);
        button->setPressHandler([=]() -> uint8_t {
          button->bringToTop();
          Menu * menu = new Menu(window);
          menu->addLine(STR_EDIT, [=]() {
            editMix(window, ch, mixIndex);
          });
          if (!reachMixesLimit()) {
            menu->addLine(STR_INSERT_BEFORE, [=]() {
              insertMix(mixIndex, ch);
              editMix(window, ch, mixIndex);
            });
            menu->addLine(STR_INSERT_AFTER, [=]() {
              insertMix(mixIndex + 1, ch);
              editMix(window, ch, mixIndex + 1);
            });
            menu->addLine(STR_COPY, [=]() {
              s_copyMode = COPY_MODE;
              s_copySrcIdx =mixIndex;
            });
            if (s_copyMode != 0) {
              menu->addLine(STR_PASTE_BEFORE, [=]() {
                copyMix(s_copySrcIdx, mixIndex, PASTE_BEFORE);
                if(s_copyMode == MOVE_MODE) {
                  deleteMix((s_copySrcIdx > mixIndex) ? s_copySrcIdx+1 : s_copySrcIdx);
                  s_copyMode = 0;
                }
                rebuild(window, mixIndex);
              });
              menu->addLine(STR_PASTE_AFTER, [=]() {
                copyMix(s_copySrcIdx, mixIndex, PASTE_AFTER);
                if(s_copyMode == MOVE_MODE) {
                  deleteMix((s_copySrcIdx > mixIndex) ? s_copySrcIdx+1 : s_copySrcIdx);
                  s_copyMode = 0;
                }
                rebuild(window, mixIndex+1);
              });
            }
          }
          menu->addLine(STR_MOVE, [=]() {
            s_copyMode = MOVE_MODE;
            s_copySrcIdx = mixIndex;
          });
          menu->addLine(STR_DELETE, [=]() {
            deleteMix(mixIndex);
            rebuild(window, -1);
          });
          return 0;
        });

        if (count++ > 0) {
          new StaticBitmap(
              txt,
              {35 - txt->left(),
               button->top() - txt->top() + (button->height() - 18) / 2, 25,
               17},
              mixerMultiplexBitmap[mix->mltpx], COLOR_THEME_SECONDARY1);
        }

        button->setFocusHandler([=](bool focus) {
          if (focus) {
            txt->setBackgroundColor(COLOR_THEME_FOCUS);
            txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED);
          } else {
            txt->setBackgroundColor(COLOR_THEME_SECONDARY2);
            txt->setTextFlags(COLOR_THEME_PRIMARY1 | CENTERED);
          }
          txt->invalidate();
          if (focus) button->bringToTop();
        });

        if (focusMixIndex == mixIndex) {
          button->setFocus(SET_FOCUS_DEFAULT);
          txt->setBackgroundColor(COLOR_THEME_FOCUS);
          txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED);
          txt->invalidate();
        }


        grid.spacer(button->height() - 1);
        ++mixIndex;
        ++mix;

        skip_mix = (ch == 0 && is_memclear(mix, sizeof(MixData)));
      }

      h = grid.getWindowHeight() - h + 1;
      txt->setHeight(h);

      grid.spacer(7);
    }
    else {
      auto button = new TextButton(window, grid.getLabelSlot(),
                                   getSourceString(MIXSRC_CH1 + ch));
      if (focusMixIndex == mixIndex) button->setFocus(SET_FOCUS_DEFAULT);
      button->setPressHandler([=]() -> uint8_t {
        button->bringToTop();
        Menu *menu = new Menu(window);
        menu->addLine(STR_EDIT, [=]() {
          insertMix(mixIndex, ch);
          editMix(window, ch, mixIndex);
          return 0;
        });
        if (!reachMixesLimit()) {
          if (s_copyMode != 0) {
            menu->addLine(STR_PASTE, [=]() {
              copyMix(s_copySrcIdx, mixIndex, ch);
              if (s_copyMode == MOVE_MODE) {
                deleteMix((s_copySrcIdx >= mixIndex) ? s_copySrcIdx + 1
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

  Window * focus = Window::getFocus();
  if (focus) {
    focus->bringToTop();
  }

  grid.nextLine();

  window->setInnerHeight(grid.getWindowHeight());
}

void deleteMix(uint8_t idx)
{
  pauseMixerCalculations();
  MixData * mix = mixAddress(idx);
  memmove(mix, mix + 1, (MAX_MIXERS - (idx + 1)) * sizeof(MixData));
  memclear(&g_model.mixData[MAX_MIXERS - 1], sizeof(MixData));
  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}

#if defined(LUA)
// This is necessary as the LUA API uses th old interface
// where insertMix() has only one param. The channel is
// passed through s_currCh
void insertMix(uint8_t idx)
{
  insertMix(idx, s_currCh - 1);
}
#endif

void copyMix(uint8_t source, uint8_t dest, int8_t ch)
{
  pauseMixerCalculations();
  MixData sourceMix;
  memcpy(&sourceMix, mixAddress(source), sizeof(MixData));
  MixData *mix = mixAddress(dest);
  size_t trailingMixes = MAX_MIXERS - (dest + 1);
  if (ch == PASTE_AFTER) {
    trailingMixes--;
    memmove(mix + 2, mix + 1, trailingMixes * sizeof(MixData));
    memcpy(mix + 1, &sourceMix, sizeof(MixData));
    (mix + 1)->destCh = (mix)->destCh;
  } else if (ch == PASTE_BEFORE) {
    memmove(mix + 1, mix, trailingMixes * sizeof(MixData));
    memcpy(mix, &sourceMix, sizeof(MixData));
    mix->destCh = (mix + 1)->destCh;
  } else {
    memmove(mix + 1, mix, trailingMixes * sizeof(MixData));
    memcpy(mix, &sourceMix, sizeof(MixData));
    mix->destCh = ch;
  }
  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}

bool swapMixes(uint8_t &idx, uint8_t up)
{
  MixData * x, * y;
  int8_t tgt_idx = (up ? idx - 1 : idx + 1);

  x = mixAddress(idx);

  if (tgt_idx < 0) {
    if (x->destCh == 0)
      return false;
    x->destCh--;
    return true;
  }

  if (tgt_idx == MAX_MIXERS) {
    if (x->destCh == MAX_OUTPUT_CHANNELS - 1)
      return false;
    x->destCh++;
    return true;
  }

  y = mixAddress(tgt_idx);
  uint8_t destCh = x->destCh;
  if (!y->srcRaw || destCh != y->destCh) {
    if (up) {
      if (destCh > 0) x->destCh--;
      else return false;
    }
    else {
      if (destCh < MAX_OUTPUT_CHANNELS - 1) x->destCh++;
      else return false;
    }
    return true;
  }

  pauseMixerCalculations();
  memswap(x, y, sizeof(MixData));
  resumeMixerCalculations();

  idx = tgt_idx;
  return true;
}
