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
#include "source_numberedit.h"
#include "curve_param.h"
#include "mixer_edit_adv.h"
#include "mixes.h"
#include "edgetx.h"
#include "etx_lv_theme.h"
#include "sourcechoice.h"
#include "switchchoice.h"
#include "curveedit.h"

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

  static LAYOUT_VAL(MIX_STATUS_BAR_MARGIN, 3, 0)

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
      {window->getRect().w - MIX_STATUS_BAR_WIDTH - MIX_RIGHT_MARGIN, 0,
       MIX_STATUS_BAR_WIDTH, EdgeTxStyles::MENU_HEADER_HEIGHT},
      channel);
}

#if !PORTRAIT_LCD
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
  new SourceChoice(line, rect_t{}, 0, MIXSRC_LAST,
                   GET_SET_DEFAULT(mix->srcRaw), true);

  // Weight
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_WEIGHT);
  auto svar = new SourceNumberEdit(line, MIX_WEIGHT_MIN, MIX_WEIGHT_MAX,
                                   GET_SET_DEFAULT(mix->weight), MIXSRC_FIRST);
  svar->setSuffix("%");

  // Offset
  new StaticText(line, rect_t{}, STR_OFFSET);
  auto gvar = new SourceNumberEdit(line, MIX_OFFSET_MIN, MIX_OFFSET_MAX,
                                   GET_SET_DEFAULT(mix->offset), MIXSRC_FIRST);
  gvar->setSuffix("%");

  // Switch
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_SWITCH);
  new SwitchChoice(line, rect_t{}, SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES,
                   GET_SET_DEFAULT(mix->swtch));

  // Curve
  new StaticText(line, rect_t{}, STR_CURVE);
  new CurveParam(line, rect_t{}, &mix->curve, SET_DEFAULT(mix->curve.value), MIXSRC_FIRST, mix->srcRaw);

  line = form->newLine(grid);
  line->padAll(PAD_LARGE);
  auto btn =
      new TextButton(line, rect_t{}, LV_SYMBOL_SETTINGS, [=]() -> uint8_t {
        new MixEditAdvanced(channel, index);
        return 0;
      });
  lv_obj_set_width(btn->getLvObj(), lv_pct(100));
}
