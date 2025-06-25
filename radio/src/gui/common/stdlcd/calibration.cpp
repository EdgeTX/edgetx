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
#include "calibration.h"
#include "analogs.h"

#include "hal/adc_driver.h"

void menuCommonCalibOptions(event_t event)
{
  SIMPLE_SUBMENU(STR_AXISDIR, HEADER_LINE+4);
  uint8_t sub = menuVerticalPosition;

  coord_t y = MENU_HEADER_HEIGHT + 1;
  lcdDrawTextAlignedLeft(y, STR_STICKS);
  lcdDrawText(LCD_W / 3 * 2, y, STR_MENU_INVERT, CENTERED);
  y+=FH;
  for(uint8_t i = 0; i < 4; i++) {
    uint8_t k = i + menuVerticalOffset;
    LcdFlags attr = (sub == k) ? INVERS : 0;

    lcdDrawTextIndented(y, STR_CHAR_STICK);
    lcdDrawText(lcdNextPos, y, analogGetCanonicalName(ADC_INPUT_MAIN, i), 0);
    bool stickInversion = getStickInversion(i);
    lcdDrawChar(LCD_W / 3 * 2, y, stickInversion ? 127 : 126, attr);
    if (attr) stickInversion = checkIncDec(event, stickInversion, 0, 1, EE_GENERAL);
    setStickInversion(i, stickInversion);
    y+=FH;
  }
}

void menuCommonCalib(event_t event)
{
  // make sure we don't scroll while calibrating
  menuCalibrationState = reusableBuffer.calib.state;

  switch (event) {
    case EVT_ENTRY:
    case EVT_KEY_BREAK(KEY_EXIT):
      reusableBuffer.calib.state = CALIB_START;
      break;

    case EVT_KEY_LONG(KEY_ENTER):
      killEvents(event);
      pushMenu(menuCommonCalibOptions);
      break;

    case EVT_KEY_BREAK(KEY_ENTER):
      reusableBuffer.calib.state++;
      break;
  }

  switch (reusableBuffer.calib.state) {
    case CALIB_START:
      // START CALIBRATION
      lcdDrawText(LCD_W/2, MENU_HEADER_HEIGHT+2*FH, STR_MENUTOSTART, CENTERED);
      break;

    case CALIB_SET_MIDPOINT:
      // SET MIDPOINT
      lcdDrawText(LCD_W/2, MENU_HEADER_HEIGHT+FH, STR_SETMIDPOINT, INVERS|CENTERED);
      lcdDrawText(LCD_W/2, MENU_HEADER_HEIGHT+2*FH, STR_MENUWHENDONE, CENTERED);
      adcCalibSetMidPoint();
      break;

    case CALIB_MOVE_STICKS:
      // MOVE STICKS/POTS
      lcdDrawText(LCD_W/2, MENU_HEADER_HEIGHT, STR_MOVESTICKSPOTS, INVERS|CENTERED);
      lcdDrawText(LCD_W/2, MENU_HEADER_HEIGHT+FH, STR_MENUAXISDIR, CENTERED);
      lcdDrawText(LCD_W/2, MENU_HEADER_HEIGHT+2*FH, STR_MENUWHENDONE, CENTERED);
      adcCalibSetMinMax();
      break;

    case CALIB_STORE:
      adcCalibStore();
      reusableBuffer.calib.state = CALIB_FINISHED;
      break;

    default:
      reusableBuffer.calib.state = CALIB_START;
      break;
  }

  doMainScreenGraphics();
}

