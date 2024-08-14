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

#include "libopenui.h"
#include "edgetx.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

constexpr coord_t SCALE_HEIGHT = 15;
constexpr coord_t FOOTER_HEIGHT = 32;
constexpr coord_t SPECTRUM_HEIGHT = LCD_H - EdgeTxStyles::MENU_HEADER_HEIGHT - SCALE_HEIGHT - FOOTER_HEIGHT;

coord_t getAverage(uint8_t number, const uint8_t* value)
{
  uint16_t sum = 0;
  for (uint8_t i = 0; i < number; i++) {
    sum += value[i];
  }
  return sum / number;
}

class SpectrumFooterWindow : public Window
{
 public:
  SpectrumFooterWindow(Window* parent, const rect_t& rect, int moduleIdx) :
      Window(parent, rect)
  {
    padAll(PAD_ZERO);

    if (isModuleMultimodule(moduleIdx)) {
      char label[16];

      // Frequency
      sprintf(label, "T: %dMHz",
              int(reusableBuffer.spectrumAnalyser.freq / 1000000));
      (new StaticText(this, rect_t{PAD_TINY, 0, FLD_W, FOOTER_HEIGHT}, label))
          ->padTop(PAD_MEDIUM);

      // Span
      sprintf(label, "S: %dMHz",
              int(reusableBuffer.spectrumAnalyser.span / 1000000));
      (new StaticText(this, rect_t{PAD_TINY + FLD_W, 0, FLD_W, FOOTER_HEIGHT}, label))
          ->padTop(PAD_MEDIUM);
    } else {
      // Frequency
      auto freq = new NumberEdit(
          this, rect_t{PAD_TINY, 0, FLD_W, 0},
          reusableBuffer.spectrumAnalyser.freqMin,
          reusableBuffer.spectrumAnalyser.freqMax,
          GET_DEFAULT(reusableBuffer.spectrumAnalyser.freq / 1000000),
          SET_VALUE(reusableBuffer.spectrumAnalyser.freq, newValue * 1000000));
      freq->setSuffix("MHz");
      freq->setPrefix("F: ");

      // Span
      auto span = new NumberEdit(
          this, rect_t{PAD_TINY + FLD_W, 0, FLD_W, 0}, 1,
          reusableBuffer.spectrumAnalyser.spanMax,
          GET_DEFAULT(reusableBuffer.spectrumAnalyser.span / 1000000),
          SET_VALUE(reusableBuffer.spectrumAnalyser.span, newValue * 1000000));
      span->setSuffix("MHz");
      span->setPrefix("S: ");
    }

    // Tracker
    auto tracker = new NumberEdit(
        this, rect_t{(PAD_TINY + FLD_W) * 2, 0, FLD_W, FOOTER_HEIGHT},
        (reusableBuffer.spectrumAnalyser.freq -
         reusableBuffer.spectrumAnalyser.span / 2) /
            1000000,
        (reusableBuffer.spectrumAnalyser.freq +
         reusableBuffer.spectrumAnalyser.span / 2) /
            1000000,
        GET_DEFAULT(reusableBuffer.spectrumAnalyser.track / 1000000),
        SET_VALUE(reusableBuffer.spectrumAnalyser.track, newValue * 1000000));
    tracker->setSuffix("MHz");
    tracker->setPrefix("T: ");
    tracker->setDefault(reusableBuffer.spectrumAnalyser.freqDefault);
  }

  static constexpr coord_t FLD_W = (LCD_W - PAD_TINY * 4) / 3;
};

class SpectrumScaleWindow : public Window
{
 public:
  SpectrumScaleWindow(Window* parent, const rect_t& rect) : Window(parent, rect)
  {
    etx_solid_bg(lvobj, COLOR_THEME_SECONDARY2_INDEX);

    build();
  }

