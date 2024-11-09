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

#include "hal/adc_driver.h"
#include "input_mapping.h"
#include "libopenui.h"
#include "edgetx.h"
#include "strhelpers.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

RadioTrainerPage::RadioTrainerPage() :
    PageTab(STR_MENUTRAINER, ICON_RADIO_TRAINER)
{
}

#if !PORTRAIT_LCD
static const lv_coord_t col_dsc[] = {LV_GRID_FR(7),  LV_GRID_FR(13),
                                     LV_GRID_FR(10), LV_GRID_FR(10),
                                     LV_GRID_FR(10), LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t col_dsc[] = {LV_GRID_FR(7), LV_GRID_FR(15),
                                     LV_GRID_FR(9), LV_GRID_FR(9),
                                     LV_GRID_TEMPLATE_LAST};
#endif

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

void RadioTrainerPage::build(Window* form)
{
  form->padAll(PAD_SMALL);

  if (SLAVE_MODE()) {
    form->setHeight(TabsGroup::MENU_BODY_HEIGHT);
    auto txt = new StaticText(form, rect_t{}, STR_SLAVE, COLOR_THEME_PRIMARY1_INDEX, FONT(L));
    lv_obj_align(txt->getLvObj(), LV_ALIGN_CENTER, 0, 0);
  } else {
    FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);
    form->setFlexLayout();

    auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
    for (uint8_t i = 0; i < max_sticks; i++) {
      uint8_t chan = inputMappingChannelOrder(i);
      TrainerMix* td = &g_eeGeneral.trainer.mix[chan];

      auto line = form->newLine(grid);
      new StaticText(line, rect_t{}, getMainControlLabel(chan));

      new Choice(line, rect_t{}, STR_TRNMODE, 0, 2, GET_SET_DEFAULT(td->mode));
      new Choice(line, rect_t{}, STR_TRNCHN, 0, 3, GET_SET_DEFAULT(td->srcChn));
      auto weight = new NumberEdit(line, rect_t{0, 0, NUM_EDIT_W, 0}, -125, 125,
                                   GET_SET_DEFAULT(td->studWeight));
      weight->setSuffix("%");

#if PORTRAIT_LCD
      line = form->newLine(grid);
      line->padLeft(30);
      line->padBottom(PAD_LARGE);
#endif

      LcdFlags flags = LEFT;
      if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1) flags |= PREC1;

      new DynamicNumber<int16_t>(
          line, rect_t{},
          [=]() {
            return (trainerInput[i] - g_eeGeneral.trainer.calib[i]) * 2;
          },
          COLOR_THEME_PRIMARY1_INDEX, flags);
    }

    auto line = form->newLine(grid);
#if PORTRAIT_LCD
    line->padTop(10);
#else
    line->padTop(PAD_MEDIUM);
#endif

    // Trainer multiplier
    if (g_model.trainerData.mode == TRAINER_MODE_MASTER_TRAINER_JACK) {
      auto lbl = new StaticText(line, rect_t{}, STR_MULTIPLIER);
      lbl->padRight(PAD_SMALL);
      lv_obj_set_grid_cell(lbl->getLvObj(), LV_GRID_ALIGN_END, 0, 2,
                           LV_GRID_ALIGN_CENTER, 0, 1);

      auto multiplier =
              new NumberEdit(line, rect_t{0, 0, NUM_EDIT_W, 0}, -10, 40,
                             GET_SET_DEFAULT(g_eeGeneral.PPM_Multiplier));
      multiplier->setDisplayHandler(
              [](int32_t value) { return formatNumberAsString(value + 10, PREC1); });
      lv_obj_set_grid_cell(multiplier->getLvObj(), LV_GRID_ALIGN_START, 2, 1,
                           LV_GRID_ALIGN_CENTER, 0, 1);

#if PORTRAIT_LCD
      line = form->newLine(grid);
      line->padTop(10);
#endif
    }

    // Trainer calibration
    auto btn = new TextButton(line, rect_t{}, std::string(STR_CALIBRATION),
                              [=]() -> uint8_t {
                                memcpy(g_eeGeneral.trainer.calib, trainerInput,
                                       sizeof(g_eeGeneral.trainer.calib));
                                SET_DIRTY();
                                return 0;
                              });
#if PORTRAIT_LCD
    lv_obj_set_grid_cell(btn->getLvObj(), LV_GRID_ALIGN_STRETCH, 1, 2,
                         LV_GRID_ALIGN_CENTER, 0, 1);
#else
    lv_obj_set_grid_cell(btn->getLvObj(), LV_GRID_ALIGN_START, 3, 2,
                         LV_GRID_ALIGN_CENTER, 0, 1);
#endif
  }
}
