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

#include "radio_trainer.h"
#include "opentx.h"
#include "libopenui.h"

#define SET_DIRTY()     storageDirty(EE_GENERAL)

RadioTrainerPage::RadioTrainerPage():
  PageTab(STR_MENUTRAINER, ICON_RADIO_TRAINER)
{
}

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};

#if LCD_H >= LCD_W
static const lv_coord_t col4_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                      LV_GRID_FR(1), LV_GRID_FR(1),
                                      LV_GRID_TEMPLATE_LAST};
#endif

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

#if defined(PPM_UNIT_PERCENT_PREC1)
  #define PPM_PRECISION PREC1
#else
  #define PPM_PRECISION 0
#endif

void RadioTrainerPage::build(FormWindow * form)
{
  FlexGridLayout grid(col_dsc, row_dsc, 2);
  form->setFlexLayout();

  for (uint8_t i = 0; i < NUM_STICKS; i++) {
    uint8_t chan = channelOrder(i + 1);
    TrainerMix* td = &g_eeGeneral.trainer.mix[chan - 1];

    auto line = form->newLine(&grid);
    new StaticText(line, rect_t{}, STR_VSRCRAW[i + 1], 0, COLOR_THEME_PRIMARY1);

    auto box = new FormGroup(line, rect_t{});
    box->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(4));
    auto box_obj = box->getLvObj();
    lv_obj_set_width(box_obj, LV_SIZE_CONTENT);
    lv_obj_set_style_flex_cross_place(box_obj, LV_FLEX_ALIGN_CENTER, 0);

    new Choice(box, rect_t{}, STR_TRNMODE, 0, 2, GET_SET_DEFAULT(td->mode));
    new Choice(box, rect_t{}, STR_TRNCHN, 0, 3, GET_SET_DEFAULT(td->srcChn));
    auto weight = new NumberEdit(box, rect_t{}, -125, 125,
                                 GET_SET_DEFAULT(td->studWeight));
    weight->setSuffix("%");

#if LCD_W > LCD_H
    new DynamicNumber<int16_t>(box, rect_t{},
        [=]() { return (ppmInput[i] - g_eeGeneral.trainer.calib[i]) * 2; },
        LEFT | PPM_PRECISION | COLOR_THEME_PRIMARY1);
#endif
  }

#if LCD_H >= LCD_W
  {
    FlexGridLayout grid4(col4_dsc, row_dsc, 2);
    auto line = form->newLine(&grid4);
    
    for (uint8_t i = 0; i < NUM_STICKS; i++) {
      new DynamicNumber<int16_t>(line, rect_t{},
          [=]() { return (ppmInput[i] - g_eeGeneral.trainer.calib[i]) * 2; },
          CENTERED | PPM_PRECISION | COLOR_THEME_PRIMARY1);
    }
  }
#endif

  // Trainer calibration
  auto line = form->newLine(&grid);
  auto btn = new TextButton(line, rect_t{}, std::string(STR_CAL), [=]() -> uint8_t {
    memcpy(g_eeGeneral.trainer.calib, ppmInput,
           sizeof(g_eeGeneral.trainer.calib));
    SET_DIRTY();
    return 0;
  });
  lv_obj_set_style_min_width(btn->getLvObj(), 2*LV_DPI_DEF/3, 0);

  // Trainer multiplier
  auto box = new FormGroup(line, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(4));
  auto box_obj = box->getLvObj();
  lv_obj_set_width(box_obj, LV_SIZE_CONTENT);
  lv_obj_set_style_flex_cross_place(box_obj, LV_FLEX_ALIGN_CENTER, 0);
  
  new StaticText(box, rect_t{}, STR_MULTIPLIER, 0, COLOR_THEME_PRIMARY1);
  auto multiplier = new NumberEdit(box, rect_t{}, -10, 40,
                                   GET_SET_DEFAULT(g_eeGeneral.PPM_Multiplier));
  multiplier->setDisplayHandler(
      [](int32_t value) { return formatNumberAsString(value + 10, PREC1); });
}
