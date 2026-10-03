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

#include "hrs_tool.h"

#include "edgetx.h"
#include "ext_port_safety.h"
#include "mainwindow.h"

#include <stdio.h>

class ExtPortUnsafeWarnDialog : public HRSTool
{
 public:
  ExtPortUnsafeWarnDialog() : HRSTool()
  {
    // Full-screen danger backdrop
    lv_obj_set_style_bg_color(lvobj, lv_color_hex(0x2a0008), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lvobj, LV_OPA_COVER, LV_PART_MAIN);

    auto* topBar = new Window(this, {0, 0, LCD_W, EdgeTxStyles::MENU_HEADER_HEIGHT});
    lv_obj_set_style_bg_color(topBar->getLvObj(), colDanger(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(topBar->getLvObj(), LV_OPA_COVER, LV_PART_MAIN);

    new HRSText(topBar, {0, PAD_LARGE, LCD_W, 0}, "WARNING", HRS_COLOR_WHITE,
                CENTERED | FONT(L));

    new HRSText(this, {PAD_LARGE, EdgeTxStyles::UI_ELEMENT_HEIGHT * 2, LCD_W - PAD_LARGE * 2, 0},
                "EXT signal unsafe", HRS_COLOR_DANGER, CENTERED | FONT(L));

    new HRSText(this, {PAD_LARGE, EdgeTxStyles::UI_ELEMENT_HEIGHT * 3 + PAD_SMALL,
                       LCD_W - PAD_LARGE * 2, 0},
                "Test blocked", HRS_COLOR_DANGER, CENTERED | FONT(STD));

    char detail[80];
    const uint16_t mv = v15ExtPortLastVoltageMv();
    if (mv > 0) {
      snprintf(detail, sizeof(detail), "S2 = %u.%02u V  (> 5.00 V)",
               (unsigned)(mv / 1000u), (unsigned)((mv % 1000u) / 10u));
    } else {
      snprintf(detail, sizeof(detail), "%s", v15ExtPortBlockedMessage());
    }
    new HRSText(this, {PAD_LARGE, EdgeTxStyles::UI_ELEMENT_HEIGHT * 5,
                       LCD_W - PAD_LARGE * 2, 0},
                detail, HRS_COLOR_WARN, CENTERED | FONT(STD));

    new HRSText(this, {PAD_LARGE, EdgeTxStyles::UI_ELEMENT_HEIGHT * 7,
                       LCD_W - PAD_LARGE * 2, 0},
                "Disconnect high voltage\nSIG must be <= 5V",
                HRS_COLOR_WHITE, CENTERED | FONT(STD));

    LAYOUT_VAL_SCALED(BTN_W, 160)
    LAYOUT_VAL_SCALED(BTN_H, 40)

    auto* ok = new HRSButton(
        this, {(LCD_W - BTN_W) / 2, LCD_H - BTN_H - PAD_LARGE * 3, BTN_W, BTN_H}, "OK",
        [=]() {
          onCancel();
          return 0;
        });
    // Emphasize confirm as red danger action
    if (ok && ok->getLvObj()) {
      lv_obj_set_style_bg_color(ok->getLvObj(), colDanger(), LV_PART_MAIN);
      lv_obj_set_style_border_color(ok->getLvObj(), colWhite(), LV_PART_MAIN);
      lv_obj_set_style_text_color(ok->getLvObj(), colWhite(), LV_PART_MAIN);
    }
  }
};

void hrsShowExtPortUnsafeWarning(void)
{
  new ExtPortUnsafeWarnDialog();
}

HRSButton::HRSButton(Window* parent, const rect_t& rect, std::string text,
                     std::function<uint8_t(void)> pressHandler) :
  TextButton(parent, rect, text, pressHandler)
{
  lv_obj_set_style_bg_color(lvobj, HRSTool::colBtnIdle(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(lvobj, HRSTool::colCardBorder(), LV_PART_MAIN);
  lv_obj_set_style_border_color(lvobj, HRSTool::colNeonGreen(), LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_style_border_width(lvobj, PAD_BORDER, LV_PART_MAIN);
  lv_obj_set_style_radius(lvobj, PAD_MEDIUM, LV_PART_MAIN);
  lv_obj_set_style_text_color(lvobj, HRSTool::colMuted(), LV_PART_MAIN);
}

HRSText::HRSText(Window *parent, const rect_t &rect, std::string text, HRSColor color, LcdFlags textFlags) :
            StaticText(parent, rect, text, COLOR_WHITE_INDEX, textFlags)
{
  lv_obj_set_style_text_color(lvobj, HRSTool::colMuted(), LV_PART_MAIN);
  lv_obj_set_style_text_color(lvobj, HRSTool::colElecBlue(), LV_PART_MAIN | HRS_COLOR_ELEC_BLUE);
  lv_obj_set_style_text_color(lvobj, HRSTool::colNeonGreen(), LV_PART_MAIN | HRS_COLOR_NEON_GREEN);
  lv_obj_set_style_text_color(lvobj, HRSTool::colWhite(), LV_PART_MAIN | HRS_COLOR_WHITE);
  lv_obj_set_style_text_color(lvobj, HRSTool::colWarn(), LV_PART_MAIN | HRS_COLOR_WARN);
  lv_obj_set_style_text_color(lvobj, HRSTool::colDanger(), LV_PART_MAIN | HRS_COLOR_DANGER);
  
  setColor(color);
}

void HRSText::setColor(HRSColor color)
{
  lv_obj_clear_state(lvobj, LV_STATE_USER_1 | LV_STATE_USER_2 | LV_STATE_USER_3 | LV_STATE_USER_4);
  if (color != HRS_COLOR_MUTED)
    lv_obj_add_state(lvobj, color);
}

HRSSlider::HRSSlider(Window* parent, coord_t x, coord_t y, coord_t width, int32_t vmin, int32_t vmax,
        std::function<int()> getValue, std::function<void(int)> setValue) :
  Slider(parent, width, vmin, vmax, getValue, setValue)
{
  setPos(x, y);

  // Adjust to prevent clipping
  lv_obj_set_x(slider, PAD_LARGE);
  lv_obj_set_size(slider, width - PAD_LARGE * 4, KNOB_TRACK_H);

  // Track
  etx_remove_bg_color(slider, LV_PART_MAIN | LV_STATE_FOCUSED);
  etx_remove_bg_color(slider, LV_PART_MAIN | LV_STATE_FOCUSED | LV_STATE_EDITED);
  lv_obj_set_style_bg_color(slider, HRSTool::colKnobTrack(), LV_PART_MAIN);
  lv_obj_set_style_radius(slider, KNOB_TRACK_H / 2, LV_PART_MAIN);
  lv_obj_set_style_border_width(slider, 1, LV_PART_MAIN);
  lv_obj_set_style_border_color(slider, HRSTool::colElecBlue(), LV_PART_MAIN);
  lv_obj_set_style_pad_all(slider, 0, LV_PART_MAIN);

  // Fill
  etx_remove_bg_color(slider, LV_PART_INDICATOR | LV_STATE_FOCUSED);
  etx_remove_bg_color(slider, LV_PART_INDICATOR | LV_STATE_FOCUSED | LV_STATE_EDITED);
  lv_obj_set_style_bg_color(slider, HRSTool::colKnobFill(), LV_PART_INDICATOR);
  lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, LV_PART_INDICATOR);
  lv_obj_set_style_radius(slider, KNOB_TRACK_H / 2, LV_PART_INDICATOR);

  // Handle — compact round knob
  etx_remove_bg_color(slider, LV_PART_MAIN);
  etx_remove_border_color(slider, LV_PART_KNOB | LV_STATE_FOCUSED);
  lv_obj_set_style_bg_color(slider, HRSTool::colNeonGreen(), LV_PART_KNOB);
  etx_bg_color(slider, COLOR_THEME_ACTIVE_INDEX, LV_PART_KNOB | LV_STATE_FOCUSED);
  etx_bg_color(slider, COLOR_THEME_EDIT_INDEX, LV_PART_KNOB | LV_STATE_FOCUSED | LV_STATE_EDITED);
  lv_obj_set_style_border_width(slider, PAD_BORDER, LV_PART_KNOB);
  lv_obj_set_style_border_color(slider, HRSTool::colWhite(), LV_PART_KNOB);
  lv_obj_set_style_radius(slider, LV_RADIUS_CIRCLE, LV_PART_KNOB);
  lv_obj_set_style_pad_all(slider, KNOB_PAD, LV_PART_KNOB);
}

void HRSSlider::setRange(int32_t min, int32_t max)
{
  vmin = min;
  vmax = max;
  lv_slider_set_range(slider, vmin, vmax);
}

void HRSSlider::setValue(int32_t v)
{
  lv_slider_set_value(slider, v, LV_ANIM_OFF);
}

HRSTool::HRSTool() : NavWindow(MainWindow::instance(), {0, 0, LCD_W, LCD_H})
{
  setWindowFlag(OPAQUE);
  pushLayer();

  lv_obj_set_style_bg_color(lvobj, colDashBg(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_COVER, LV_PART_MAIN);
}

void HRSTool::setColor(lv_obj_t* obj, HRSColor color)
{
  lv_obj_clear_state(obj, LV_STATE_USER_1 | LV_STATE_USER_2 | LV_STATE_USER_3 | LV_STATE_USER_4);
  if (color != HRS_COLOR_MUTED)
    lv_obj_add_state(obj, color);
}

void HRSTool::buildHeader(const char* title)
{
  auto* hdr = new Window(this, {0, PAD_MEDIUM, LCD_W, HDR_H});
  hdr->padLeft(PAD_MEDIUM);
  hdr->padRight(PAD_MEDIUM);

  new HRSText(hdr, {0, PAD_TINY, 0, 0}, "//>", HRS_COLOR_ELEC_BLUE, FONT(STD));
  new HRSText(hdr, {0, 0, LV_PCT(100), 0}, title, HRS_COLOR_ELEC_BLUE, CENTERED | FONT(L));
  new HRSText(hdr, {0, PAD_TINY, LV_PCT(100), 0}, "<//", HRS_COLOR_ELEC_BLUE, RIGHT | FONT(STD));
}

Window* HRSTool::createPanel(Window* parent, const rect_t& rect, bool scrollable, int pad)
{
  Window* w = new Window(parent, rect);

  auto obj = w->getLvObj();
  if (!scrollable)
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(obj, colCardBg(), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_border_color(obj, colCardBorder(), LV_PART_MAIN);
  lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
  lv_obj_set_style_radius(obj, PAD_SMALL, LV_PART_MAIN);
  lv_obj_set_style_pad_all(obj, pad, LV_PART_MAIN);

  return w;
}

Window* HRSTool::createPanel(const rect_t& rect, bool scrollable, int pad)
{
  return createPanel(this, rect, scrollable, pad);
}

Window* HRSTool::leftPanel(coord_t width, bool scrollable)
{
  auto w = createPanel({LEFT_X, MAIN_TOP, width, LCD_H - MAIN_TOP - FOOTER_H - PAD_MEDIUM}, scrollable);
  w->padLeft(PAD_SMALL);
  return w;
}

Window* HRSTool::footerPanel()
{
  auto w = createPanel(this, {PAD_MEDIUM, LCD_H - FOOTER_H - PAD_TINY,
                       LCD_W - PAD_MEDIUM * 2, FOOTER_H});
  w->padLeft(PAD_SMALL);
  w->padRight(PAD_SMALL);
  return w;
}

void HRSMeasureTool::waveformPanel(int channels, const char* title, const char* status)
{
  const coord_t h = LCD_H - MAIN_TOP - FOOTER_H - PAD_MEDIUM;
  auto rightCard = createPanel({RIGHT_X, MAIN_TOP, RIGHT_W, h});

  static LAYOUT_VAL_SCALED(H1, 14)

  coord_t y = PAD_TINY;

  new HRSText(rightCard, {0, y, 0, 0}, title, HRS_COLOR_ELEC_BLUE, FONT(XS));
  y += WAVE_TITLE_H;

  waveViewW = RIGHT_W - PAD_SMALL * 2;
  waveCanvasW = (waveViewW - 2) * WAVE_ZOOM;

  // Viewport: finger-swipe horizontally to pan the zoomed wave
  auto wave = createPanel(rightCard, {PAD_TINY, y, waveViewW, WAVE_H}, true, 0);
  auto waveScroll = wave->getLvObj();
  lv_obj_set_scroll_dir(waveScroll, LV_DIR_HOR);
  lv_obj_set_scrollbar_mode(waveScroll, LV_SCROLLBAR_MODE_AUTO);
  lv_obj_add_flag(waveScroll, LV_OBJ_FLAG_SCROLL_MOMENTUM);

  // Zoomed canvas (4× viewport width)
  waveBg = lv_obj_create(waveScroll);
  lv_obj_set_size(waveBg, waveCanvasW, WAVE_H - 2);
  lv_obj_set_pos(waveBg, 0, 0);

  waveMidPts[0] = {0, WAVE_H / 2};
  waveMidPts[1] = {waveCanvasW - 1, WAVE_H / 2};
  auto waveMidLine = lv_line_create(waveBg);
  lv_obj_set_style_line_width(waveMidLine, 1, LV_PART_MAIN);
  lv_obj_set_style_line_color(waveMidLine, colCardBorder(), LV_PART_MAIN);
  lv_line_set_points(waveMidLine, waveMidPts, 2);

  waveLine = lv_line_create(waveBg);
  lv_obj_set_style_line_width(waveLine, PAD_TINY, LV_PART_MAIN);
  lv_obj_set_style_line_color(waveLine, colMuted(), LV_PART_MAIN | HRS_COLOR_MUTED);
  lv_obj_set_style_line_color(waveLine, colNeonGreen(), LV_PART_MAIN | HRS_COLOR_NEON_GREEN);
  lv_obj_set_style_line_color(waveLine, colDanger(), LV_PART_MAIN | HRS_COLOR_DANGER);
  lv_obj_set_style_line_color(waveLine, colWarn(), LV_PART_MAIN | HRS_COLOR_WARN);
  lv_obj_set_style_line_rounded(waveLine, false, LV_PART_MAIN);
  wavePts[0] = {0, (lv_coord_t)(WAVE_H / 2)};
  wavePts[1] = {(lv_coord_t)(waveCanvasW - 1), (lv_coord_t)(WAVE_H / 2)};
  wavePtCount = 2;
  lv_line_set_points(waveLine, wavePts, wavePtCount);

  y += WAVE_H + PAD_SMALL;

  new HRSText(rightCard, {PAD_SMALL, y, 0, 0}, "Channels (us)  — swipe", HRS_COLOR_ELEC_BLUE, FONT(XS));
  y += H1;

  static LAYOUT_VAL_SCALED(MAX_CHLIST_H, 40)

  const coord_t statusY = h - STATUS_H - PAD_TINY;
  const coord_t listH = statusY - y - PAD_TINY;
  auto chList = new Window(rightCard, {0, y, RIGHT_W - PAD_LARGE, listH > MAX_CHLIST_H ? listH : MAX_CHLIST_H});
  lv_obj_t* listObj = chList->getLvObj();
  lv_obj_add_flag(listObj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_scroll_dir(listObj, LV_DIR_VER);
  lv_obj_set_scrollbar_mode(listObj, LV_SCROLLBAR_MODE_AUTO);
  lv_obj_add_flag(listObj, LV_OBJ_FLAG_SCROLL_MOMENTUM);

  const coord_t barW = RIGHT_W - CH_BAR_X - CH_VAL_W - PAD_SMALL * 2;
  coord_t cy = PAD_TINY;
  for (int i = 0; i < channels; ++i) {
    char name[16];
    snprintf(name, sizeof(name), "CH%d", i + 1);
    auto lbl = new HRSText(chList, {PAD_SMALL, cy - PAD_TINY, 0, 0}, name, HRS_COLOR_MUTED, FONT(XS));
    lblChName.push_back(lbl);

    lbl = new HRSText(chList, {0, cy - PAD_TINY, LV_PCT(100), 0}, "----", HRS_COLOR_MUTED, RIGHT | FONT(XS));
    lblChVal.push_back(lbl);

    auto bg = lv_obj_create(listObj);
    lv_obj_set_size(bg, barW, CH_BAR_H);
    lv_obj_set_pos(bg, CH_BAR_X, cy + PAD_TINY);
    lv_obj_set_style_radius(bg, PAD_TINY, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bg, colBarTrack(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, LV_PART_MAIN);
    chBarBg.push_back(bg);

    auto fill = lv_obj_create(bg);
    lv_obj_set_size(fill, 0, CH_BAR_H);
    lv_obj_set_pos(fill, 0, 0);
    lv_obj_set_style_radius(fill, PAD_TINY, LV_PART_MAIN);
    lv_obj_set_style_bg_color(fill, colNeonGreen(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(fill, LV_OPA_COVER, LV_PART_MAIN);
    chBarFill.push_back(fill);

    cy += CH_ROW_H;
  }

  lblStatus = new HRSText(rightCard, {PAD_SMALL, statusY, 0, 0}, status, HRS_COLOR_MUTED, FONT(XS));
}
