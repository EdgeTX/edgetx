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

#pragma once

#include "window.h"
#include "button.h"
#include "slider.h"
#include "static.h"
#include <vector>

enum HRSColor
{
  HRS_COLOR_MUTED = 0,
  HRS_COLOR_ELEC_BLUE = LV_STATE_USER_1,
  HRS_COLOR_NEON_GREEN = LV_STATE_USER_2,
  HRS_COLOR_WHITE = LV_STATE_USER_2 | LV_STATE_USER_1,
  HRS_COLOR_WARN = LV_STATE_USER_3,
  HRS_COLOR_DANGER = LV_STATE_USER_3 | LV_STATE_USER_1,
};

class HRSButton : public TextButton
{
 public:
  HRSButton(Window* parent, const rect_t& rect, std::string text,
             std::function<uint8_t(void)> pressHandler = nullptr);

 protected:
};

class HRSText : public StaticText
{
 public:
  HRSText(Window *parent, const rect_t &rect, std::string text = "",
             HRSColor color = HRS_COLOR_MUTED, LcdFlags textFlags = 0);

  void setColor(HRSColor color);

 protected:
};

class HRSSlider : public Slider
{
 public:
  HRSSlider(Window* parent, coord_t x, coord_t y, coord_t width, int32_t vmin, int32_t vmax,
         std::function<int()> getValue, std::function<void(int)> setValue);

  void setRange(int32_t min, int32_t max);
  void setValue(int32_t v);

  static LAYOUT_VAL_SCALED(KNOB_PAD, 7)
  static LAYOUT_VAL_SCALED(KNOB_TRACK_H, 16)

 protected:
};

class HRSTool : public NavWindow
{
 public:
  HRSTool();

  void setColor(lv_obj_t*, HRSColor color);

  void onCancel() override { deleteLater(); }
#if defined(HARDWARE_KEYS)
  void onLongPressRTN() override { onCancel(); }
#endif

  static inline lv_color_t colNeonGreen() { return lv_color_hex(0x39ff14); }
  static inline lv_color_t colElecBlue() { return lv_color_hex(0x00b4ff); }
  static inline lv_color_t colRed() { return lv_color_hex(0xff2200); }
  static inline lv_color_t colDashBg() { return lv_color_hex(0x0a100e); }
  static inline lv_color_t colCardBg() { return lv_color_hex(0x121a17); }
  static inline lv_color_t colCardBorder() { return lv_color_hex(0x2a3d34); }
  static inline lv_color_t colMuted() { return lv_color_hex(0x8aa898); }
  static inline lv_color_t colWarn() { return lv_color_hex(0xffcc00); }
  static inline lv_color_t colDanger() { return lv_color_hex(0xff3355); }
  static inline lv_color_t colWhite() { return lv_color_hex(0xffffff); }
  static inline lv_color_t colBtnIdle() { return lv_color_hex(0x1a2822); }
  static inline lv_color_t colKnobTrack() { return lv_color_hex(0x1e2e28); }
  static inline lv_color_t colKnobFill() { return lv_color_hex(0x00c8ff); }
  static inline lv_color_t colBarTrack() { return lv_color_hex(0x1e2e28); }

  static LAYOUT_VAL_SCALED(HDR_H, 30)
  static LAYOUT_VAL_SCALED(FOOTER_H, 24)
  static constexpr coord_t MAIN_TOP = PAD_MEDIUM + HDR_H;
  static constexpr coord_t LEFT_X = PAD_MEDIUM;
  static LAYOUT_VAL_SCALED(LEFT_W, 150)
  static constexpr coord_t RIGHT_X = LEFT_X + LEFT_W + PAD_MEDIUM;
  static constexpr coord_t RIGHT_W = LCD_W - RIGHT_X - PAD_MEDIUM;

 protected:

  void buildHeader(const char* title);

  Window* createPanel(const rect_t& rect, bool scrollable = false, int pad = PAD_TINY);
  Window* createPanel(Window* parent, const rect_t& rect, bool scrollable = false, int pad = PAD_TINY);
  Window* leftPanel(coord_t width, bool scrollable = false);
  Window* footerPanel();
};

/** Full-screen red warning when EXT signal-entry voltage is unsafe (>5 V). */
void hrsShowExtPortUnsafeWarning(void);

class HRSMeasureTool : public HRSTool
{
 public:
  HRSMeasureTool() : HRSTool() {}

  void waveformPanel(int channels, const char* title, const char* status);

  /** Scope polyline capacity (full CHANNELS frame + edges). */
  static constexpr int WAVE_MAX_PTS = 360;
  /** Horizontal zoom: canvas is WAVE_ZOOM × viewport; swipe to pan. */
  static constexpr int WAVE_ZOOM = 4;

  static LAYOUT_VAL_SCALED(WAVE_H, 52)
  static LAYOUT_VAL_SCALED(WAVE_TITLE_H, 14)
  static LAYOUT_VAL_SCALED(CH_ROW_H, 16)
  static LAYOUT_VAL_SCALED(CH_BAR_H, 8)
  static LAYOUT_VAL_SCALED(CH_BAR_X, 40)
  static LAYOUT_VAL_SCALED(CH_VAL_W, 58)
  static LAYOUT_VAL_SCALED(STATUS_H, 28)

 protected:
  std::vector<HRSText*> lblChName;
  std::vector<HRSText*> lblChVal;
  std::vector<lv_obj_t*> chBarBg;
  std::vector<lv_obj_t*> chBarFill;

  lv_obj_t* waveBg = nullptr;      // zoomed canvas
  lv_obj_t* waveLine = nullptr;
  lv_point_t waveMidPts[2] = {};
  lv_point_t wavePts[WAVE_MAX_PTS] = {};
  uint16_t wavePtCount = 0;
  coord_t waveViewW = 0;
  coord_t waveCanvasW = 0;

  HRSText* lblStatus = nullptr;
};
