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
#include "etx_lv_theme.h"
#include "fm_matrix.h"
#include "getset_helpers.h"
#include "gvar_numberedit.h"
#include "input_source.h"
#include "source_numberedit.h"
#include "switchchoice.h"
#include "textedit.h"

// Defined in mixer.cpp
extern mixsrc_t sourceRefToMixSrc(const SourceRef& ref);
extern swsrc_t switchRefToSwSrc(const SwitchRef& ref);

// Defined in curves.cpp
extern gvar_t valueOrSourceToLegacy(const ValueOrSource& vos);

// Convert legacy swsrc_t back to SwitchRef
static SwitchRef swSrcToSwitchRef(swsrc_t src)
{
  SwitchRef ref = {};
  if (src == SWSRC_NONE) return ref;

  bool inverted = (src < 0);
  swsrc_t absSrc = inverted ? -src : src;

  struct Range { swsrc_t first; swsrc_t last; uint8_t type; };
  static const Range ranges[] = {
    {SWSRC_FIRST_SWITCH, SWSRC_LAST_SWITCH, SWITCH_TYPE_SWITCH},
    {SWSRC_FIRST_MULTIPOS_SWITCH, SWSRC_LAST_MULTIPOS_SWITCH, SWITCH_TYPE_MULTIPOS},
    {SWSRC_FIRST_TRIM, SWSRC_LAST_TRIM, SWITCH_TYPE_TRIM},
    {SWSRC_FIRST_LOGICAL_SWITCH, SWSRC_LAST_LOGICAL_SWITCH, SWITCH_TYPE_LOGICAL},
    {SWSRC_FIRST_FLIGHT_MODE, SWSRC_LAST_FLIGHT_MODE, SWITCH_TYPE_FLIGHT_MODE},
    {SWSRC_FIRST_SENSOR, SWSRC_LAST_SENSOR, SWITCH_TYPE_SENSOR},
  };

  if (absSrc == SWSRC_ON) { ref.type = SWITCH_TYPE_ON; }
  else if (absSrc == SWSRC_ONE) { ref.type = SWITCH_TYPE_ONE; }
  else if (absSrc == SWSRC_TELEMETRY_STREAMING) { ref.type = SWITCH_TYPE_TELEMETRY; }
  else if (absSrc == SWSRC_RADIO_ACTIVITY) { ref.type = SWITCH_TYPE_RADIO_ACTIVITY; }
  else if (absSrc == SWSRC_TRAINER_CONNECTED) { ref.type = SWITCH_TYPE_TRAINER; }
  else {
    for (const auto& r : ranges) {
      if (absSrc >= r.first && absSrc <= r.last) {
        ref.type = r.type;
        ref.index = absSrc - r.first;
        break;
      }
    }
  }

  if (inverted) ref.flags = SWITCH_FLAG_INVERTED;
  return ref;
}

// Convert legacy SourceNumVal rawValue to ValueOrSource
static ValueOrSource legacyToValueOrSource(int32_t rawValue)
{
  ValueOrSource vos = {};
  SourceNumVal v;
  v.rawValue = rawValue;
  if (v.isSource) {
    vos.isSource = 1;
    mixsrc_t src = v.value;
    if (src >= MIXSRC_FIRST_GVAR && src <= MIXSRC_LAST_GVAR) {
      vos.srcType = SOURCE_TYPE_GVAR;
      vos.value = src - MIXSRC_FIRST_GVAR;
    } else if (src >= MIXSRC_FIRST_INPUT && src <= MIXSRC_LAST_INPUT) {
      vos.srcType = SOURCE_TYPE_INPUT;
      vos.value = src - MIXSRC_FIRST_INPUT;
    } else if (src >= MIXSRC_FIRST_STICK && src <= MIXSRC_LAST_STICK) {
      vos.srcType = SOURCE_TYPE_STICK;
      vos.value = src - MIXSRC_FIRST_STICK;
    } else if (src >= MIXSRC_FIRST_CH && src <= MIXSRC_LAST_CH) {
      vos.srcType = SOURCE_TYPE_CHANNEL;
      vos.value = src - MIXSRC_FIRST_CH;
    } else {
      vos.srcType = 0;
      vos.value = src;
    }
  } else {
    vos.setNumeric(v.value);
  }
  return vos;
}

