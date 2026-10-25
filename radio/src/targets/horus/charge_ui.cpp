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

#if defined(V16_CHARGE_UI)

#include "csd203_sensor.h"
#include "board.h"
#include "hal.h"
#include "hal/gpio.h"
#include "hal/module_port.h"
#include "stm32_gpio.h"

#include "edgetx.h"
#include "keys.h"
#include "etx_lv_theme.h"
#include "fonts.h"
#include "mainwindow.h"
#include "static.h"
#include "button.h"
#include "LvglWrapper.h"
#include "startup_shutdown.h"
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

static bool chargeCableConnected()
{
#if defined(SIMU)
  return usbChargerLed();
#else
  return gpio_read(USB_USBDet_GPIO) != 0;
#endif
}

/** Raw charger STAT pin (active low); pulses during the CV phase on V16. */
static bool chargerStatActive()
{
#if defined(SIMU)
  return usbChargerLed();
#else
  return !gpio_read(USB_CHARGER_GPIO);
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
/** Pack "full" for %: ~8.35V (incl. display rounding) -> 99% after +2; >= this -> 100%. */
constexpr uint16_t TX2S_FULL_MV = 8417;

/** 480x272 sci-fi power HUD: black / forest / neon green (#39FF14 tone) */
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

/** RTN hint in the bottom margin, below the current strip */
constexpr coord_t HINT_Y = IBUS_BAR_Y + IBUS_BAR_H + 2;
constexpr coord_t HINT_H = LCD_H - HINT_Y - 4;

/** Touch close / power off button, top-right corner */
constexpr coord_t CLOSE_BTN_SZ = 40;
constexpr coord_t CLOSE_BTN_X = LCD_W - CLOSE_BTN_SZ - 12;
constexpr coord_t CLOSE_BTN_Y = 12;

constexpr int32_t IBUS_ABS_MAX_MA = 3500;

/** Monitor update period (10ms ticks) */
constexpr tmr10ms_t MON_PERIOD = 5;
/** Battery current that confirms the charger has started (mA) */
constexpr int16_t CHARGE_MIN_MA = 50;
/** Give up waiting for charge current after this long (10ms ticks) */
constexpr tmr10ms_t DETECT_TIMEOUT = 500;
/** STAT off this long ends a charge; it pulses during CV (10ms ticks) */
constexpr tmr10ms_t STAT_OFF_TIMEOUT = 150;
/** STAT on this long restarts a charge from Full/NoCharge (10ms ticks) */
constexpr tmr10ms_t STAT_ON_STABLE = 15;
/** Runtime HUD stays up this long showing Full/NoCharge (10ms ticks) */
constexpr tmr10ms_t RESULT_SHOW_TIME = 300;
/** Charger may not start at all on a near-full pack (full pack under radio
 *  load reads ~8.2V) */
constexpr uint16_t TX2S_NEAR_FULL_MV = 8100;
/** STAT does not pulse during CV on V16, so detect it from the readings.
 *  CV starts ~8.31V at ~1A and reads 8.31-8.36V while tapering (IR drop). */
constexpr uint16_t TX2S_CV_MV = 8300;
/** Lower bound for the taper test, for higher currents with more IR drop */
constexpr uint16_t TX2S_CV_MIN_MV = 8250;
/** Current below this % of the session peak means CV taper */
constexpr int32_t TAPER_PCT = 70;
/** CV condition must hold this long before entering Topping (10ms ticks) */
constexpr tmr10ms_t TAPER_STABLE = 200;
/** Keep showing 100% when Full while the pack stays above this; radio load
 *  pulls it to ~8.2V after termination */
constexpr uint16_t TX2S_FULL_HOLD_MV = 8000;

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

enum class ChargePhase : uint8_t {
  Idle,       // no USB power
  Detecting,  // USB power present, waiting for the charger to start
  Charging,
  Topping,    // STAT pulsing: CV phase
  Full,
  NoCharge,   // USB power present but the charger never started
};

/**
 * Charger phase and filtered battery readings. Outlives the dialog so the
 * runtime and shutdown HUDs share one state; ticked from customUIActions()
 * while running and from the shutdown loop.
 */
class ChargeMonitor
{
 public:
  void update()
  {
    const tmr10ms_t now = get_tmr10ms();
    if (updated_ && (tmr10ms_t)(now - lastUpdate_) < MON_PERIOD) {
      return;
    }
    updated_ = true;
    lastUpdate_ = now;

    vmv_ = batterySystemVoltageMv();
    ima_ = batterySystemCurrentMa();
    updateSoc();

    const bool stat = chargerStatActive();
    if (stat != statOn_) {
      statOn_ = stat;
      statSince_ = now;
    }
    const tmr10ms_t statFor = now - statSince_;

    if (!chargeCableConnected()) {
      setPhase(ChargePhase::Idle);
      return;
    }

    switch (phase_) {
      case ChargePhase::Idle:
        setPhase(ChargePhase::Detecting);
        break;

      case ChargePhase::Detecting:
        if (stat && ima_ >= CHARGE_MIN_MA) {
          setPhase(ChargePhase::Charging);
        } else if (timeInPhase() >= DETECT_TIMEOUT) {
          /* STAT on without net charge current: load exceeds charge rate */
          if (stat) {
            setPhase(ChargePhase::Charging);
          } else if (vmv_ >= TX2S_NEAR_FULL_MV) {
            setPhase(ChargePhase::Full);
          } else {
            setPhase(ChargePhase::NoCharge);
          }
        }
        break;

      case ChargePhase::Charging:
      case ChargePhase::Topping:
        if (!stat) {
          if (statFor >= STAT_OFF_TIMEOUT) {
            setPhase(ChargePhase::Full);
          } else {
            setPhase(ChargePhase::Topping);
          }
        } else if (phase_ == ChargePhase::Charging) {
          updateTaper(now);
        }
        break;

      case ChargePhase::Full:
      case ChargePhase::NoCharge:
        if (stat && statFor >= STAT_ON_STABLE) {
          setPhase(ChargePhase::Charging);
        }
        break;
    }
  }

  ChargePhase phase() const { return phase_; }
  tmr10ms_t timeInPhase() const { return get_tmr10ms() - phaseSince_; }
  uint16_t voltageMv() const { return vmv_; }
  int16_t currentMa() const { return ima_; }
  uint8_t soc() const
  {
    // Voltage alone reads ~99% well before termination and sags after it
    if (phase_ == ChargePhase::Full && vmv_ >= TX2S_FULL_HOLD_MV) {
      return 100;
    }
    if ((phase_ == ChargePhase::Detecting ||
         phase_ == ChargePhase::Charging ||
         phase_ == ChargePhase::Topping) &&
        socDisplay_ > 99) {
      return 99;
    }
    return socDisplay_;
  }

 private:
  ChargePhase phase_ = ChargePhase::Idle;
  tmr10ms_t phaseSince_ = 0;
  bool updated_ = false;
  tmr10ms_t lastUpdate_ = 0;
  bool statOn_ = false;
  tmr10ms_t statSince_ = 0;
  uint16_t vmv_ = 0;
  int16_t ima_ = 0;
  bool socFilterInit_ = false;
  uint16_t socDisplayX100_ = 0;
  uint8_t socDisplay_ = 0;
  int16_t peakMa_ = 0;
  tmr10ms_t taperSince_ = 0;

  void setPhase(ChargePhase phase)
  {
    if (phase != phase_) {
      phase_ = phase;
      phaseSince_ = get_tmr10ms();
      if (phase == ChargePhase::Charging) {
        peakMa_ = 0;
        taperSince_ = phaseSince_;
      }
    }
  }

  void updateTaper(tmr10ms_t now)
  {
    if (ima_ > peakMa_) {
      peakMa_ = ima_;
    }
    // Voltage alone also covers a near-full pack that starts straight in CV
    const bool tapering =
        vmv_ >= TX2S_CV_MV ||
        (vmv_ >= TX2S_CV_MIN_MV &&
         static_cast<int32_t>(ima_) * 100 <
             static_cast<int32_t>(peakMa_) * TAPER_PCT);
    if (!tapering) {
      taperSince_ = now;
    } else if ((tmr10ms_t)(now - taperSince_) >= TAPER_STABLE) {
      setPhase(ChargePhase::Topping);
    }
  }

  void updateSoc()
  {
    uint8_t rawSoc = estimateSoc2s(vmv_);
    if (!socFilterInit_) {
      socDisplayX100_ = static_cast<uint16_t>(rawSoc) * 100u;
      socDisplay_ = rawSoc;
      socFilterInit_ = true;
    } else {
      /* IIR ~1/20: stable % against pack voltage ripple from charge current */
      uint32_t blend = static_cast<uint32_t>(socDisplayX100_) * 19u +
                       static_cast<uint32_t>(rawSoc) * 100u;
      socDisplayX100_ = static_cast<uint16_t>((blend + 10u) / 20u);
      socDisplay_ = static_cast<uint8_t>((socDisplayX100_ + 50u) / 100u);
    }
    if (rawSoc >= 100) {
      socDisplay_ = 100;
    } else if (rawSoc == 0) {
      socDisplay_ = 0;
    } else if (rawSoc >= 88 && rawSoc > socDisplay_) {
      /* IIR is slow upward near full; voltage text updates while % lags (e.g. 8.3x V vs 92%). */
      socDisplay_ = rawSoc;
    }
  }
};

static ChargeMonitor s_monitor;

static const char* phaseLabel(ChargePhase phase)
{
  switch (phase) {
    case ChargePhase::Detecting:
      return STR_CHARGE_DETECTING;
    case ChargePhase::Charging:
      return STR_BATTERYCHARGING;
    case ChargePhase::Topping:
      return STR_CHARGE_TOPPING_OFF;
    case ChargePhase::Full:
      return STR_BATTERYFULL;
    case ChargePhase::NoCharge:
      return STR_CHARGE_NOT_CHARGING;
    default:
      return "";
  }
}

static bool phaseIsCharging(ChargePhase phase)
{
  return phase == ChargePhase::Charging || phase == ChargePhase::Topping;
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
                       "",
                       COLOR_THEME_PRIMARY2_INDEX, CENTERED | FONT(XS));
    lockNoScroll(lblStatus->getLvObj());
    lv_obj_set_style_text_color(lblStatus->getLvObj(), colTextDim(),
                                LV_PART_MAIN);
    lv_label_set_long_mode(lblStatus->getLvObj(), LV_LABEL_LONG_DOT);

    auto lblHint = new StaticText(
        this, {MARGIN_H, HINT_Y, LCD_W - 2 * MARGIN_H, HINT_H},
        shutdownHold_ ? STR_PRESS_RTN_TO_POWER_OFF : STR_PRESS_RTN_TO_CLOSE,
        COLOR_THEME_PRIMARY2_INDEX, CENTERED | FONT(XS));
    lockNoScroll(lblHint->getLvObj());
    lv_obj_set_style_text_color(lblHint->getLvObj(), colTextDim(),
                                LV_PART_MAIN);
    lv_label_set_long_mode(lblHint->getLvObj(), LV_LABEL_LONG_DOT);

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

    /* Same action as RTN; the symbols need no translation */
    auto btnClose = new TextButton(
        this, {CLOSE_BTN_X, CLOSE_BTN_Y, CLOSE_BTN_SZ, CLOSE_BTN_SZ},
        shutdownHold_ ? LV_SYMBOL_POWER : LV_SYMBOL_CLOSE, [=]() -> uint8_t {
          onCancel();
          return 0;
        });
    lv_obj_t* btnObj = btnClose->getLvObj();
    lv_group_remove_obj(btnObj);  // touch only, keep keys on RTN
    lv_obj_set_style_radius(btnObj, CLOSE_BTN_SZ / 2, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btnObj, colEmptyCell(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btnObj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(btnObj, colNeonOutline(), LV_PART_MAIN);
    lv_obj_set_style_border_width(btnObj, 2, LV_PART_MAIN);
    lv_obj_set_style_text_color(btnObj, colNeonOutline(), LV_PART_MAIN);
    lv_obj_move_foreground(btnObj);

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
  uint16_t lastVmV = 0xFFFF;
  int16_t lastImA = INT16_MIN;
  ChargePhase lastPhase = ChargePhase::Idle;
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

  void updateAnimations(ChargePhase phase, int16_t ima, uint8_t soc)
  {
    layoutFromSoc(soc);

    uint32_t t = static_cast<uint32_t>(get_tmr10ms());
    lv_opa_t pulse = static_cast<lv_opa_t>(
        LV_OPA_20 + (uint8_t)((t % 18u) * (LV_OPA_60 - LV_OPA_20) / 17u));
    lv_obj_set_style_shadow_opa(batOutline,
                                phaseIsCharging(phase) ? pulse : LV_OPA_30,
                                LV_PART_MAIN);

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
    /* Neutral while detecting: discharge before the charger ramps is expected */
    lv_color_t ibusCol = phase == ChargePhase::Detecting ? colTextDim()
                         : ima >= 0                      ? colFillBright()
                                                         : colWarnLo();
    lv_obj_set_style_bg_color(ibusBarFill, ibusCol, LV_PART_MAIN);
  }

  void refreshUi(bool forceTxt)
  {
    uint16_t vmv = s_monitor.voltageMv();
    int16_t ima = s_monitor.currentMa();
    ChargePhase phase = s_monitor.phase();
    uint8_t soc = s_monitor.soc();

    updateAnimations(phase, ima, soc);

    bool dataStale = forceTxt || vmv != lastVmV || ima != lastImA ||
                     phase != lastPhase || soc != lastPaintedSoc_;
    if (!dataStale) {
      return;
    }
    lastVmV = vmv;
    lastImA = ima;
    lastPhase = phase;

    char line[128];
    snprintf(line, sizeof(line), "%u%%", static_cast<unsigned>(soc));
    lblSocBig->setText(line);

    lv_color_t pctCol = soc < 18    ? colWarnLo()
                        : soc < 45 ? colWarnHi()
                                   : colNeonOutline();
    lv_obj_set_style_text_color(lblSocBig->getLvObj(), pctCol, LV_PART_MAIN);
    lastPaintedSoc_ = soc;

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

    snprintf(line, sizeof(line),
             "%s  %u.%02u V | %s%u.%02u A | %s%u.%02u W",
             phaseLabel(phase),
             static_cast<unsigned>(vc / 100u), static_cast<unsigned>(vc % 100u),
             negi ? "-" : "+",
             static_cast<unsigned>(imaAbs / 1000u),
             static_cast<unsigned>((imaAbs % 1000u) / 10u), negp ? "-" : "+", pw, pwf);
    lblStatus->setText(line);
  }

  void checkEvents() override
  {
    Window::checkEvents();
    refreshUi(false);
  }

  void onCancel() override
  {
    if (shutdownHold_) {
      s_shutdownChargeForceQuit = true;
      return;
    }
    closeWindow();
  }
};

ChargeDashboardDialog* s_chargeDlg = nullptr;

static void chargeUiTaskImpl()
{
  if (s_shutdownChargeBlocking) {
    return;
  }
  static ChargePhase prevPhase = ChargePhase::Idle;

  if (s_chargeDlg && s_chargeDlg->deleted()) {
    s_chargeDlg = nullptr;
  }

  s_monitor.update();
  const ChargePhase phase = s_monitor.phase();

  // Show once per plug-in; a dismissed HUD stays closed until the next one
  if (!s_chargeDlg && prevPhase == ChargePhase::Idle &&
      phase == ChargePhase::Detecting) {
    s_chargeDlg = new ChargeDashboardDialog();
    s_chargeDlg->onClosing([]() { s_chargeDlg = nullptr; });
  }
  prevPhase = phase;

  if (s_chargeDlg &&
      (phase == ChargePhase::Idle ||
       ((phase == ChargePhase::Full || phase == ChargePhase::NoCharge) &&
        s_monitor.timeInPhase() >= RESULT_SHOW_TIME))) {
    s_chargeDlg->closeWindow();
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
  if (!chargeCableConnected()) {
    return;
  }

  shutdownChargeModulesOff();

  // edgeTxClose() sets suspendI2CTasks; re-enable for live charge telemetry.
  suspendI2CTasks = false;

  s_shutdownChargeForceQuit = false;
  s_shutdownChargeBlocking = true;

  // edgeTxClose() has already removed the sleep bitmap and every window via
  // MainWindow::shutdown(). Create the dialog before any LVGL refresh so the
  // empty MainWindow background is never drawn. CSD203 reads were suspended
  // during edgeTxClose(), so refresh them first (no LVGL, sleep bitmap stays).
  for (int i = 0; i < 10; ++i) {
    WDG_RESET();
    pollCsd203Sensors();
    sleep_ms(5);
  }
  s_monitor.update();
  s_chargeDlg = new ChargeDashboardDialog(true);
  s_chargeDlg->onClosing([]() { s_chargeDlg = nullptr; });

  while (chargeCableConnected() && !s_shutdownChargeForceQuit) {
    WDG_RESET();
    pollShutdownChargeRtnQuit();
    pollCsd203Sensors();
    s_monitor.update();
    if (s_chargeDlg) {
      s_chargeDlg->pumpShutdownUi();
    }
    LvglWrapper::instance()->run();
    MainWindow::instance()->run();
    sleep_ms(10);
    checkBacklight();
  }

  // Cover the charge UI with the sleep screen (drawn on top and refreshed
  // immediately) rather than exposing the empty MainWindow background.
  drawSleepBitmap();

  if (s_chargeDlg) {
    s_chargeDlg->closeWindow();
    s_chargeDlg = nullptr;
  }

  s_shutdownChargeBlocking = false;
  s_shutdownChargeForceQuit = false;
}

}  // namespace charge_ui_detail

// Radio specific UI actions (overrides weak default in main.cpp)
void customUIActions()
{
  charge_ui_detail::chargeUiTaskImpl();
}

// Radio specific shutdown actions (overrides weak default in tasks.cpp):
// if charging at power-down, drive charge UI until it clears.
void customShutdownActions()
{
  charge_ui_detail::shutdownChargeWait();
}

#endif