  void build()
  {
    // Draw text scale
    char s[16];
    uint32_t startFreq = reusableBuffer.spectrumAnalyser.freq -
                         reusableBuffer.spectrumAnalyser.span / 2;
    for (uint32_t frequency = (startFreq / 10000000) * 10000000 + 10000000;;
         frequency += 10000000) {
      int x = (frequency - startFreq) / reusableBuffer.spectrumAnalyser.step;
      if (x >= LCD_W - 1) break;
      formatNumberAsString(s, 16, frequency / 1000000, 16);
      new StaticText(this, {x - 16, 0, 32, SCALE_HEIGHT}, s, 
                     COLOR_THEME_PRIMARY1_INDEX, FONT(XS) | CENTERED);
    }
  }

  void checkEvents() override
  {
    if (lastFreq != reusableBuffer.spectrumAnalyser.freq ||
        lastSpan != reusableBuffer.spectrumAnalyser.span) {
      lastFreq = reusableBuffer.spectrumAnalyser.freq;
      lastSpan = reusableBuffer.spectrumAnalyser.span;
      clear();
      build();
    }
  }

 protected:
  uint32_t lastFreq = 0;
  uint32_t lastSpan = 0;
};

class SpectrumWindow : public Window
{
 public:
  SpectrumWindow(Window* parent, const rect_t& rect) :
      Window(parent, rect)
  {
    lv_style_init(&style);
    lv_style_set_line_width(&style, 3);
    lv_style_set_line_opa(&style, LV_OPA_COVER);
    lv_style_set_line_color(&style, makeLvColor(COLOR_THEME_ACTIVE));

    lv_coord_t w = width() - 1;
    for (int i = 0; i < SPECTRUM_HEIGHT / 40; i += 1) {
      lv_coord_t y = height() - 40 - (i * 40);
      hAxisPts[i * 2] = {0, y};
      hAxisPts[i * 2 + 1] = {w, y};
      auto line = lv_line_create(lvobj);
      etx_obj_add_style(line, styles->graph_dashed, LV_PART_MAIN);
      lv_line_set_points(line, &hAxisPts[i * 2], 2);
    }

    for (int i = 0; i < 8; i += 1) {
      auto line = lv_line_create(lvobj);
      etx_obj_add_style(line, styles->graph_dashed, LV_PART_MAIN);
      lv_obj_add_flag(line, LV_OBJ_FLAG_HIDDEN);
      vAxisLines[i] = line;
    }

    for (int i = 0; i < width() / 4; i += 1) {
      auto line = lv_line_create(lvobj);
      etx_obj_add_style(line, styles->div_line_black, LV_PART_MAIN);
      maxLines[i] = line;

      line = lv_line_create(lvobj);
      lv_obj_add_style(line, &style, LV_PART_MAIN);
      barLines[i] = line;
    }

    warning = new StaticText(
        this, {0, height() / 2 - 20, lv_pct(100), LV_SIZE_CONTENT},
        STR_TURN_OFF_RECEIVER, COLOR_THEME_PRIMARY1_INDEX, CENTERED | FONT(XL));
    warning->show(TELEMETRY_STREAMING());
  }

