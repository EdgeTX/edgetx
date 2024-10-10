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

#include "mixer_edit_adv.h"

#include "fm_matrix.h"
#include "mixes.h"
#include "numberedit.h"
#include "edgetx.h"
#include "etx_lv_theme.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

MixEditAdvanced::MixEditAdvanced(int8_t channel, uint8_t index) :
    Page(ICON_MODEL_MIXER, PAD_MEDIUM), channel(channel), index(index)
{
  std::string title(STR_MIXES);
  title += "\n";
  title += getSourceString(MIXSRC_FIRST_CH + channel);
  header->setTitle(title);

  buildBody(body);
}

#if !PORTRAIT_LCD
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t col_dsc[] = {LV_GRID_FR(12), LV_GRID_FR(13),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
#endif

void MixEditAdvanced::buildBody(Window* form)
{
  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);
  form->setFlexLayout();

  MixData* mix = mixAddress(index);

  // Advanced...
  FormLine* line;

  // Multiplex
  if (index > 0 && mixAddress(index - 1)->destCh == channel) {
    line = form->newLine(grid);
    new StaticText(line, rect_t{}, STR_MULTPX);
    new Choice(line, rect_t{}, STR_VMLTPX, 0, 2, GET_SET_DEFAULT(mix->mltpx));
  }

  // Flight modes
  if (modelFMEnabled()) {
    line = form->newLine(grid);
    new StaticText(line, rect_t{}, STR_FLMODE);
    new FMMatrix<MixData>(line, rect_t{}, mix);
  }

  // Trim
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_TRIM);
  new ToggleSwitch(line, rect_t{}, GET_SET_INVERTED(mix->carryTrim));

  // Warning
  new StaticText(line, rect_t{}, STR_MIXWARNING);
  auto edit = new NumberEdit(line, rect_t{0, 0, NUM_EDIT_W, 0}, 0, 3,
                             GET_SET_DEFAULT(mix->mixWarn));
  edit->setZeroText(STR_OFF);

  // Delay up/down precision
#if !PORTRAIT_LCD
  grid.setColSpan(2);
#endif
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_MIX_DELAY_PREC);
  new Choice(line, rect_t{}, &STR_VPREC[1], 0, 1,
             GET_DEFAULT(mix->delayPrec),
             [=](int newValue) {
              mix->delayPrec = newValue;
              delayUp->clearTextFlag(PREC2);
              delayUp->setTextFlag(mix->delayPrec ? PREC2 : PREC1);
              delayUp->update();
              delayDn->clearTextFlag(PREC2);
              delayDn->setTextFlag(mix->delayPrec ? PREC2 : PREC1);
              delayDn->update();
              SET_DIRTY();
             });
#if !PORTRAIT_LCD
  grid.setColSpan(1);
#endif

  // Delay up
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_DELAYUP);
  delayUp = new NumberEdit(line, rect_t{0, 0, NUM_EDIT_W, 0}, 0, DELAY_MAX,
                           GET_DEFAULT(mix->delayUp),
                           SET_VALUE(mix->delayUp, newValue), mix->delayPrec ? PREC2 : PREC1);
  delayUp->setSuffix("s");

  // Delay down
  new StaticText(line, rect_t{}, STR_DELAYDOWN);
  delayDn = new NumberEdit(line, rect_t{0, 0, NUM_EDIT_W, 0}, 0, DELAY_MAX,
                           GET_DEFAULT(mix->delayDown),
                           SET_VALUE(mix->delayDown, newValue), mix->delayPrec ? PREC2 : PREC1);
  delayDn->setSuffix("s");

  // Slow up/down precision
#if !PORTRAIT_LCD
  grid.setColSpan(2);
#endif
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_MIX_SLOW_PREC);
  new Choice(line, rect_t{}, &STR_VPREC[1], 0, 1,
             GET_DEFAULT(mix->speedPrec),
             [=](int newValue) {
              mix->speedPrec = newValue;
              slowUp->clearTextFlag(PREC2);
              slowUp->setTextFlag(mix->speedPrec ? PREC2 : PREC1);
              slowUp->update();
              slowDn->clearTextFlag(PREC2);
              slowDn->setTextFlag(mix->speedPrec ? PREC2 : PREC1);
              slowDn->update();
              SET_DIRTY();
             });
#if !PORTRAIT_LCD
  grid.setColSpan(1);
#endif

  // Slow up
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_SLOWUP);
  slowUp = new NumberEdit(line, rect_t{0, 0, NUM_EDIT_W, 0}, 0, DELAY_MAX, GET_DEFAULT(mix->speedUp),
                          SET_VALUE(mix->speedUp, newValue), mix->speedPrec ? PREC2 : PREC1);
  slowUp->setSuffix("s");

  // Slow down
  new StaticText(line, rect_t{}, STR_SLOWDOWN);
  slowDn = new NumberEdit(line, rect_t{0, 0, NUM_EDIT_W, 0}, 0, DELAY_MAX, GET_DEFAULT(mix->speedDown),
                          SET_VALUE(mix->speedDown, newValue), mix->speedPrec ? PREC2 : PREC1);
  slowDn->setSuffix("s");
}
