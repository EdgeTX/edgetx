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
#include "gui/common/stdlcd/calibration.h"

#define XPOT_DELTA    10
#define XPOT_DELAY    10 /* cycles */
#define BAR_SPACING   12
#define BAR_HEIGHT    22

void drawPotsBars()
{
  // Optimization by Mike Blandford
  uint8_t max_pots = adcGetMaxInputs(ADC_INPUT_FLEX);
  uint8_t offset = adcGetInputOffset(ADC_INPUT_FLEX);

  for (uint8_t x = LCD_W / 2 - max_pots / 2 * BAR_SPACING + BAR_SPACING / 2,
               i = 0;
       i < max_pots; x += BAR_SPACING, i++) {
    if (IS_POT_SLIDER_AVAILABLE(i)) {
      // calculate once per loop
      auto v = calibratedAnalogs[offset + i] + RESX;
      uint8_t len = (v * BAR_HEIGHT / (RESX * 2)) + 1l;
      V_BAR(x, LCD_H - 8, len);
      lcdDrawText(x - 2, LCD_H - 6, getPotLabel(i), TINSIZE);
    }
  }
}

void menuRadioCalibration(event_t event)
{
  check_submenu_simple(event, 0);
  title(STR_MENUCALIBRATION);
  menuCommonCalib(event);
  drawPotsBars();
  if (menuEvent) {
    menuCalibrationState = CALIB_START;
  }
}

void menuFirstCalib(event_t event)
{
  if (event == EVT_KEY_BREAK(KEY_EXIT) || reusableBuffer.calib.state == CALIB_FINISHED) {
    menuCalibrationState = CALIB_START;
    chainMenu(menuMainView);
  }
  else {
    lcdDrawText(LCD_W / 2, 0, STR_MENUCALIBRATION, CENTERED);
    lcdInvertLine(0);
    menuCommonCalib(event);
    drawPotsBars();
  }
}
