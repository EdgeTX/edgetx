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
#include "timers_driver.h"

extern uint8_t g_moduleIdx;

enum SpectrumFields {
  SPECTRUM_FREQUENCY,
  SPECTRUM_SPAN,
  SPECTRUM_TRACK,
  SPECTRUM_FIELDS_MAX
};

#define SPECTRUM_ROW  (isModuleMultimodule(g_moduleIdx) ? READONLY_ROW : (uint8_t)0)

void menuRadioSpectrumAnalyser(event_t event)
{
  SUBMENU(STR_MENU_SPECTRUM_ANALYSER, SPECTRUM_FIELDS_MAX, {
    SPECTRUM_ROW,  //Freq
    SPECTRUM_ROW,  //Span
    0              //Tracker
  });

  if (menuEvent) {
    lcdDrawCenteredText(LCD_H/2, STR_STOPPING);
    lcdRefresh();
#if defined(PXX2)
    moduleState[g_moduleIdx].readModuleInformation(&reusableBuffer.moduleSetup.pxx2.moduleInformation, PXX2_HW_INFO_TX_ID, PXX2_HW_INFO_TX_ID);
#endif
    /* wait 1s to resume normal operation before leaving */
    watchdogSuspend(500 /*5s*/);
    RTOS_WAIT_MS(1000);
    return;
  }

  if (moduleState[g_moduleIdx].mode != MODULE_MODE_SPECTRUM_ANALYSER) {
    if (TELEMETRY_STREAMING()) {
      lcdDrawCenteredText(LCD_H/2, STR_TURN_OFF_RECEIVER);
      if (event == EVT_KEY_BREAK(KEY_EXIT)) {
        popMenu();
      }
      return;
    }

    memclear(reusableBuffer.spectrumAnalyser.bars, sizeof(reusableBuffer.spectrumAnalyser.bars));

    if (isModuleR9MAccess(g_moduleIdx)) {
      reusableBuffer.spectrumAnalyser.spanDefault = 20;
      reusableBuffer.spectrumAnalyser.spanMax = 40;
      reusableBuffer.spectrumAnalyser.freqDefault = 890;
      reusableBuffer.spectrumAnalyser.freqMin = 850;
      reusableBuffer.spectrumAnalyser.freqMax = 930;
    }
    else {
      if (isModuleMultimodule(g_moduleIdx))
        reusableBuffer.spectrumAnalyser.spanDefault = 80;  // 80MHz
      else
        reusableBuffer.spectrumAnalyser.spanDefault = 40;  // 40MHz
      reusableBuffer.spectrumAnalyser.spanMax = 80;
      reusableBuffer.spectrumAnalyser.freqDefault = 2440; // 2440MHz
      reusableBuffer.spectrumAnalyser.freqMin = 2400;
      reusableBuffer.spectrumAnalyser.freqMax = 2485;
    }

    reusableBuffer.spectrumAnalyser.span = reusableBuffer.spectrumAnalyser.spanDefault * 1000000;
    reusableBuffer.spectrumAnalyser.freq = reusableBuffer.spectrumAnalyser.freqDefault * 1000000;
    reusableBuffer.spectrumAnalyser.track = reusableBuffer.spectrumAnalyser.freq;
    reusableBuffer.spectrumAnalyser.step = reusableBuffer.spectrumAnalyser.span / LCD_W;
    reusableBuffer.spectrumAnalyser.dirty = true;
    moduleState[g_moduleIdx].mode = MODULE_MODE_SPECTRUM_ANALYSER;
  }

  for (uint8_t i=0; i<SPECTRUM_FIELDS_MAX; i++) {
    uint8_t sub = menuVerticalPosition;
    LcdFlags attr = (sub==i ? (s_editMode>0 ? BLINK|INVERS : INVERS) : 0);

    switch (i) {
      case SPECTRUM_FREQUENCY: {
        uint16_t frequency = reusableBuffer.spectrumAnalyser.freq / 1000000;
        lcdDrawText(1, 10, "F:", SMLSIZE);
        lcdDrawNumber(lcdLastRightPos + 1, 10, frequency, attr|SMLSIZE);
        lcdDrawText(lcdLastRightPos + 1, 10, "MHz", SMLSIZE);
        if (attr) {
          reusableBuffer.spectrumAnalyser.freq = uint32_t(checkIncDec(event, frequency, reusableBuffer.spectrumAnalyser.freqMin, reusableBuffer.spectrumAnalyser.freqMax, 0)) * 1000000;
          if (checkIncDec_Ret) {
            reusableBuffer.spectrumAnalyser.dirty = true;
          }
        }
        break;
      }

      case SPECTRUM_SPAN: {
        uint8_t span = reusableBuffer.spectrumAnalyser.span / 1000000;
        lcdDrawText(lcdLastRightPos + 2, 10, "S:", SMLSIZE);
        lcdDrawNumber(lcdLastRightPos + 1, 10, reusableBuffer.spectrumAnalyser.span / 1000000, attr | SMLSIZE);
        lcdDrawText(lcdLastRightPos + 1, 10, "MHz", SMLSIZE);
        if (attr) {
          reusableBuffer.spectrumAnalyser.span = checkIncDec(event, span, 1, reusableBuffer.spectrumAnalyser.spanMax, 0) * 1000000;
          if (checkIncDec_Ret) {
            reusableBuffer.spectrumAnalyser.step = reusableBuffer.spectrumAnalyser.span / LCD_W;
            reusableBuffer.spectrumAnalyser.dirty = true;
          }
        }
        break;
      }

      case SPECTRUM_TRACK: {
        uint16_t track = reusableBuffer.spectrumAnalyser.track / 1000000;
        lcdDrawText(lcdNextPos + 2, 10, "T:", SMLSIZE);
        lcdDrawNumber(lcdNextPos + 1, 10, reusableBuffer.spectrumAnalyser.track / 1000000, attr | SMLSIZE);
        lcdDrawText(lcdNextPos + 1, 10, "MHz", SMLSIZE);
        if (attr) {
          reusableBuffer.spectrumAnalyser.track = uint32_t(
            checkIncDec(event, track, (reusableBuffer.spectrumAnalyser.freq - reusableBuffer.spectrumAnalyser.span / 2) / 1000000,
                        (reusableBuffer.spectrumAnalyser.freq + reusableBuffer.spectrumAnalyser.span / 2) / 1000000, 0)) * 1000000;
          if (checkIncDec_Ret) {
            reusableBuffer.spectrumAnalyser.dirty = true;
          }
        }
        break;
      }
    }
  }

  // Signal bar
  for (uint8_t i=0; i<LCD_W; i++) {
    uint8_t h = min<uint8_t >(reusableBuffer.spectrumAnalyser.bars[i] >> 1, LCD_H);
    lcdDrawSolidVerticalLine(i, LCD_H - h, h);
  }

  // Signal max
  for (uint8_t i=0; i<LCD_W; i++) {
    uint8_t h = min<uint8_t >(reusableBuffer.spectrumAnalyser.max[i] >> 1, LCD_H);
    lcdDrawPoint(i, LCD_H - h);
    if (reusableBuffer.spectrumAnalyser.max[i] > 1)
      reusableBuffer.spectrumAnalyser.max[i] -= 1;
  }

  // Draw Tracker
  int offset = reusableBuffer.spectrumAnalyser.track - (reusableBuffer.spectrumAnalyser.freq - reusableBuffer.spectrumAnalyser.span / 2);
  int x = offset / reusableBuffer.spectrumAnalyser.step;
  lcdDrawVerticalLine(x, 10+FH+1, LCD_H, SOLID);
}
