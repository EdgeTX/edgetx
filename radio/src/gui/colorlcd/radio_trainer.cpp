/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

void RadioTrainerPage::build(FormWindow * window)
{
#if LCD_W > LCD_H
#define TRAINER_LABEL_WIDTH  180
#else
#define TRAINER_LABEL_WIDTH  100
#endif
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);
  grid.setLabelWidth(TRAINER_LABEL_WIDTH);

  for (uint8_t i=0; i<NUM_STICKS; i++) {
    uint8_t chan = channelOrder(i+1);
    TrainerMix * td = &g_eeGeneral.trainer.mix[chan-1];
    new StaticText(window, grid.getLabelSlot(), TEXT_AT_INDEX(STR_VSRCRAW, (i + 1)), 0, COLOR_THEME_PRIMARY1);
    new Choice(window, grid.getFieldSlot(3,0), STR_TRNMODE, 0, 2, GET_SET_DEFAULT(td->mode));
    auto weight = new NumberEdit(window, grid.getFieldSlot(3, 1), -125, 125, GET_SET_DEFAULT(td->studWeight));
    weight->setSuffix("%");
    new Choice(window, grid.getFieldSlot(3,2), STR_TRNCHN, 0, 3, GET_SET_DEFAULT(td->srcChn));
    grid.nextLine();
  }
  grid.nextLine();

  // Trainer multiplier
  new StaticText(window, grid.getLabelSlot(), STR_MULTIPLIER, 0, COLOR_THEME_PRIMARY1);
  auto multiplier = new NumberEdit(window, grid.getFieldSlot(3, 0), -10, 40, GET_SET_DEFAULT(g_eeGeneral.PPM_Multiplier));
  multiplier->setDisplayHandler([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
    dc->drawNumber(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value+10, flags | PREC1, 0);
  });
  grid.nextLine();
  grid.nextLine();

  // Trainer calibration
  new StaticText(window, grid.getLabelSlot(), STR_CAL, 0, COLOR_THEME_PRIMARY1);
  for (int i = 0; i < NUM_STICKS; i++) {
#if defined (PPM_UNIT_PERCENT_PREC1)
    auto calib = new NumberEdit(window, grid.getFieldSlot(4, i), 0 , 0, [=]() { return (ppmInput[i]-g_eeGeneral.trainer.calib[i]) * 2; }, nullptr, 0, LEFT | PREC1);
#else
    auto calib = new NumberEdit(window, grid.getFieldSlot(4, i), 0 , 0, [=]() { return (ppmInput[i]-g_eeGeneral.trainer.calib[i]) / 5; }, nullptr, 0, LEFT);
#endif
    calib->setWindowFlags(REFRESH_ALWAYS);
  }
}