  void checkEvents() override
  {
    lv_coord_t SCALE_TOP = height();
    constexpr uint8_t step = 4;
    lv_coord_t x;
    int i;

    warning->show(TELEMETRY_STREAMING());

    if (TELEMETRY_STREAMING()) return;

#if defined(SIMU)
    // Geneate random data for simu
    for (coord_t x = 0; x < width() - 1; x++) {
      uint8_t power = rand() % 80;
      reusableBuffer.spectrumAnalyser.bars[x] = power;
      reusableBuffer.spectrumAnalyser.bars[x + 1] = power;
      if (power > reusableBuffer.spectrumAnalyser.max[x]) {
        reusableBuffer.spectrumAnalyser.max[x] = power;
        reusableBuffer.spectrumAnalyser.max[x + 1] = power;
      }
    }
#endif

    for (x = 0, i = 0; x < width(); x += step, i += 2) {
      lv_coord_t yv =
          SCALE_TOP - 1 -
          limit<int>(0,
                     getAverage(step, &reusableBuffer.spectrumAnalyser.bars[x])
                         << 1,
                     SCALE_TOP);
      lv_coord_t max_yv =
          SCALE_TOP - 1 -
          limit<int>(
              0, getAverage(step, &reusableBuffer.spectrumAnalyser.max[x]) << 1,
              SCALE_TOP);

      maxPts[i] = {x, max_yv};
      maxPts[i + 1] = {(lv_coord_t)(x + 3), max_yv};
      lv_line_set_points(maxLines[i / 2], &maxPts[i], 2);

      barPts[i] = {(lv_coord_t)(x + 1), yv};
      barPts[i + 1] = {(lv_coord_t)(x + 1), SCALE_TOP};
      lv_line_set_points(barLines[i / 2], &barPts[i], 2);

      // Decay max values
      if (max_yv < yv) {
        for (uint8_t n = 0; n < step; n += 1) {
          reusableBuffer.spectrumAnalyser.max[x + n] =
              max<int>(0, reusableBuffer.spectrumAnalyser.max[x + n] - 1);
        }
      }
    }

    if (lastFreq != reusableBuffer.spectrumAnalyser.freq ||
        lastSpan != reusableBuffer.spectrumAnalyser.span) {
      lastFreq = reusableBuffer.spectrumAnalyser.freq;
      lastSpan = reusableBuffer.spectrumAnalyser.span;

      int i = 0;
      lv_coord_t h = height() - 1;
      uint32_t startFreq = reusableBuffer.spectrumAnalyser.freq -
                           reusableBuffer.spectrumAnalyser.span / 2;
      for (uint32_t frequency = (startFreq / 10000000) * 10000000 + 10000000;;
           frequency += 10000000) {
        lv_coord_t x = (frequency - startFreq) / reusableBuffer.spectrumAnalyser.step;
        if (x >= LCD_W - 1) break;
        vAxisPts[i * 2] = {x, 0};
        vAxisPts[i * 2 + 1] = {x, h};
        lv_line_set_points(vAxisLines[i], &vAxisPts[i * 2], 2);
        lv_obj_clear_flag(vAxisLines[i], LV_OBJ_FLAG_HIDDEN);
        i += 1;
      }
      for (; i < 8; i += 1) {
        lv_obj_add_flag(vAxisLines[i], LV_OBJ_FLAG_HIDDEN);
      }
    }

    Window::checkEvents();
  }

 protected:
  lv_style_t style;
  lv_point_t maxPts[2 * LCD_W / 4];
  lv_point_t barPts[2 * LCD_W / 4];
  lv_point_t hAxisPts[2 * SPECTRUM_HEIGHT / 40];
  lv_point_t vAxisPts[2 * 8];
  lv_obj_t* maxLines[LCD_W / 4];
  lv_obj_t* barLines[LCD_W / 4];
  lv_obj_t* vAxisLines[8];
  StaticText* warning = nullptr;

  uint32_t lastFreq = 0;
  uint32_t lastSpan = 0;
};

RadioSpectrumAnalyser::RadioSpectrumAnalyser(uint8_t moduleIdx) :
    Page(ICON_RADIO_TOOLS, PAD_ZERO), moduleIdx(moduleIdx)
{
  setCloseHandler([=]() { stop(); });
  init();
  buildHeader(header);
  buildBody(body);
  start();

  trackLine = lv_line_create(lvobj);
  etx_obj_add_style(trackLine, styles->div_line_black, LV_PART_MAIN);
}

void RadioSpectrumAnalyser::buildHeader(Window* window)
{
  header->setTitle(STR_MENUTOOLS);
  header->setTitle2(STR_MENU_SPECTRUM_ANALYSER);
}

