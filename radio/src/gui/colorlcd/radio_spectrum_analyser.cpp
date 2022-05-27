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

#include "radio_spectrum_analyser.h"
#include "opentx.h"
#include "libopenui.h"

#define SET_DIRTY()     storageDirty(EE_GENERAL)

constexpr coord_t SPECTRUM_HEIGHT = 180;
constexpr coord_t SCALE_HEIGHT = 20;

coord_t getAverage(uint8_t number, const uint8_t * value)
{
  uint16_t sum = 0;
  for (uint8_t i = 0; i < number; i++) {
    sum += value[i];
  }
  return sum / number;
}

class SpectrumFooterWindow: public FormGroup
{
  public:
    SpectrumFooterWindow(FormGroup * parent, const rect_t & rect, int moduleIdx) :
      FormGroup(parent, rect// , FORM_FORWARD_FOCUS
                )
    {
      FormGridLayout grid;
      grid.spacer(4);
      grid.setLabelWidth(5);

      // Tracker
      auto tracker = new NumberEdit(this, grid.getFieldSlot(3, 2), (reusableBuffer.spectrumAnalyser.freq - reusableBuffer.spectrumAnalyser.span / 2) / 1000000,
                                    (reusableBuffer.spectrumAnalyser.freq + reusableBuffer.spectrumAnalyser.span / 2) / 1000000,
                                    GET_DEFAULT(reusableBuffer.spectrumAnalyser.track / 1000000),
                                    SET_VALUE(reusableBuffer.spectrumAnalyser.track, newValue * 1000000));
      tracker->setSuffix("MHz");
      tracker->setPrefix("T: ");
      // tracker->setFocus(SET_FOCUS_DEFAULT);

      if (isModuleMultimodule(moduleIdx)) {
        char label[16];

        // Frequency
        sprintf(label,"T: %dMHz", int(reusableBuffer.spectrumAnalyser.freq / 1000000));
        new StaticText(this, grid.getFieldSlot(3, 0), label);

        // Span
        sprintf(label,"S: %dMHz", int(reusableBuffer.spectrumAnalyser.span / 1000000));
        new StaticText(this, grid.getFieldSlot(3, 1), label);
      }
      else {
        // Frequency
        auto freq = new NumberEdit(this, grid.getFieldSlot(3, 0), reusableBuffer.spectrumAnalyser.freqMin,
                                   reusableBuffer.spectrumAnalyser.freqMax,
                                   GET_DEFAULT(reusableBuffer.spectrumAnalyser.freq / 1000000),
                                   SET_VALUE(reusableBuffer.spectrumAnalyser.freq, newValue * 1000000));
        freq->setSuffix("MHz");
        freq->setPrefix("F: ");

        // Span
        auto span = new NumberEdit(this, grid.getFieldSlot(3, 1), 1, reusableBuffer.spectrumAnalyser.spanMax,
                                   GET_DEFAULT(reusableBuffer.spectrumAnalyser.span / 1000000),
                                   SET_VALUE(reusableBuffer.spectrumAnalyser.span, newValue * 1000000));
        span->setSuffix("MHz");
        span->setPrefix("S: ");
      }
    }
};

class SpectrumScaleWindow: public Window
{
  public:
    SpectrumScaleWindow(Window * parent, const rect_t & rect) :
      Window(parent, rect)
    {
      // This window will be automatically invalidated because it's between the frequency analyser (always invalidated) and the modified field
    }

