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
#include "mixer_edit_adv.h"
#include "channel_bar.h"
#include "gvar_numberedit.h"
#include "curve_param.h"
#include "mixes.h"

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
      Window(parent, rect), _channel(channel)
  {
    channelBar =
        new ComboChannelBar(this,
                            {MIX_STATUS_BAR_MARGIN, 0,
                             rect.w - (MIX_STATUS_BAR_MARGIN * 2), rect.h},
                            channel);
    channelBar->setLeftMargin(15);
    channelBar->setTextColor(COLOR_THEME_PRIMARY2);
    channelBar->setOutputChannelBarLimitColor(COLOR_THEME_EDIT);
  }

 protected:
  ComboChannelBar *channelBar;
  int8_t _channel;
};

MixEditWindow::MixEditWindow(int8_t channel, uint8_t index) :
    Page(ICON_MODEL_MIXER), channel(channel), index(index)
{
  auto form = new FormWindow(&body, rect_t{});
  lv_obj_set_style_pad_all(form->getLvObj(), lv_dpx(8), 0);

  buildBody(form);
  buildHeader(&header);
}

void MixEditWindow::buildHeader(Window *window)
{
  std::string title2(getSourceString(MIXSRC_FIRST_CH + channel));
  header.setTitle(STR_MIXES);
  header.setTitle2(title2);

  new MixerEditStatusBar(
      window,
      {window->getRect().w - MIX_STATUS_BAR_WIDTH - MIX_RIGHT_MARGIN, 0,
       MIX_STATUS_BAR_WIDTH, MENU_HEADER_HEIGHT},
      channel);
}

#if LCD_W > LCD_H
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_FR(1), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
#endif

void MixEditWindow::buildBody(FormWindow* form)
{
  FlexGridLayout grid(col_dsc, row_dsc, 2);
  form->setFlexLayout();

  MixData *mix = mixAddress(index);

  // Mix name
  auto line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_NAME, 0, COLOR_THEME_PRIMARY1);
  new ModelTextEdit(line, rect_t{}, mix->name, sizeof(mix->name));

  // Source
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_SOURCE, 0, COLOR_THEME_PRIMARY1);
  new SourceChoice(line, rect_t{}, 0, MIXSRC_LAST,
                   GET_SET_DEFAULT(mix->srcRaw));
  CurveEdit::SetCurrentSource(mix->srcRaw);

  // Weight
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_WEIGHT, 0, COLOR_THEME_PRIMARY1);
  auto gvar = new GVarNumberEdit(line, rect_t{}, MIX_WEIGHT_MIN, MIX_WEIGHT_MAX,
                                 GET_SET_DEFAULT(mix->weight));
  gvar->setSuffix("%");

  // Offset
  new StaticText(line, rect_t{}, STR_OFFSET, 0, COLOR_THEME_PRIMARY1);
  gvar = new GVarNumberEdit(line, rect_t{}, MIX_OFFSET_MIN, MIX_OFFSET_MAX,
                            GET_SET_DEFAULT(mix->offset));
  gvar->setSuffix("%");

  // Switch
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_SWITCH, 0, COLOR_THEME_PRIMARY1);
  new SwitchChoice(line, rect_t{}, SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES,
                   GET_SET_DEFAULT(mix->swtch));

  // Curve
  new StaticText(line, rect_t{}, STR_CURVE, 0, COLOR_THEME_PRIMARY1);
  new CurveParam(line, rect_t{}, &mix->curve, SET_DEFAULT(mix->curve.value));

  line = form->newLine();
  lv_obj_set_style_pad_all(line->getLvObj(), lv_dpx(8), 0);
  auto btn = new TextButton(line, rect_t{}, LV_SYMBOL_SETTINGS, [=]() -> uint8_t {
    new MixEditAdvanced(channel, index);
    return 0;
  });
  lv_obj_set_width(btn->getLvObj(), lv_pct(100));
}

void MixEditWindow::deleteLater(bool detach, bool trash)
{
  if (!deleted()) {
    CurveEdit::SetCurrentSource(0);
    Page::deleteLater(detach, trash);
  }
}