void RadioSpectrumAnalyser::buildBody(Window* window)
{
  new SpectrumWindow(window, {0, 0, LCD_W, SPECTRUM_HEIGHT});
  new SpectrumScaleWindow(window, {0, SPECTRUM_HEIGHT, LCD_W, SCALE_HEIGHT});
  new SpectrumFooterWindow(
      window, {0, SPECTRUM_HEIGHT + SCALE_HEIGHT, LCD_W, FOOTER_HEIGHT},
      moduleIdx);
}

void RadioSpectrumAnalyser::checkEvents()
{
  int32_t offset = reusableBuffer.spectrumAnalyser.track -
                   (reusableBuffer.spectrumAnalyser.freq -
                    reusableBuffer.spectrumAnalyser.span / 2);
  if (offset < 0) offset = 0;
  int16_t x =
      limit<int>(0, offset / reusableBuffer.spectrumAnalyser.step, width() - 1);
  if (x != trackX) {
    trackX = x;
    trackPts[0] = {(lv_coord_t)x, EdgeTxStyles::MENU_HEADER_HEIGHT};
    trackPts[1] = {(lv_coord_t)x, (lv_coord_t)(height() - FOOTER_HEIGHT)};
    lv_line_set_points(trackLine, trackPts, 2);
  }
  Page::checkEvents();
}

void RadioSpectrumAnalyser::init()
{
  memclear(&reusableBuffer.spectrumAnalyser,
           sizeof(reusableBuffer.spectrumAnalyser));

#if defined(INTERNAL_MODULE_MULTI)
  if (moduleIdx == INTERNAL_MODULE &&
      g_model.moduleData[INTERNAL_MODULE].type == MODULE_TYPE_NONE) {
    reusableBuffer.spectrumAnalyser.moduleOFF = true;
    // this needs to be set BEFORE the module is set to prevent proto list
    // scanning
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
  } else {
    if (isModuleMultimodule(moduleIdx)) {
      reusableBuffer.spectrumAnalyser.spanDefault = 80;  // 80MHz
    } else {
      reusableBuffer.spectrumAnalyser.spanDefault = 40;  // 40MHz
    }
    reusableBuffer.spectrumAnalyser.spanMax = 80;
    reusableBuffer.spectrumAnalyser.freqDefault = 2440;  // 2440MHz
    reusableBuffer.spectrumAnalyser.freqMin = 2400;
    reusableBuffer.spectrumAnalyser.freqMax = 2485;
  }

  reusableBuffer.spectrumAnalyser.span =
      reusableBuffer.spectrumAnalyser.spanDefault * 1000000;
  reusableBuffer.spectrumAnalyser.freq =
      reusableBuffer.spectrumAnalyser.freqDefault * 1000000;
  reusableBuffer.spectrumAnalyser.track = reusableBuffer.spectrumAnalyser.freq;
  reusableBuffer.spectrumAnalyser.step =
      reusableBuffer.spectrumAnalyser.span / LCD_W;
  reusableBuffer.spectrumAnalyser.dirty = true;
}

void RadioSpectrumAnalyser::start()
{
  moduleState[moduleIdx].mode = MODULE_MODE_SPECTRUM_ANALYSER;
}

void RadioSpectrumAnalyser::stop()
{
#if defined(PXX2)
  if (isModulePXX2(moduleIdx)) {
    moduleState[moduleIdx].readModuleInformation(
        &reusableBuffer.moduleSetup.pxx2.moduleInformation, PXX2_HW_INFO_TX_ID,
        PXX2_HW_INFO_TX_ID);
  }
#endif

#if defined(MULTIMODULE)
  if (isModuleMultimodule(moduleIdx)) {
    moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
    if (reusableBuffer.spectrumAnalyser.moduleOFF)
      setModuleType(INTERNAL_MODULE, MODULE_TYPE_NONE);
  }
#endif
}
