/*
 * Copyright (C) EdgeTX
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "charge_ui.h"

#if defined(V16_CHARGE_UI)

#include "csd203_sensor.h"
#include "board.h"
#include "hal/module_port.h"

#include "edgetx.h"
#include "keys.h"
#include "etx_lv_theme.h"
#include "fonts.h"
#include "mainwindow.h"
#include "static.h"
#include "LvglWrapper.h"
#include "os/sleep.h"
#include "hal/watchdog_driver.h"

#include <lvgl/lvgl.h>

#include <stdint.h>
#include <stdio.h>

extern bool suspendI2CTasks;

namespace charge_ui_detail {

static void pollCsd203Sensors()
{
#if !defined(SIMU)
  for (int i = 0; i < 3; i++) {
    readCSD203();
  }
#endif
}

static uint16_t batterySystemVoltageMv()
{
  return getBatteryVoltage() * 10u;
}

static int16_t batterySystemCurrentMa()
{
  return -getSystemCurrent();
}

/** Stop RF modules and cut module power before shutdown charge UI */
static void shutdownChargeModulesOff()
{
#if !defined(SIMU)
  pulsesStopModule(INTERNAL_MODULE);
  pulsesStopModule(EXTERNAL_MODULE);
#if defined(HARDWARE_INTERNAL_MODULE)
  modulePortSetPower(INTERNAL_MODULE, false);
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
  modulePortSetPower(EXTERNAL_MODULE, false);
#endif
  sleep_ms(200);
#endif
}

constexpr uint16_t TX2S_EMPTY_MV = 6000;
/** Pack 鈥渇ull鈥?for %: ~8.35V (incl. display rounding) 鈫?99% after +2; >= this 鈫?100%. */
constexpr uint16_t TX2S_FULL_MV = 8417;

/** 480x272 鈥?sci鈥慺i power HUD: black / forest / neon green (#39FF14 tone) */
constexpr coord_t MARGIN_H = 24;

constexpr coord_t BAT_BODY_W = 352;
constexpr coord_t BAT_BODY_H = 96;
constexpr coord_t BAT_CAP_W = 14;
constexpr coord_t BAT_GAP = 6;
constexpr coord_t BAT_TOTAL_W = BAT_BODY_W + BAT_GAP + BAT_CAP_W;
constexpr coord_t BAT_X = (LCD_W - BAT_TOTAL_W) / 2;

constexpr coord_t SOC_Y = 20;
constexpr coord_t SOC_LABEL_H = 48;

constexpr coord_t REGION_GAP = 10;

constexpr coord_t BAT_Y = SOC_Y + SOC_LABEL_H + REGION_GAP;

constexpr coord_t FILL_INSET = 8;

constexpr coord_t SOC_BAR_Y = BAT_Y + BAT_BODY_H + REGION_GAP;
constexpr coord_t SOC_BAR_H = 8;
constexpr coord_t SOC_BAR_W = BAT_BODY_W;
constexpr coord_t SOC_BAR_X = BAT_X;

constexpr coord_t IBUS_BAR_H = 6;
/** Keep status line clear of bottom current strip */
constexpr coord_t IBUS_BAR_Y = LCD_H - MARGIN_H - IBUS_BAR_H - 6;

constexpr coord_t STATUS_Y = SOC_BAR_Y + SOC_BAR_H + 8;
constexpr coord_t STATUS_H = IBUS_BAR_Y - STATUS_Y - 6;

constexpr int32_t IBUS_ABS_MAX_MA = 3500;

/** Dismiss charge UI only after this many consecutive inactive samples (~50ms each). */
constexpr uint8_t CHG_UI_OFF_TICKS = 40;

bool s_holdOffChargeUiUntilUnplug = false;

static bool s_shutdownChargeBlocking = false;
static bool s_shutdownChargeForceQuit = false;