#define SET_DIRTY() storageDirty(EE_MODEL)

#if LANDSCAPE
static const lv_coord_t col_dsc[] = {LV_GRID_FR(3), LV_GRID_FR(8),
                                     LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
#endif
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class InputEditAdvanced : public Page
{
 public:
  InputEditAdvanced(uint8_t input_n, uint8_t index) : Page(ICON_MODEL_INPUTS)
  {
    std::string title2(getSourceString(MIXSRC_FIRST_INPUT + input_n));
    header->setTitle(STR_MENUINPUTS);
    header->setTitle2(title2);

    FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);
    body->setFlexLayout();

    ExpoData* input = expoAddress(index);

    // Side
    auto line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_SIDE);
    new Choice(
        line, rect_t{}, STR_VCURVEFUNC, 1, 3,
        [=]() -> int16_t { return 4 - input->mode; },
        [=](int16_t newValue) {
          input->mode = 4 - newValue;
          Messaging::send(Messaging::CURVE_UPDATE);
          SET_DIRTY();
        });

    // Trim
    line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_TRIM);
    const auto trimLast = TRIM_OFF + keysGetMaxTrims() - 1;
    auto c = new Choice(line, rect_t{}, -TRIM_OFF, trimLast,
                        GET_VALUE(-input->trimSource),
                        SET_VALUE(input->trimSource, -newValue));

    mixsrc_t srcRaw = sourceRefToMixSrc(input->srcRaw);
    c->setAvailableHandler([=](int value) {
      return value != TRIM_ON || srcRaw <= MIXSRC_LAST_STICK;
    });
    c->setTextHandler([=](int value) -> std::string {
      return getTrimSourceLabel(srcRaw, -value);
    });

    // Flight modes
    if (modelFMEnabled()) {
      line = body->newLine(grid);
      new StaticText(line, rect_t{}, STR_FLMODE);
      new FMMatrix<ExpoData>(line, rect_t{}, input);
    }
  }
};

InputEditWindow::InputEditWindow(int8_t input, uint8_t index) :
    Page(ICON_MODEL_INPUTS), input(input), index(index)
{
  header->setTitle(STR_MENUINPUTS);
  headerSwitchName = header->setTitle2("");

  etx_txt_color(headerSwitchName->getLvObj(), COLOR_THEME_ACTIVE_INDEX,
                LV_STATE_USER_1);
  etx_font(headerSwitchName->getLvObj(), FONT_BOLD_INDEX, LV_STATE_USER_1);

  setTitle();

#if PORTRAIT
  body->padAll(PAD_ZERO);

  auto box = new Window(body, rect_t{0, 0, body->width(), body->height() - INPUT_EDIT_CURVE_HEIGHT - PAD_TINY * 2});
  auto box_obj = box->getLvObj();
  etx_scrollbar(box_obj);
  box->padAll(PAD_SMALL);

  auto form = new Window(box, rect_t{});
  buildBody(form);

  preview = new Curve(
      body, rect_t{(LCD_W - INPUT_EDIT_CURVE_WIDTH) / 2, body->height() - INPUT_EDIT_CURVE_HEIGHT - PAD_TINY, INPUT_EDIT_CURVE_WIDTH, INPUT_EDIT_CURVE_HEIGHT},
      [=](int x) -> int {
        ExpoData* line = expoAddress(index);
        int16_t anas[MAX_INPUTS] = {0};
        applyExpos(anas, e_perout_mode_inactive_flight_mode, sourceRefToMixSrc(line->srcRaw), x);
        return anas[line->chn];
      },
      [=]() -> int { return getValue(sourceRefToMixSrc(expoAddress(index)->srcRaw)); });
#else
  body->padAll(PAD_SMALL);
  buildBody(body);

  preview = new Curve(
      this, rect_t{LCD_W - INPUT_EDIT_CURVE_WIDTH - PAD_LARGE, EdgeTxStyles::MENU_HEADER_HEIGHT + PAD_TINY, INPUT_EDIT_CURVE_WIDTH, INPUT_EDIT_CURVE_HEIGHT},
      [=](int x) -> int {
        ExpoData* line = expoAddress(index);
        int16_t anas[MAX_INPUTS] = {0};
        applyExpos(anas, e_perout_mode_inactive_flight_mode, sourceRefToMixSrc(line->srcRaw), x);
        return anas[line->chn];
      },
      [=]() -> int { return getValue(sourceRefToMixSrc(expoAddress(index)->srcRaw)); });
#endif
}

