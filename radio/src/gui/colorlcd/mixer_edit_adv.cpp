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
#include "numberedit.h"
#include "fm_matrix.h"

#include "opentx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

MixEditAdvanced::MixEditAdvanced(int8_t channel, uint8_t index) :
    Page(ICON_MODEL_MIXER), channel(channel), index(index)
{
  std::string title(STR_MIXES);
  title += "\n";
  title += getSourceString(MIXSRC_FIRST_CH + channel);
  header.setTitle(title);

  auto form = new FormWindow(&body, rect_t{});
  lv_obj_set_style_pad_all(form->getLvObj(), lv_dpx(8), 0);

  buildBody(form);
}

#if LCD_W > LCD_H
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
#endif

void MixEditAdvanced::buildBody(FormWindow* form)
{
  FlexGridLayout grid(col_dsc, row_dsc, 2);
  form->setFlexLayout();

  MixData *mix = mixAddress(index);

  // Advanced...
  
  // Multiplex
  auto line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_MULTPX, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_VMLTPX, 0, 2, GET_SET_DEFAULT(mix->mltpx));

  // Flight modes
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_FLMODE, 0, COLOR_THEME_PRIMARY1);
  new FMMatrix<MixData>(line, rect_t{}, mix);

  // Trim
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_TRIM, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_INVERTED(mix->carryTrim));

  // Warning
  new StaticText(line, rect_t{}, STR_MIXWARNING, 0, COLOR_THEME_PRIMARY1);
  auto edit = new NumberEdit(line, rect_t{}, 0, 3, GET_SET_DEFAULT(mix->mixWarn));
  edit->setZeroText(STR_OFF);

  // Delay up
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_DELAYUP, 0, COLOR_THEME_PRIMARY1);
  edit = new NumberEdit(line, rect_t{}, 0, DELAY_MAX, GET_DEFAULT(mix->delayUp),
                        SET_VALUE(mix->delayUp, newValue), 0, PREC1);
  edit->setSuffix("s");

  // Delay down
  new StaticText(line, rect_t{}, STR_DELAYDOWN, 0, COLOR_THEME_PRIMARY1);
  edit =
      new NumberEdit(line, rect_t{}, 0, DELAY_MAX, GET_DEFAULT(mix->delayDown),
                     SET_VALUE(mix->delayDown, newValue), 0, PREC1);
  edit->setSuffix("s");

  // Slow up
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_SLOWUP, 0, COLOR_THEME_PRIMARY1);
  edit = new NumberEdit(line, rect_t{}, 0, DELAY_MAX, GET_DEFAULT(mix->speedUp),
                        SET_VALUE(mix->speedUp, newValue), 0, PREC1);
  edit->setSuffix("s");

  // Slow down
  new StaticText(line, rect_t{}, STR_SLOWDOWN, 0, COLOR_THEME_PRIMARY1);
  edit =
      new NumberEdit(line, rect_t{}, 0, DELAY_MAX, GET_DEFAULT(mix->speedDown),
                     SET_VALUE(mix->speedDown, newValue), 0, PREC1);
  edit->setSuffix("s");
}
