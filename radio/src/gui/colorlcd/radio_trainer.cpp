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
#include "input_mapping.h"

#include "hal/adc_driver.h"
#include "strhelpers.h"

#define SET_DIRTY()     storageDirty(EE_GENERAL)

RadioTrainerPage::RadioTrainerPage():
  PageTab(STR_MENUTRAINER, ICON_RADIO_TRAINER)
{
}

#if LCD_W > LCD_H
static const lv_coord_t col_dsc[] = {LV_GRID_FR(7), LV_GRID_FR(13), LV_GRID_FR(10), LV_GRID_FR(10), LV_GRID_FR(10),
                                     LV_GRID_TEMPLATE_LAST};
                                     
#define MULT_COL_CNT    3
#else
static const lv_coord_t col_dsc[] = {LV_GRID_FR(7), LV_GRID_FR(15), LV_GRID_FR(9), LV_GRID_FR(9),
                                     LV_GRID_TEMPLATE_LAST};
#define MULT_COL_CNT    2
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
  form->padAll(4);
#if LCD_W > LCD_H
  form->padLeft(8);
  form->padRight(8);
#endif

  auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (uint8_t i = 0; i < max_sticks; i++) {
    uint8_t chan = inputMappingChannelOrder(i);
    TrainerMix* td = &g_eeGeneral.trainer.mix[chan];

    auto line = form->newLine(&grid);
    new StaticText(line, rect_t{}, getMainControlLabel(chan), 0, COLOR_THEME_PRIMARY1);

    new Choice(line, rect_t{}, STR_TRNMODE, 0, 2, GET_SET_DEFAULT(td->mode));
    new Choice(line, rect_t{}, STR_TRNCHN, 0, 3, GET_SET_DEFAULT(td->srcChn));
    auto weight = new NumberEdit(line, rect_t{}, -125, 125,
                                 GET_SET_DEFAULT(td->studWeight));
    weight->setSuffix("%");

#if LCD_H > LCD_W
    line = form->newLine(&grid);
    line->padLeft(30);
    line->padBottom(8);
#endif

    auto d = new DynamicNumber<int16_t>(line, rect_t{},
        [=]() { return (ppmInput[i] - g_eeGeneral.trainer.calib[i]) * 2; },
        LEFT | PPM_PRECISION | COLOR_THEME_PRIMARY1);
  }

  auto line = form->newLine(&grid);
  line->padTop(10);

  // Trainer multiplier
  auto lbl = new StaticText(line, rect_t{}, STR_MULTIPLIER, 0, COLOR_THEME_PRIMARY1);
  lbl->padRight(4);
  lv_obj_set_grid_cell(lbl->getLvObj(), LV_GRID_ALIGN_END, 0, MULT_COL_CNT, LV_GRID_ALIGN_CENTER, 0, 1);
  auto multiplier = new NumberEdit(line, rect_t{}, -10, 40,
                                   GET_SET_DEFAULT(g_eeGeneral.PPM_Multiplier));
  multiplier->setDisplayHandler(
      [](int32_t value) { return formatNumberAsString(value + 10, PREC1); });
  lv_obj_set_grid_cell(multiplier->getLvObj(), LV_GRID_ALIGN_START, MULT_COL_CNT, 1, LV_GRID_ALIGN_CENTER, 0, 1);

#if LCD_H > LCD_W
  line = form->newLine(&grid);
  line->padTop(10);
#endif

  // Trainer calibration
  auto btn = new TextButton(line, rect_t{0, 0, 0, 30}, std::string(STR_CAL), [=]() -> uint8_t {
    memcpy(g_eeGeneral.trainer.calib, ppmInput,
           sizeof(g_eeGeneral.trainer.calib));
    SET_DIRTY();
    return 0;
  });
  lv_obj_set_grid_cell(btn->getLvObj(), LV_GRID_ALIGN_STRETCH, MULT_COL_CNT+1, 1, LV_GRID_ALIGN_CENTER, 0, 1);
}