void InputEditWindow::setTitle()
{
  headerSwitchName->setText(getSourceString(MIXSRC_FIRST_INPUT + input));
}

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
      new SourceNumberEdit(line, -100, 100,
                           [=]() -> int32_t { return valueOrSourceToLegacy(input->weight); },
                           [=](int32_t newValue) {
                             input->weight = legacyToValueOrSource(newValue);
                             updatePreview = true;
                             SET_DIRTY();
                           }, MIXSRC_FIRST);
  gvar->setSuffix("%");

  // Offset
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_OFFSET);
  gvar = new SourceNumberEdit(line, -100, 100,
                              [=]() -> int32_t { return valueOrSourceToLegacy(input->offset); },
                              [=](int32_t newValue) {
                                input->offset = legacyToValueOrSource(newValue);
                                updatePreview = true;
                                SET_DIRTY();
                              }, MIXSRC_FIRST);
  gvar->setSuffix("%");

  // Switch
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_SWITCH);
  new SwitchChoice(line, rect_t{}, SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES,
                   [=]() -> int16_t { return (int16_t)switchRefToSwSrc(input->swtch); },
                   [=](int16_t newValue) {
                     input->swtch = swSrcToSwitchRef((swsrc_t)newValue);
                     updatePreview = true;
                     SET_DIRTY();
                   });

  // Curve
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_CURVE);
  mixsrc_t srcForCurve = sourceRefToMixSrc(input->srcRaw);
  auto param =
      new CurveParam(line, rect_t{}, &input->curve,
        [=](int32_t newValue) {
          input->curve.value = legacyToValueOrSource(newValue);
          updatePreview = true;
          SET_DIRTY();
        }, MIXSRC_FIRST, srcForCurve);
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
  if (_deleted) return;

  ExpoData* input = expoAddress(index);

  getvalue_t val;

  if (input->weight.isSource) {
    val = getValue(sourceRefToMixSrc(input->weight.toSourceRef()));
    if (val != lastWeightVal) {
      lastWeightVal = val;
      updatePreview = true;
    }
  }

  if (input->offset.isSource) {
    val = getValue(sourceRefToMixSrc(input->offset.toSourceRef()));
    if (val != lastOffsetVal) {
      lastOffsetVal = val;
      updatePreview = true;
    }
  }

  if (input->curve.value.isSource) {
    val = getValue(sourceRefToMixSrc(input->curve.value.toSourceRef()));
    if (val != lastCurveVal) {
      lastCurveVal = val;
      updatePreview = true;
    }
  }

  uint8_t activeIdx = 255;
  for (int i = 0; i < MAX_EXPOS; i += 1) {
    auto inp = expoAddress(i);
    if (inp->chn == input->chn) {
      if (getSwitch(switchRefToSwSrc(inp->swtch))) {
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
    Messaging::send(Messaging::CURVE_UPDATE);
  }

  Page::checkEvents();
}