/** HUD palette: black / forest / neon green (~#39FF14), cyberpunk power style */
static inline lv_color_t colBg()
{
  return lv_color_hex(0x000000);
}
static inline lv_color_t colBgGlow()
{
  return lv_color_hex(0x061208);
}
static inline lv_color_t colText()
{
  return lv_color_hex(0xe8ffe8);
}
static inline lv_color_t colTextDim()
{
  return lv_color_hex(0x7a9a82);
}
/** Primary neon outline / glow */
static inline lv_color_t colNeonOutline()
{
  return lv_color_hex(0x39ff14);
}
/** Gradient deep (forest) inside fills */
static inline lv_color_t colFillDeep()
{
  return lv_color_hex(0x084218);
}
/** Gradient bright peak */
static inline lv_color_t colFillBright()
{
  return lv_color_hex(0x5dff3a);
}
/** Empty battery interior */
static inline lv_color_t colEmptyCell()
{
  return lv_color_hex(0x0a140c);
}
/** Dim track (segment trench) */
static inline lv_color_t colTrackPurple()
{
  return lv_color_hex(0x16281a);
}
static inline lv_color_t colWarnLo()
{
  return lv_color_hex(0xff6b35);
}
static inline lv_color_t colWarnHi()
{
  return lv_color_hex(0xcfff4a);
}

/** 2S linear SOC + 1% display lift; smoothing applied in dialog (socDisplay_) */
static uint8_t estimateSoc2s(uint16_t totalMv)
{
  if (totalMv <= TX2S_EMPTY_MV) {
    return 0;
  }
  if (totalMv >= TX2S_FULL_MV) {
    return 100;
  }
  uint8_t raw = static_cast<uint8_t>((totalMv - TX2S_EMPTY_MV) * 100u /
                                     (TX2S_FULL_MV - TX2S_EMPTY_MV));
  if (raw == 0) {
    return 0;
  }
  uint16_t lifted = static_cast<uint16_t>(raw) + 2u;
  if (lifted >= 100) {
    return 100;
  }
  return static_cast<uint8_t>(lifted);
}

static void lockNoScroll(lv_obj_t* obj)
{
  lv_obj_clear_flag(obj,
                    LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_SCROLL_CHAIN_HOR |
                        LV_OBJ_FLAG_SCROLL_CHAIN_VER | LV_OBJ_FLAG_GESTURE_BUBBLE |
                        LV_OBJ_FLAG_SCROLL_MOMENTUM | LV_OBJ_FLAG_SCROLL_ELASTIC);
  lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_scroll_dir(obj, LV_DIR_NONE);
}

