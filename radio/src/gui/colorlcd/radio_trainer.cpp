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

class StaticNumber : public StaticText
{
  public:
    StaticNumber(Window * parent, const rect_t & rect, std::function<int()> getValue, WindowFlags windowFlags = 0, LcdFlags textFlags = 0) :
    StaticText(parent, rect, "", windowFlags, textFlags),
    _getValue(std::move(getValue))
    {    
    }

  void paint(BitmapBuffer * dc) {
    auto curval = _getValue();
    char buf[20];
    if(textFlags & PREC1)
      snprintf(buf, sizeof(buf), "%d.%.1d", curval / 10, abs(curval % 10));
    else if(textFlags & PREC2)
      snprintf(buf, sizeof(buf), "%d.%.2d", curval / 100, abs(curval % 100));
    else
      snprintf(buf, sizeof(buf), "%d", curval);
    text = buf;
    StaticText::paint(dc);
  }

  protected:
    std::function<int()> _getValue;
};

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
  new TextButton(window, grid.getLabelSlot(), std::string(STR_CAL), [=]() -> uint8_t {
    memcpy(g_eeGeneral.trainer.calib, ppmInput, sizeof(g_eeGeneral.trainer.calib));
    SET_DIRTY();
    return 0;
    });

  for (int i = 0; i < NUM_STICKS; i++) {
#if defined (PPM_UNIT_PERCENT_PREC1)
    auto calib = new StaticNumber(window, grid.getFieldSlot(4, i), [=]() { return (ppmInput[i]-g_eeGeneral.trainer.calib[i]) * 2; }, 0, LEFT | PREC1);
#else
    auto calib = new StaticNumber(window, grid.getFieldSlot(4, i), [=]() { return (ppmInput[i]-g_eeGeneral.trainer.calib[i]) / 5; }, 0, LEFT);
#endif

    calib->setWindowFlags(REFRESH_ALWAYS);
  }
}