    void paint(BitmapBuffer * dc) override
    {
      dc->drawSolidFilledRect(0, 0, width(), height(), COLOR_THEME_SECONDARY2);

      // Draw tracker
      int offset = reusableBuffer.spectrumAnalyser.track - (reusableBuffer.spectrumAnalyser.freq - reusableBuffer.spectrumAnalyser.span / 2);
      int x = limit<int>(0, offset / reusableBuffer.spectrumAnalyser.step, width() - 1);
      dc->drawSolidVerticalLine(x, 0, height(), COLOR_THEME_PRIMARY1);

      // Draw text scale
      for (uint32_t frequency =
        ((reusableBuffer.spectrumAnalyser.freq - reusableBuffer.spectrumAnalyser.span / 2) / 10000000) * 10000000 + 10000000;; frequency += 10000000) {
        offset = frequency - (reusableBuffer.spectrumAnalyser.freq - reusableBuffer.spectrumAnalyser.span / 2);
        x = offset / reusableBuffer.spectrumAnalyser.step;
        if (x >= LCD_W - 1)
          break;
        if ((frequency / 1000000) % 2 == 0) {
          dc->drawNumber(x, 3, frequency / 1000000, FONT(XS) | CENTERED);
        }
      }
    }
};

class SpectrumWindow: public Window
{
  public:
    SpectrumWindow(Window * parent, const rect_t & rect) :
      Window(parent, rect, REFRESH_ALWAYS)
    {
    }

    void paint(BitmapBuffer * dc) override
    {
      if (TELEMETRY_STREAMING()) {
        dc->drawText(width() / 2, height() / 2, STR_TURN_OFF_RECEIVER, CENTERED);
        return;
      }

#if defined(SIMU)
      // Geneate random data for simu
      for (coord_t x= 0; x < width(); x++) {
        uint8_t power = rand() % 80;
        reusableBuffer.spectrumAnalyser.bars[x] = power;
        reusableBuffer.spectrumAnalyser.bars[x+1] = power;
        if (power > reusableBuffer.spectrumAnalyser.max[x]) {
          reusableBuffer.spectrumAnalyser.max[x] = power;
          reusableBuffer.spectrumAnalyser.max[x+1] = power;
        }
      }
#endif

      coord_t SCALE_TOP = height() - MENU_FOOTER_HEIGHT;

      // Draw fixed part (scale,..)
      for (uint32_t frequency = ((reusableBuffer.spectrumAnalyser.freq - reusableBuffer.spectrumAnalyser.span / 2) / 10000000) * 10000000 + 10000000; ; frequency += 10000000) {
        int offset = frequency - (reusableBuffer.spectrumAnalyser.freq - reusableBuffer.spectrumAnalyser.span / 2);
        int x = offset / reusableBuffer.spectrumAnalyser.step;
        if (x >= LCD_W - 1)
          break;
        dc->drawVerticalLine(x, 0, height(), STASHED, COLOR_THEME_SECONDARY2);
      }

      for (uint8_t power = 20;; power += 20) {
        int y = SCALE_TOP - 1 - limit<int>(0, power << 1, SCALE_TOP);
        if (y <= 0)
          break;
        dc->drawHorizontalLine(0, y, width(), STASHED, COLOR_THEME_SECONDARY2);
      }

      // Draw spectrum data
      constexpr uint8_t step = 4;

      for (coord_t xv = 0; xv < width(); xv += step) {
        coord_t yv = SCALE_TOP - 1 - limit<int>(0, getAverage(step, &reusableBuffer.spectrumAnalyser.bars[xv]) << 1, SCALE_TOP);
        coord_t max_yv = SCALE_TOP - 1 - limit<int>(0, getAverage(step, &reusableBuffer.spectrumAnalyser.max[xv]) << 1, SCALE_TOP);

        // Signal bar
        dc->drawSolidFilledRect(xv, yv, step - 1, SCALE_TOP - yv, COLOR_THEME_ACTIVE);
        // lcdDrawSolidRect(xv, yv, step - 1, SCALE_TOP - yv, 1, TEXT_COLOR);

        // Signal max
        dc->drawSolidHorizontalLine(xv, max_yv, step - 1, COLOR_THEME_PRIMARY1);

        // Decay max values
        if (max_yv < yv) { // Those value are INVERTED (MENU_FOOTER_TOP - value)
          for (uint8_t i = 0; i < step; i++) {
            reusableBuffer.spectrumAnalyser.max[xv + i] = max<int>(0, reusableBuffer.spectrumAnalyser.max[xv + i] - 1);
          }
        }
      }

      // Draw tracker
      int offset = reusableBuffer.spectrumAnalyser.track - (reusableBuffer.spectrumAnalyser.freq - reusableBuffer.spectrumAnalyser.span / 2);
      int x = limit<int>(0, offset / reusableBuffer.spectrumAnalyser.step, width() - 1);
      dc->drawSolidVerticalLine(x, 0, height(), COLOR_THEME_PRIMARY1);
    }