class ChargeDashboardDialog : public Window
{
 public:
  explicit ChargeDashboardDialog(bool shutdownHold = false) :
      Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H}),
      shutdownHold_(shutdownHold)
  {
    setWindowFlag(OPAQUE | NO_FORCED_SCROLL);
    lockNoScroll(lvobj);

    lv_obj_set_style_bg_grad_dir(lvobj, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(lvobj, colBg(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(lvobj, colBgGlow(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lvobj, LV_OPA_COVER, LV_PART_MAIN);

    pushLayer();
    lv_obj_move_foreground(lvobj);

    lblSocBig = new StaticText(this, {0, SOC_Y, LCD_W, SOC_LABEL_H}, "0%",
                               COLOR_THEME_PRIMARY2_INDEX,
                               CENTERED | FONT(XXL));
    lockNoScroll(lblSocBig->getLvObj());
    lv_obj_set_style_text_color(lblSocBig->getLvObj(), colNeonOutline(),
                                LV_PART_MAIN);
    lv_obj_set_style_text_opa(lblSocBig->getLvObj(), LV_OPA_COVER, LV_PART_MAIN);

    batOutline = lv_obj_create(lvobj);
    lv_obj_set_pos(batOutline, BAT_X, BAT_Y);
    lv_obj_set_size(batOutline, BAT_BODY_W, BAT_BODY_H);
    lv_obj_set_style_radius(batOutline, 16, LV_PART_MAIN);
    lv_obj_set_style_pad_all(batOutline, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(batOutline, colEmptyCell(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(batOutline, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(batOutline, colNeonOutline(), LV_PART_MAIN);
    lv_obj_set_style_border_width(batOutline, 2, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(batOutline, colNeonOutline(), LV_PART_MAIN);
    lv_obj_set_style_shadow_width(batOutline, 14, LV_PART_MAIN);
    lv_obj_set_style_shadow_spread(batOutline, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(batOutline, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(batOutline, true, LV_PART_MAIN);
    lv_obj_clear_flag(batOutline, LV_OBJ_FLAG_SCROLLABLE);
    lockNoScroll(batOutline);

    batFill = lv_obj_create(batOutline);
    lv_obj_remove_style_all(batFill);
    lv_obj_set_size(batFill, 8, BAT_BODY_H - 2 * FILL_INSET);
    lv_obj_align(batFill, LV_ALIGN_LEFT_MID, FILL_INSET, 0);
    lv_obj_set_style_radius(batFill, 11, LV_PART_MAIN);
    lv_obj_set_style_border_width(batFill, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(batFill, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(batFill, LV_GRAD_DIR_HOR, LV_PART_MAIN);
    lv_obj_set_style_bg_color(batFill, colFillDeep(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(batFill, colFillBright(), LV_PART_MAIN);
    lv_obj_clear_flag(batFill, LV_OBJ_FLAG_SCROLLABLE);
    lockNoScroll(batFill);

    batCap = lv_obj_create(lvobj);
    lv_obj_set_pos(batCap, BAT_X + BAT_BODY_W + BAT_GAP,
                   BAT_Y + (BAT_BODY_H - 44) / 2);
    lv_obj_set_size(batCap, BAT_CAP_W, 44);
    lv_obj_set_style_radius(batCap, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(batCap, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_color(batCap, colNeonOutline(), LV_PART_MAIN);
    lv_obj_set_style_border_width(batCap, 2, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(batCap, colNeonOutline(), LV_PART_MAIN);
    lv_obj_set_style_shadow_width(batCap, 10, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(batCap, LV_OPA_30, LV_PART_MAIN);
    lv_obj_clear_flag(batCap, LV_OBJ_FLAG_SCROLLABLE);
    lockNoScroll(batCap);

    lblBolt = lv_label_create(lvobj);
    lv_label_set_text(lblBolt, LV_SYMBOL_CHARGE);
    etx_font(lblBolt, FONT_XL_INDEX, LV_PART_MAIN);
    lv_obj_set_style_text_color(lblBolt, colNeonOutline(), LV_PART_MAIN);
    lv_obj_set_style_text_opa(lblBolt, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_pos(lblBolt, BAT_X + BAT_BODY_W / 2 - 16,
                   BAT_Y + BAT_BODY_H / 2 - 16);
    lv_obj_clear_flag(lblBolt, LV_OBJ_FLAG_SCROLLABLE);
    lockNoScroll(lblBolt);

    socBarBg = lv_obj_create(lvobj);
    lv_obj_set_pos(socBarBg, SOC_BAR_X, SOC_BAR_Y);
    lv_obj_set_size(socBarBg, SOC_BAR_W, SOC_BAR_H);
    lv_obj_set_style_pad_all(socBarBg, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(socBarBg, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(socBarBg, colTrackPurple(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(socBarBg, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(socBarBg, 0, LV_PART_MAIN);
    lv_obj_clear_flag(socBarBg, LV_OBJ_FLAG_SCROLLABLE);
    lockNoScroll(socBarBg);

    socBarFill = lv_obj_create(socBarBg);
    lv_obj_remove_style_all(socBarFill);
    lv_obj_set_size(socBarFill, 6, SOC_BAR_H - 4);
    lv_obj_align(socBarFill, LV_ALIGN_LEFT_MID, 2, 0);
    lv_obj_set_style_radius(socBarFill, 5, LV_PART_MAIN);
    lv_obj_set_style_border_width(socBarFill, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(socBarFill, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(socBarFill, LV_GRAD_DIR_HOR, LV_PART_MAIN);
    lv_obj_set_style_bg_color(socBarFill, colFillBright(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(socBarFill, colFillDeep(), LV_PART_MAIN);
    lv_obj_clear_flag(socBarFill, LV_OBJ_FLAG_SCROLLABLE);
    lockNoScroll(socBarFill);

    lblStatus =
        new StaticText(this,
                       {MARGIN_H, STATUS_Y, LCD_W - 2 * MARGIN_H, STATUS_H},
                       "Charging ...",
                       COLOR_THEME_PRIMARY2_INDEX, CENTERED | FONT(XS));
    lockNoScroll(lblStatus->getLvObj());
    lv_obj_set_style_text_color(lblStatus->getLvObj(), colTextDim(),
                                LV_PART_MAIN);
    lv_label_set_long_mode(lblStatus->getLvObj(), LV_LABEL_LONG_DOT);

    ibusBarBg = lv_obj_create(lvobj);
    lv_obj_set_pos(ibusBarBg, MARGIN_H, IBUS_BAR_Y);
    lv_obj_set_size(ibusBarBg, LCD_W - 2 * MARGIN_H, IBUS_BAR_H);
    lv_obj_set_style_radius(ibusBarBg, 4, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ibusBarBg, colTrackPurple(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ibusBarBg, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(ibusBarBg, 0, LV_PART_MAIN);
    lv_obj_clear_flag(ibusBarBg, LV_OBJ_FLAG_SCROLLABLE);
    lockNoScroll(ibusBarBg);

    ibusBarFill = lv_obj_create(ibusBarBg);
    lv_obj_remove_style_all(ibusBarFill);
    lv_obj_set_size(ibusBarFill, 4, IBUS_BAR_H - 2);
    lv_obj_align(ibusBarFill, LV_ALIGN_LEFT_MID, 1, 0);
    lv_obj_set_style_radius(ibusBarFill, 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ibusBarFill, colFillBright(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ibusBarFill, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(ibusBarFill, 0, LV_PART_MAIN);
    lv_obj_clear_flag(ibusBarFill, LV_OBJ_FLAG_SCROLLABLE);
    lockNoScroll(ibusBarFill);

    lv_obj_move_foreground(lblBolt);
    lv_obj_move_foreground(lblSocBig->getLvObj());

    refreshUi(true);
  }

  /** Menus task has left perMain(); pump this so gauges/animations keep updating */
  void pumpShutdownUi() { checkEvents(); }

 protected:
  lv_obj_t* batOutline = nullptr;
  lv_obj_t* batFill = nullptr;
  lv_obj_t* batCap = nullptr;
  lv_obj_t* lblBolt = nullptr;
  lv_obj_t* socBarBg = nullptr;
  lv_obj_t* socBarFill = nullptr;
  lv_obj_t* ibusBarBg = nullptr;
  lv_obj_t* ibusBarFill = nullptr;
  StaticText* lblSocBig = nullptr;
  StaticText* lblStatus = nullptr;

  const bool shutdownHold_;
  uint8_t ledOffTicks = 0;
  uint16_t lastVmV = 0xFFFF;
  int16_t lastImA = INT16_MIN;
  bool lastChgLed = false;
  bool socFilterInit_ = false;
  uint8_t socDisplay_ = 0;
  uint8_t lastPaintedSoc_ = 255;

  void layoutFromSoc(uint8_t soc)
  {
    lv_obj_update_layout(batOutline);
    lv_obj_update_layout(socBarBg);

    lv_coord_t innerW = lv_obj_get_content_width(batOutline) - 2 * FILL_INSET;
    lv_coord_t innerH = lv_obj_get_content_height(batOutline) - 2 * FILL_INSET;
    if (innerW < 8) {
      innerW = BAT_BODY_W - 2 * FILL_INSET - 4;
    }
    if (innerH < 8) {
      innerH = BAT_BODY_H - 2 * FILL_INSET - 4;
    }

    lv_coord_t fw =
        static_cast<lv_coord_t>((innerW * static_cast<uint32_t>(soc)) / 100u);
    if (soc > 0 && fw < 10) {
      fw = 10;
    }
    if (soc == 0) {
      lv_obj_add_flag(batFill, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_clear_flag(batFill, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_size(batFill, fw, innerH);
      lv_obj_align(batFill, LV_ALIGN_LEFT_MID, FILL_INSET, 0);
      lv_obj_set_style_bg_opa(batFill, LV_OPA_COVER, LV_PART_MAIN);
    }

    lv_coord_t sbw = lv_obj_get_content_width(socBarBg) - 4;
    if (sbw < 8) {
      sbw = SOC_BAR_W - 4;
    }
    lv_coord_t sw =
        static_cast<lv_coord_t>((sbw * static_cast<uint32_t>(soc)) / 100u);
    if (soc > 0 && sw < 4) {
      sw = 4;
    }
    if (soc == 0) {
      lv_obj_add_flag(socBarFill, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_clear_flag(socBarFill, LV_OBJ_FLAG_HIDDEN);
      lv_obj_set_width(socBarFill, sw);
      lv_obj_set_style_bg_opa(socBarFill, LV_OPA_COVER, LV_PART_MAIN);
    }

    lv_color_t deep = soc < 18    ? colWarnLo()
                      : soc < 45 ? colWarnHi()
                                 : colFillDeep();
    lv_color_t bright = soc < 18    ? colWarnHi()
                        : soc < 45 ? colFillBright()
                                   : colFillBright();
    lv_obj_set_style_bg_color(batFill, deep, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(batFill, bright, LV_PART_MAIN);
    lv_obj_set_style_bg_color(socBarFill, bright, LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(socBarFill, deep, LV_PART_MAIN);
  }

  void updateAnimations(bool chgLed, int16_t ima, uint8_t soc)
  {
    layoutFromSoc(soc);

    uint32_t t = static_cast<uint32_t>(get_tmr10ms());
    lv_opa_t pulse = static_cast<lv_opa_t>(
        LV_OPA_20 + (uint8_t)((t % 18u) * (LV_OPA_60 - LV_OPA_20) / 17u));
    lv_obj_set_style_shadow_opa(batOutline,
                                chgLed ? pulse : LV_OPA_30, LV_PART_MAIN);

    lv_coord_t iw = lv_obj_get_width(ibusBarBg) - 3;
    int32_t a = ima >= 0 ? ima : -ima;
    if (a > IBUS_ABS_MAX_MA) {
      a = IBUS_ABS_MAX_MA;
    }
    lv_coord_t ifw =
        static_cast<lv_coord_t>((iw * static_cast<int32_t>(a)) / IBUS_ABS_MAX_MA);
    if (ifw < 3) {
      ifw = (ima != 0) ? 3 : 1;
    }
    lv_obj_set_width(ibusBarFill, ifw);
    lv_obj_set_style_bg_color(
        ibusBarFill, ima >= 0 ? colFillBright() : colWarnLo(), LV_PART_MAIN);
  }

  void refreshUi(bool forceTxt)
  {
    uint16_t vmv = batterySystemVoltageMv();
    int16_t ima = batterySystemCurrentMa();
    bool chgLed = usbChargerLed();

    uint8_t rawSoc = estimateSoc2s(vmv);
    if (!socFilterInit_) {
      socDisplay_ = rawSoc;
      socFilterInit_ = true;
    } else {
      /* IIR ~1/8: stable % against pack voltage ripple from charge current */
      int32_t blend = socDisplay_ * 19 + static_cast<int32_t>(rawSoc);
      socDisplay_ = static_cast<uint8_t>(blend / 20);
    }
    if (rawSoc >= 100) {
      socDisplay_ = 100;
    } else if (rawSoc == 0) {
      socDisplay_ = 0;
    } else if (rawSoc >= 88 && rawSoc > socDisplay_) {
      /* IIR is slow upward near full; voltage text updates while % lags (e.g. 8.3x V vs 92%). */
      socDisplay_ = rawSoc;
    }

    updateAnimations(chgLed, ima, socDisplay_);

    bool dataStale = forceTxt || vmv != lastVmV || ima != lastImA ||
                     chgLed != lastChgLed ||
                     socDisplay_ != lastPaintedSoc_;
    if (!dataStale) {
      return;
    }
    lastVmV = vmv;
    lastImA = ima;
    lastChgLed = chgLed;

    char line[96];
    snprintf(line, sizeof(line), "%u%%", static_cast<unsigned>(socDisplay_));
    lblSocBig->setText(line);

    lv_color_t pctCol = socDisplay_ < 18    ? colWarnLo()
                        : socDisplay_ < 45 ? colWarnHi()
                                           : colNeonOutline();
    lv_obj_set_style_text_color(lblSocBig->getLvObj(), pctCol, LV_PART_MAIN);
    lastPaintedSoc_ = socDisplay_;

    uint16_t vc = static_cast<uint16_t>((vmv + 5u) / 10u);
    int32_t im32 = static_cast<int32_t>(ima);
    bool negi = im32 < 0;
    uint32_t imaAbs =
        negi ? static_cast<uint32_t>(-im32) : static_cast<uint32_t>(im32);

    int64_t p_mw64 =
        (static_cast<int64_t>(vmv) * static_cast<int32_t>(ima)) / 1000;
    bool negp = p_mw64 < 0;
    uint32_t p_abs_mw = static_cast<uint32_t>(negp ? -p_mw64 : p_mw64);
    unsigned pw = static_cast<unsigned>(p_abs_mw / 1000u);
    unsigned pwf = static_cast<unsigned>((p_abs_mw % 1000u) / 10u);

    const char* rtnHint =
        shutdownHold_ ? "RTN: power off" : "RTN close";

    snprintf(line, sizeof(line),
             "Charging ...  %u.%02u V | %s%u.%02u A | %s%u.%02u W | %s",
             static_cast<unsigned>(vc / 100u), static_cast<unsigned>(vc % 100u),
             negi ? "-" : "+",
             static_cast<unsigned>(imaAbs / 1000u),
             static_cast<unsigned>((imaAbs % 1000u) / 10u), negp ? "-" : "+", pw, pwf,
             rtnHint);
    lblStatus->setText(line);
  }

  void holdOffUntilUnplug()
  {
    if (!shutdownHold_) {
      s_holdOffChargeUiUntilUnplug = true;
    }
  }

  void onEvent(event_t event) override
  {
    if (event == EVT_KEY_FIRST(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_EXIT)) {
      holdOffUntilUnplug();
    }
    Window::onEvent(event);
  }

  void checkEvents() override
  {
    Window::checkEvents();
    refreshUi(false);

    if (shutdownHold_) {
      ledOffTicks = 0;
      return;
    }

    if (!usbChargerLed()) {
      if (ledOffTicks < 250) {
        ++ledOffTicks;
      }
      if (ledOffTicks >= CHG_UI_OFF_TICKS) {
        deleteLater();
      }
    } else {
      ledOffTicks = 0;
    }
  }

  void onCancel() override
  {
    if (shutdownHold_) {
      s_shutdownChargeForceQuit = true;
      return;
    }
    holdOffUntilUnplug();
    deleteLater();
  }
};

ChargeDashboardDialog* s_chargeDlg = nullptr;

static void chargeUiTaskImpl()
{
  if (s_shutdownChargeBlocking) {
    return;
  }
  static uint8_t stableOn = 0;
  constexpr uint8_t CHG_LED_STABLE_TICKS = 3;

  if (s_chargeDlg && s_chargeDlg->deleted()) {
    s_chargeDlg = nullptr;
  }

  bool led = usbChargerLed();
  if (!led) {
    stableOn = 0;
    s_holdOffChargeUiUntilUnplug = false;
  } else if (stableOn < 255) {
    ++stableOn;
  }

  if (!s_chargeDlg && led && stableOn >= CHG_LED_STABLE_TICKS &&
      !s_holdOffChargeUiUntilUnplug) {
    s_chargeDlg = new ChargeDashboardDialog();
    s_chargeDlg->setCloseHandler([]() {
      if (!s_shutdownChargeBlocking) {
        s_holdOffChargeUiUntilUnplug = true;
      }
      s_chargeDlg = nullptr;
    });
  }
}

/** Handle RTN (KEY_EXIT) before LvglWrapper eats the queue; restores other events */
static void pollShutdownChargeRtnQuit()
{
  if (!isEvent()) return;

  event_t evt = getEvent();
  if (evt == EVT_KEY_BREAK(KEY_EXIT)) {
    s_shutdownChargeForceQuit = true;
    return;
  }
  pushEvent(evt);
}

void shutdownChargeWait()
{
  if (!usbChargerLed()) {
    return;
  }

  shutdownChargeModulesOff();

  // edgeTxClose() sets suspendI2CTasks; re-enable for live charge telemetry.
  suspendI2CTasks = false;

  s_shutdownChargeForceQuit = false;
  s_shutdownChargeBlocking = true;

  if (s_chargeDlg) {
    s_chargeDlg->deleteLater();
    s_chargeDlg = nullptr;
  }

  /* Let LVGL dispose any pending delete before attaching shutdown dialog */
  for (int i = 0; i < 30; ++i) {
    WDG_RESET();
    pollShutdownChargeRtnQuit();
    pollCsd203Sensors();
    LvglWrapper::instance()->run();
    MainWindow::instance()->run();
    sleep_ms(5);
    checkBacklight();
  }

  s_chargeDlg = new ChargeDashboardDialog(true);
  s_chargeDlg->setCloseHandler([]() { s_chargeDlg = nullptr; });

  while (usbChargerLed() && !s_shutdownChargeForceQuit) {
    WDG_RESET();
    pollShutdownChargeRtnQuit();
    pollCsd203Sensors();
    if (s_chargeDlg) {
      s_chargeDlg->pumpShutdownUi();
    }
    LvglWrapper::instance()->run();
    MainWindow::instance()->run();
    sleep_ms(10);
    checkBacklight();
  }

  if (s_chargeDlg) {
    s_chargeDlg->deleteLater();
    s_chargeDlg = nullptr;
    for (int i = 0; i < 20; ++i) {
      WDG_RESET();
      pollShutdownChargeRtnQuit();
      pollCsd203Sensors();
      LvglWrapper::instance()->run();
      MainWindow::instance()->run();
      sleep_ms(5);
    }
  }

  s_shutdownChargeBlocking = false;
  s_shutdownChargeForceQuit = false;
}

}  // namespace charge_ui_detail

void chargeUiTask(void)
{
  charge_ui_detail::chargeUiTaskImpl();
}

void shutdownWaitIfCharging(void)
{
  charge_ui_detail::shutdownChargeWait();
}

#endif
