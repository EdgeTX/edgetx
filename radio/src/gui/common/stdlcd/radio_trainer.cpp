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

#include "edgetx.h"
#include "hal/adc_driver.h"
#include "input_mapping.h"

#define COL_TWO (5 * FW)
#if LCD_W >= 212
#define COL_THREE (16 * FW)
#define COL_FOUR (17 * FW)
#define TRAINER_CALIB_COLUMN_WIDTH (6 * FW)
#else
#define COL_THREE (12 * FW)
#define COL_FOUR (13 * FW)
#define TRAINER_CALIB_COLUMN_WIDTH (4 * FW + 2)
#endif

void menuRadioTrainer(event_t event)
{
  uint8_t y;
  bool slave = SLAVE_MODE();
  auto controls = adcGetMaxInputs(ADC_INPUT_MAIN);

  MENU(STR_MENUTRAINER, menuTabGeneral, MENU_RADIO_TRAINER, (slave ? HEADER_LINE : HEADER_LINE+6),
      {
        HEADER_LINE_COLUMNS
        NAVIGATION_LINE_BY_LINE|2,
        NAVIGATION_LINE_BY_LINE|2,
        (uint8_t)((controls > 2) ? NAVIGATION_LINE_BY_LINE|2 : HIDDEN_ROW),
        (uint8_t)((controls > 2) ? NAVIGATION_LINE_BY_LINE|2 : HIDDEN_ROW),
        0, 0
      });

  if (slave) {
    lcdDrawText(LCD_W/2, 4*FH, STR_SLAVE, CENTERED);
    return;
  }

  LcdFlags attr;
  LcdFlags blink = ((s_editMode>0) ? BLINK|INVERS : INVERS);

  lcdDrawText(COL_TWO, MENU_HEADER_HEIGHT + 1, STR_MODE);
  lcdDrawText(COL_THREE, MENU_HEADER_HEIGHT + 1, "%", RIGHT);
  lcdDrawText(COL_FOUR, MENU_HEADER_HEIGHT + 1, STR_SOURCE);

  y = MENU_HEADER_HEIGHT + 1 + FH;

  for (uint8_t i = HEADER_LINE; i < HEADER_LINE + controls; i++) {
    uint8_t chan = inputMappingChannelOrder(i - HEADER_LINE);
    TrainerMix * td = &g_eeGeneral.trainer.mix[chan];

    drawSource(0, y, MIXSRC_FIRST_STICK + chan,
               (menuVerticalPosition==i && CURSOR_ON_LINE()) ? INVERS : 0);

    for (uint8_t j=0; j<3; j++) {

      attr = ((menuVerticalPosition==i && menuHorizontalPosition==j) ? blink : 0);

      switch (j) {
        case 0:
          lcdDrawTextAtIndex(COL_TWO, y, STR_TRNMODE, td->mode, attr);
          if (attr & BLINK) CHECK_INCDEC_GENVAR(event, td->mode, 0, 2);
          break;

        case 1:
          lcdDrawNumber(COL_THREE, y, td->studWeight, attr | RIGHT);
          if (attr & BLINK)
            CHECK_INCDEC_GENVAR(event, td->studWeight, -125, 125);
          break;

        case 2:
          lcdDrawTextAtIndex(COL_FOUR, y, STR_TRNCHN, td->srcChn, attr);
          if (attr & BLINK) CHECK_INCDEC_GENVAR(event, td->srcChn, 0, 3);
          break;
      }
    }
    y += FH;
  }

  attr = (menuVerticalPosition==HEADER_LINE+4) ? blink : 0;
  if (g_model.trainerData.mode == TRAINER_MODE_MASTER_TRAINER_JACK) {
      lcdDrawTextAlignedLeft(y, STR_MULTIPLIER);
      lcdDrawNumber(LEN_MULTIPLIER * FW + 3 * FW, y, g_eeGeneral.PPM_Multiplier + 10, attr | PREC1 | RIGHT);
      if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.PPM_Multiplier, -10, 40);
  }
  y += FH;

  attr = (menuVerticalPosition==HEADER_LINE+5) ? INVERS : 0;
  if (attr)
    s_editMode = 0;
  lcdDrawText(0*FW, y, STR_CAL, attr);
  for (uint8_t i = 0; i < 4; i++) {
    uint8_t x = 8*FW + (i * TRAINER_CALIB_COLUMN_WIDTH);
    int32_t chVal = trainerInput[i] - g_eeGeneral.trainer.calib[i];
    chVal *= g_eeGeneral.trainer.mix[i].studWeight * 10;
    chVal /= 512;
    if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1) {
      lcdDrawNumber(x, y, chVal, PREC1|RIGHT);
    } else {
      lcdDrawNumber(x, y, chVal / 10, RIGHT);
    }
  }

  if (attr) {
    s_editMode = 0;
    if (event==EVT_KEY_LONG(KEY_ENTER)){
      killEvents(event);
      memcpy(g_eeGeneral.trainer.calib, trainerInput, sizeof(g_eeGeneral.trainer.calib));
      storageDirty(EE_GENERAL);
      AUDIO_WARNING1();
    }
  }
}