  protected:

};


RadioSpectrumAnalyser::RadioSpectrumAnalyser(uint8_t moduleIdx) :
  Page(ICON_RADIO_TOOLS),
  moduleIdx(moduleIdx)
{
  setCloseHandler([=]() { stop(); });
  init();
  buildHeader(&header);
  buildBody(&body);
  start();
}

void RadioSpectrumAnalyser::buildHeader(Window * window)
{
  new StaticText(window, {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + 10, LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT}, STR_MENU_SPECTRUM_ANALYSER, 0, COLOR_THEME_SECONDARY1);
  }

void RadioSpectrumAnalyser::buildBody(FormWindow * window)
{
  new SpectrumWindow(window, {0, 0, LCD_W, SPECTRUM_HEIGHT});
  new SpectrumScaleWindow(window, {0, SPECTRUM_HEIGHT, LCD_W, SCALE_HEIGHT});
  new SpectrumFooterWindow(window, {0, SPECTRUM_HEIGHT + SCALE_HEIGHT, LCD_W, window->height() - SPECTRUM_HEIGHT - SCALE_HEIGHT}, moduleIdx);
}

void RadioSpectrumAnalyser::init()
{
#if defined(INTERNAL_MODULE_MULTI)
  if (moduleIdx == INTERNAL_MODULE &&
      g_model.moduleData[INTERNAL_MODULE].type == MODULE_TYPE_NONE) {
    reusableBuffer.spectrumAnalyser.moduleOFF = true;
    // this needs to be set BEFORE the module is set to prevent proto list scanning
    moduleState[moduleIdx].mode = MODULE_MODE_SPECTRUM_ANALYSER;
    setModuleType(INTERNAL_MODULE, MODULE_TYPE_MULTIMODULE);
  } else {
    reusableBuffer.spectrumAnalyser.moduleOFF = false;
  }
#endif

  if (isModuleR9MAccess(moduleIdx)) {
    reusableBuffer.spectrumAnalyser.spanDefault = 20;
    reusableBuffer.spectrumAnalyser.spanMax = 40;
    reusableBuffer.spectrumAnalyser.freqDefault = 890;
    reusableBuffer.spectrumAnalyser.freqMin = 850;
    reusableBuffer.spectrumAnalyser.freqMax = 930;
  }
  else {
    if (isModuleMultimodule(moduleIdx)) {
      reusableBuffer.spectrumAnalyser.spanDefault = 80;  // 80MHz
    }
    else {
      reusableBuffer.spectrumAnalyser.spanDefault = 40;  // 40MHz
    }
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
}

void RadioSpectrumAnalyser::start()
{
  moduleState[moduleIdx].mode = MODULE_MODE_SPECTRUM_ANALYSER;
}

void RadioSpectrumAnalyser::stop()
{
  if (isModulePXX2(moduleIdx)) {
    moduleState[moduleIdx].readModuleInformation(
        &reusableBuffer.moduleSetup.pxx2.moduleInformation, PXX2_HW_INFO_TX_ID,
        PXX2_HW_INFO_TX_ID);
  } else if (isModuleMultimodule(moduleIdx)) {
    moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
    if (reusableBuffer.spectrumAnalyser.moduleOFF)
      setModuleType(INTERNAL_MODULE, MODULE_TYPE_NONE);
  }
  /* wait 1s to resume normal operation before leaving */
  //  watchdogSuspend(1000);
  //  RTOS_WAIT_MS(1000);
}
