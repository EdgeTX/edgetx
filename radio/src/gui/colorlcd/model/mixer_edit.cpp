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
#include "curve_param.h"
#include "curveedit.h"
#include "edgetx.h"
#include "etx_lv_theme.h"
#include "getset_helpers.h"
#include "gvar_numberedit.h"
#include "mixer_edit_adv.h"
#include "mixes.h"
#include "pagegroup.h"
#include "source_numberedit.h"
#include "sourcechoice.h"
#include "switchchoice.h"
#include "textedit.h"

// Defined in curves.cpp
extern gvar_t valueOrSourceToLegacy(const ValueOrSource& vos);

// Convert legacy SourceNumVal rawValue to ValueOrSource
static ValueOrSource legacyToValueOrSource(int32_t rawValue)
{
  ValueOrSource vos = {};
  SourceNumVal v;
  v.rawValue = rawValue;
  if (v.isSource) {
    vos.isSource = 1;
    // The legacy value in SourceNumVal.value is a mixsrc_t
    // We need to extract the type and index
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

class MixerEditStatusBar : public Window
{
 public:
  MixerEditStatusBar(Window *parent, const rect_t &rect, int8_t channel) :
      Window(parent, rect), _channel(channel)
  {
    channelBar =
        new ComboChannelBar(this,
                            {MIX_STATUS_BAR_MARGIN, 0,
                             rect.w - (MIX_STATUS_BAR_MARGIN * 2), rect.h},
                            channel, true);
  }

  static LAYOUT_SIZE_SCALED(MIX_STATUS_BAR_MARGIN, 3, 0)

 protected:
  ComboChannelBar *channelBar;
  int8_t _channel;
};

MixEditWindow::MixEditWindow(int8_t channel, uint8_t index) :
    Page(ICON_MODEL_MIXER, PAD_MEDIUM), channel(channel), index(index)
{
  buildBody(body);
  buildHeader(header);
}

void MixEditWindow::buildHeader(Window *window)
{
  std::string title2(getSourceString(MIXSRC_FIRST_CH + channel));
  header->setTitle(STR_MIXES);
  header->setTitle2(title2);

  new MixerEditStatusBar(
      window,
      {window->getRect().w - MIX_STATUS_BAR_WIDTH - PageGroup::PAGE_GROUP_BACK_BTN_W, 0,
       MIX_STATUS_BAR_WIDTH, EdgeTxStyles::MENU_HEADER_HEIGHT},
      channel);
}

#if !NARROW_LAYOUT
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_FR(1), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
#endif

void MixEditWindow::buildBody(Window *form)
{
  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);
  form->setFlexLayout();

  MixData *mix = mixAddress(index);

  // Mix name
  auto line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_NAME);
  new ModelTextEdit(line, rect_t{}, mix->name, sizeof(mix->name));

  // Source
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_SOURCE);
  new SourceChoice(line, rect_t{},
                   [=]() { return mix->srcRaw; },
                   [=](SourceRef ref) {
                     mix->srcRaw = ref;
                     SET_DIRTY();
                   }, true);

  // Weight
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_WEIGHT);
  auto svar = new SourceNumberEdit(line, MIX_WEIGHT_MIN, MIX_WEIGHT_MAX,
                                   [=]() -> int32_t { return valueOrSourceToLegacy(mix->weight); },
                                   [=](int32_t newValue) {
                                     mix->weight = legacyToValueOrSource(newValue);
                                     SET_DIRTY();
                                   }, MIXSRC_FIRST);
  svar->setSuffix("%");

  // Offset
  new StaticText(line, rect_t{}, STR_OFFSET);
  auto gvar = new SourceNumberEdit(line, MIX_OFFSET_MIN, MIX_OFFSET_MAX,
                                   [=]() -> int32_t { return valueOrSourceToLegacy(mix->offset); },
                                   [=](int32_t newValue) {
                                     mix->offset = legacyToValueOrSource(newValue);
                                     SET_DIRTY();
                                   }, MIXSRC_FIRST);
  gvar->setSuffix("%");

  // Switch
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_SWITCH);
  new SwitchChoice(line, rect_t{},
                   [=]() { return mix->swtch; },
                   [=](SwitchRef ref) {
                     mix->swtch = ref;
                     SET_DIRTY();
                   });

  // Curve
  mixsrc_t srcForCurve = sourceRefToMixSrc(mix->srcRaw);
  new StaticText(line, rect_t{}, STR_CURVE);
  new CurveParam(line, rect_t{}, &mix->curve,
      [=](int32_t newValue) {
        mix->curve.value = legacyToValueOrSource(newValue);
        SET_DIRTY();
      }, MIXSRC_FIRST, srcForCurve);

  line = form->newLine(grid);
  line->padAll(PAD_LARGE);
  auto btn =
      new TextButton(line, rect_t{}, LV_SYMBOL_SETTINGS, [=]() -> uint8_t {
        new MixEditAdvanced(channel, index);
        return 0;
      });
  lv_obj_set_width(btn->getLvObj(), lv_pct(100));
}
