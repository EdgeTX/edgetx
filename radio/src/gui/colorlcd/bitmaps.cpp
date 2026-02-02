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

#include "bitmaps.h"

#include "lz4/lz4.h"
#include "edgetx_helpers.h"

LZ4BitmapBuffer::LZ4BitmapBuffer(uint8_t format) :
    BitmapBuffer(format, 0, 0, nullptr)
{
}

LZ4BitmapBuffer::~LZ4BitmapBuffer() { free(data); }

void LZ4BitmapBuffer::load(const LZ4Bitmap* lz4Data)
{
  _width = lz4Data->width;
  _height = lz4Data->height;

  uint32_t pixels = _width * _height;
  data = (uint16_t*)malloc(align32(pixels * sizeof(uint16_t)));

  LZ4_decompress_safe((const char*)lz4Data->data, (char*)data,
                      lz4Data->compressedSize, pixels * sizeof(uint16_t));
  data_end = data + pixels;
}

MaskBitmap* _decompressed_mask(const uint8_t* lz4_compressed)
{
  const uint16_t* hdr = (const uint16_t*)lz4_compressed;
  uint16_t width = hdr[0];
  uint16_t height = hdr[1];

  size_t len = *(uint32_t*)&hdr[2];

  // skip 8 bytes header
  lz4_compressed += 8;

  uint32_t pixels = width * height;
  MaskBitmap* raw = (MaskBitmap*)malloc(align32(pixels + 4));

  raw->width = width;
  raw->height = height;

  LZ4_decompress_safe((const char*)lz4_compressed, (char*)&raw->data, len,
                      pixels);

  return raw;
}

#if !defined(BOOT)

static const uint8_t mask_menu_favs[] __FLASH = {
#include "mask_menu_favs.lbm"
};

static const uint8_t mask_menu_model[] __FLASH = {
#include "mask_menu_model.lbm"
};
static const uint8_t mask_menu_notes[] __FLASH = {
#include "mask_menu_notes.lbm"
};
static const uint8_t mask_menu_radio[] __FLASH = {
#include "mask_menu_radio.lbm"
};
static const uint8_t mask_menu_stats[] __FLASH = {
#include "mask_menu_stats.lbm"
};
static const uint8_t mask_menu_theme[] __FLASH = {
#include "mask_menu_theme.lbm"
};
static const uint8_t mask_model_curves[] __FLASH = {
#include "mask_model_curves.lbm"
};
static const uint8_t mask_model_flight_modes[] __FLASH = {
#include "mask_model_flight_modes.lbm"
};
static const uint8_t mask_model_gvars[] __FLASH = {
#include "mask_model_gvars.lbm"
};
static const uint8_t mask_model_heli[] __FLASH = {
#include "mask_model_heli.lbm"
};
static const uint8_t mask_model_inputs[] __FLASH = {
#include "mask_model_inputs.lbm"
};
static const uint8_t mask_model_logical_switches[] __FLASH = {
#include "mask_model_logical_switches.lbm"
};
static const uint8_t mask_model_lua_scripts[] __FLASH = {
#include "mask_model_lua_scripts.lbm"
};
static const uint8_t mask_model_mixer[] __FLASH = {
#include "mask_model_mixer.lbm"
};
static const uint8_t mask_model_outputs[] __FLASH = {
#include "mask_model_outputs.lbm"
};
static const uint8_t mask_model_setup[] __FLASH = {
#include "mask_model_setup.lbm"
};
static const uint8_t mask_model_special_functions[] __FLASH = {
#include "mask_model_special_functions.lbm"
};
static const uint8_t mask_model_telemetry[] __FLASH = {
#include "mask_model_telemetry.lbm"
};
static const uint8_t mask_model_usb[] __FLASH = {
#include "mask_model_usb.lbm"
};
static const uint8_t mask_menu_model_select[] __FLASH = {
#include "mask_menu_model_select.lbm"  //TODO: someone may want to make proper icon
};
static const uint8_t mask_monitor[] __FLASH = {
#include "mask_monitor.lbm"
};
static const uint8_t mask_monitor_logsw[] __FLASH = {
#include "mask_monitor_logsw.lbm"
};
static const uint8_t mask_edgetx[] __FLASH = {
#include "mask_edgetx.lbm"
};
static const uint8_t mask_radio_calibration[] __FLASH = {
#include "mask_radio_calibration.lbm"
};
static const uint8_t mask_radio_global_functions[] __FLASH = {
#include "mask_radio_global_functions.lbm"
};
static const uint8_t mask_radio_hardware[] __FLASH = {
#include "mask_radio_hardware.lbm"
};
static const uint8_t mask_radio_sd_browser[] __FLASH = {
#include "mask_radio_sd_browser.lbm"
};
static const uint8_t mask_radio_setup[] __FLASH = {
#include "mask_radio_setup.lbm"
};
static const uint8_t mask_radio_tools[] __FLASH = {
#include "mask_radio_tools.lbm"
};
static const uint8_t mask_radio_edit_theme[] __FLASH = {
#include "mask_radio_edit_theme.lbm"
};
static const uint8_t mask_radio_trainer[] __FLASH = {
#include "mask_radio_trainer.lbm"
};
static const uint8_t mask_radio_version[] __FLASH = {
#include "mask_radio_version.lbm"
};
static const uint8_t mask_stats_analogs[] __FLASH = {
#include "mask_stats_analogs.lbm"
};
static const uint8_t mask_stats_debug[] __FLASH = {
#include "mask_stats_debug.lbm"
};
static const uint8_t mask_stats_timers[] __FLASH = {
#include "mask_stats_timers.lbm"
};
static const uint8_t mask_theme_add_view[] __FLASH = {
#include "mask_theme_add_view.lbm"
};
static const uint8_t mask_theme_setup[] __FLASH = {
#include "mask_theme_setup.lbm"
};
static const uint8_t mask_theme_view1[] __FLASH = {
#include "mask_theme_view1.lbm"
};
static const uint8_t mask_theme_view2[] __FLASH = {
#include "mask_theme_view2.lbm"
};
static const uint8_t mask_theme_view3[] __FLASH = {
#include "mask_theme_view3.lbm"
};
static const uint8_t mask_theme_view4[] __FLASH = {
#include "mask_theme_view4.lbm"
};
static const uint8_t mask_theme_view5[] __FLASH = {
#include "mask_theme_view5.lbm"
};
static const uint8_t mask_theme_view6[] __FLASH = {
#include "mask_theme_view6.lbm"
};
static const uint8_t mask_theme_view7[] __FLASH = {
#include "mask_theme_view7.lbm"
};
static const uint8_t mask_theme_view8[] __FLASH = {
#include "mask_theme_view8.lbm"
};
static const uint8_t mask_theme_view9[] __FLASH = {
#include "mask_theme_view9.lbm"
};
static const uint8_t mask_theme_view10[] __FLASH = {
#include "mask_theme_view10.lbm"
};

static const uint8_t mask_chan_locked[] __FLASH = {
#include "mask_monitor_lockch.lbm"
};
static const uint8_t mask_chan_inverted[] __FLASH = {
#include "mask_monitor_inver.lbm"
};

static const uint8_t mask_shutdown_circle0[] __FLASH = {
#include "mask_shutdown_circle0.lbm"
};
static const uint8_t mask_shutdown_circle1[] __FLASH = {
#include "mask_shutdown_circle1.lbm"
};
static const uint8_t mask_shutdown_circle2[] __FLASH = {
#include "mask_shutdown_circle2.lbm"
};
static const uint8_t mask_shutdown_circle3[] __FLASH = {
#include "mask_shutdown_circle3.lbm"
};

static const uint8_t mask_shutdown[] __FLASH = {
#include "mask_shutdown.lbm"
};

const uint8_t mask_topleft_bg[] __FLASH = {
#include "mask_topleft.lbm"
};
const uint8_t mask_topright_bg[] __FLASH = {
#include "mask_topright.lbm"
};

const uint8_t mask_currentmenu_bg[] __FLASH = {
#include "mask_currentmenu_bg.lbm"
};
const uint8_t mask_currentmenu_shadow[] __FLASH = {
#include "mask_currentmenu_shadow.lbm"
};
const uint8_t mask_currentmenu_dot[] __FLASH = {
#include "mask_currentmenu_dot.lbm"
};

static const uint8_t mask_dot[] __FLASH = {
#include "mask_dot.lbm"
};

static const uint8_t mask_topmenu_usb[] __FLASH = {
#include "mask_topmenu_usb.lbm"
};
static const uint8_t mask_topmenu_vol0[] __FLASH = {
#include "mask_volume_0.lbm"
};
static const uint8_t mask_topmenu_vol1[] __FLASH = {
#include "mask_volume_1.lbm"
};
static const uint8_t mask_topmenu_vol2[] __FLASH = {
#include "mask_volume_2.lbm"
};
static const uint8_t mask_topmenu_vol3[] __FLASH = {
#include "mask_volume_3.lbm"
};
static const uint8_t mask_topmenu_vol4[] __FLASH = {
#include "mask_volume_4.lbm"
};
static const uint8_t mask_topmenu_vol_scale[] __FLASH = {
#include "mask_volume_scale.lbm"
};
static const uint8_t mask_topmenu_txbatt[] __FLASH = {
#include "mask_txbat.lbm"
};
#if defined(USB_CHARGER)
static const uint8_t mask_topmenu_txbatt_charging[] __FLASH = {
#include "mask_txbat_charging.lbm"
};
#endif
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)||defined(RADIO_V12) && defined(EXTERNAL_ANTENNA)
static const uint8_t mask_topmenu_antenna[] __FLASH = {
#include "mask_antenna.lbm"
};
#endif
#if defined(INTERNAL_GPS)
static const uint8_t mask_topmenu_gps[] __FLASH = {
#include "mask_topmenu_gps_18.lbm"
};
#endif

static const uint8_t mask_error[] __FLASH = {
#include "mask_error.lbm"
};
static const uint8_t mask_busy[] __FLASH = {
#include "mask_busy.lbm"
};

static const uint8_t mask_usb_plugged[] __FLASH = {
#include "mask_usb_symbol.lbm"
};

static const uint8_t mask_timer[] __FLASH = {
#include "mask_timer.lbm"
};
static const uint8_t mask_timer_bg[] __FLASH = {
#include "mask_timer_bg.lbm"
};

static const uint8_t mask_textline_curve[] __FLASH = {
#include "mask_textline_curve.lbm"
};
static const uint8_t mask_textline_fm[] __FLASH = {
#include "mask_textline_fm.lbm"
};

static const uint8_t mask_mplex_add[] __FLASH = {
#include "mask_mplex_add.lbm"
};
static const uint8_t mask_mplex_multi[] __FLASH = {
#include "mask_mplex_multi.lbm"
};
static const uint8_t mask_mplex_replace[] __FLASH = {
#include "mask_mplex_replace.lbm"
};

const uint8_t mask_round_title_left[] __FLASH = {
#include "mask_round_title_left.lbm"
};
const uint8_t mask_round_title_right[] __FLASH = {
#include "mask_round_title_right.lbm"
};

static const uint8_t mask_model_grid_large[] __FLASH = {
#include "mask_model_grid_large.lbm"
};
static const uint8_t mask_model_grid_small[] __FLASH = {
#include "mask_model_grid_small.lbm"
};
static const uint8_t mask_model_list_one[] __FLASH = {
#include "mask_model_list_one.lbm"
};
static const uint8_t mask_model_list_two[] __FLASH = {
#include "mask_model_list_two.lbm"
};

static const uint8_t mask_trim[] __FLASH = {
#include "mask_trim.lbm"
};
static const uint8_t mask_trim_shadow[] __FLASH = {
#include "mask_trim_shadow.lbm"
};

static const uint8_t mask_tools_apps[] __FLASH = {
#include "mask_tools_apps.lbm"
};
static const uint8_t mask_tools_reset[] __FLASH = {
#include "mask_tools_reset.lbm"
};

static const uint8_t mask_btn_close[] __FLASH = {
#include "mask_btn_close.lbm"
};
static const uint8_t mask_btn_next[] __FLASH = {
#include "mask_btn_next.lbm"
};
static const uint8_t mask_btn_prev[] __FLASH = {
#include "mask_btn_prev.lbm"
};

static const uint8_t mask_top_logo[] __FLASH = {
#include "mask_top_logo.lbm"
};

struct _BuiltinIcon {
  const uint8_t* lz4_compressed_bitmap;
};

#define BI(icon, mask) \
  {                    \
    mask               \
  }

// Note: Order must match EdgeTxIcon enum
static const _BuiltinIcon _builtinIcons[EDGETX_ICONS_COUNT] __FLASH = {
    BI(ICON_EDGETX, mask_edgetx),
    BI(ICON_QM_FAVORITES, mask_menu_favs),
    BI(ICON_RADIO, mask_menu_radio),
    BI(ICON_RADIO_SETUP, mask_radio_setup),
    BI(ICON_RADIO_SD_MANAGER, mask_radio_sd_browser),
    BI(ICON_RADIO_TOOLS, mask_radio_tools),
    BI(ICON_RADIO_GLOBAL_FUNCTIONS, mask_radio_global_functions),
    BI(ICON_RADIO_TRAINER, mask_radio_trainer),
    BI(ICON_RADIO_HARDWARE, mask_radio_hardware),
    BI(ICON_RADIO_CALIBRATION, mask_radio_calibration),
    BI(ICON_RADIO_EDIT_THEME, mask_radio_edit_theme),
    BI(ICON_RADIO_VERSION, mask_radio_version),
    BI(ICON_MODEL, mask_menu_model),
    BI(ICON_MODEL_SETUP, mask_model_setup),
    BI(ICON_MODEL_HELI, mask_model_heli),
    BI(ICON_MODEL_FLIGHT_MODES, mask_model_flight_modes),
    BI(ICON_MODEL_INPUTS, mask_model_inputs),
    BI(ICON_MODEL_MIXER, mask_model_mixer),
    BI(ICON_MODEL_NOTES, mask_menu_notes),
    BI(ICON_MODEL_OUTPUTS, mask_model_outputs),
    BI(ICON_MODEL_CURVES, mask_model_curves),
    BI(ICON_MODEL_GVARS, mask_model_gvars),
    BI(ICON_MODEL_LOGICAL_SWITCHES, mask_model_logical_switches),
    BI(ICON_MODEL_SPECIAL_FUNCTIONS, mask_model_special_functions),
    BI(ICON_MODEL_LUA_SCRIPTS, mask_model_lua_scripts),
    BI(ICON_MODEL_TELEMETRY, mask_model_telemetry),
    BI(ICON_MODEL_USB, mask_model_usb),
    BI(ICON_MODEL_SELECT, mask_menu_model_select),
    BI(ICON_THEME, mask_menu_theme),
    BI(ICON_THEME_SETUP, mask_theme_setup),
    BI(ICON_THEME_VIEW1, mask_theme_view1),
    BI(ICON_THEME_VIEW2, mask_theme_view2),
    BI(ICON_THEME_VIEW3, mask_theme_view3),
    BI(ICON_THEME_VIEW4, mask_theme_view4),
    BI(ICON_THEME_VIEW5, mask_theme_view5),
    BI(ICON_THEME_VIEW6, mask_theme_view6),
    BI(ICON_THEME_VIEW7, mask_theme_view7),
    BI(ICON_THEME_VIEW8, mask_theme_view8),
    BI(ICON_THEME_VIEW9, mask_theme_view9),
    BI(ICON_THEME_VIEW10, mask_theme_view10),
    BI(ICON_THEME_ADD_VIEW, mask_theme_add_view),
    BI(ICON_STATS, mask_menu_stats),
    BI(ICON_STATS_TIMERS, mask_stats_timers),
    BI(ICON_STATS_ANALOGS, mask_stats_analogs),
    BI(ICON_STATS_DEBUG, mask_stats_debug),
    BI(ICON_MONITOR, mask_monitor),
    BI(ICON_MONITOR_LOGICAL_SWITCHES, mask_monitor_logsw),

    BI(ICON_CHAN_MONITOR_LOCKED, mask_chan_locked),
    BI(ICON_CHAN_MONITOR_INVERTED, mask_chan_inverted),

    BI(ICON_SHUTDOWN_CIRCLE0, mask_shutdown_circle0),
    BI(ICON_SHUTDOWN_CIRCLE1, mask_shutdown_circle1),
    BI(ICON_SHUTDOWN_CIRCLE2, mask_shutdown_circle2),
    BI(ICON_SHUTDOWN_CIRCLE3, mask_shutdown_circle3),
    BI(ICON_SHUTDOWN, mask_shutdown),

    BI(ICON_TOPLEFT_BG, mask_topleft_bg),
    BI(ICON_TOPRIGHT_BG, mask_topright_bg),
    BI(ICON_CURRENTMENU_BG, mask_currentmenu_bg),
    BI(ICON_CURRENTMENU_SHADOW, mask_currentmenu_shadow),
    BI(ICON_CURRENTMENU_DOT, mask_currentmenu_dot),

    BI(ICON_DOT, mask_dot),

    BI(ICON_TOPMENU_USB, mask_topmenu_usb),
    BI(ICON_TOPMENU_VOLUME_0, mask_topmenu_vol0),
    BI(ICON_TOPMENU_VOLUME_1, mask_topmenu_vol1),
    BI(ICON_TOPMENU_VOLUME_2, mask_topmenu_vol2),
    BI(ICON_TOPMENU_VOLUME_3, mask_topmenu_vol3),
    BI(ICON_TOPMENU_VOLUME_4, mask_topmenu_vol4),
    BI(ICON_TOPMENU_VOLUME_SCALE, mask_topmenu_vol_scale),
    BI(ICON_TOPMENU_TXBATT, mask_topmenu_txbatt),
#if defined(USB_CHARGER)
    BI(ICON_TOPMENU_TXBATT_CHARGE, mask_topmenu_txbatt_charging),
#endif
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)||defined(RADIO_V12) && defined(EXTERNAL_ANTENNA)
    BI(ICON_TOPMENU_ANTENNA, mask_topmenu_antenna),
#endif
#if defined(INTERNAL_GPS)
    BI(ICON_TOPMENU_GPS, mask_topmenu_gps),
#endif

    BI(ICON_ERROR, mask_error),
    BI(ICON_BUSY, mask_busy),

    BI(ICON_USB_PLUGGED, mask_usb_plugged),

    BI(ICON_TIMER_BG, mask_timer_bg),
    BI(ICON_TIMER, mask_timer),

    BI(ICON_TEXTLINE_CURVE, mask_textline_curve),
    BI(ICON_TEXTLINE_FM, mask_textline_fm),

    BI(ICON_MPLEX_ADD, mask_mplex_add),
    BI(ICON_MPLEX_MULTIPLY, mask_mplex_multi),
    BI(ICON_MPLEX_REPLACE, mask_mplex_replace),

    BI(ICON_ROUND_TITLE_LEFT, mask_round_title_left),
    BI(ICON_ROUND_TITLE_RIGHT, mask_round_title_right),
    BI(ICON_MODEL_GRID_LARGE, mask_model_grid_large),
    BI(ICON_MODEL_GRID_SMALL, mask_model_grid_small),
    BI(ICON_MODEL_LIST_TWO, mask_model_list_two),
    BI(ICON_MODEL_LIST_ONE, mask_model_list_one),

    BI(ICON_TRIM, mask_trim),
    BI(ICON_TRIM_SHADOW, mask_trim_shadow),

    BI(ICON_TOOLS_APPS, mask_tools_apps),
    BI(ICON_TOOLS_RESET, mask_tools_reset),

    BI(ICON_BTN_CLOSE, mask_btn_close),
    BI(ICON_BTN_NEXT, mask_btn_next),
    BI(ICON_BTN_PREV, mask_btn_prev),
    BI(ICON_TOP_LOGO, mask_top_logo),
};

static MaskBitmap* _builtinIconsDecompressed[EDGETX_ICONS_COUNT] = {0};

const MaskBitmap* getBuiltinIcon(EdgeTxIcon id)
{
  // Icons are stored LZ4 compressed and de-compresssed on first use
  if (_builtinIconsDecompressed[id] == nullptr) {
    _builtinIconsDecompressed[id] =
        _decompressed_mask(_builtinIcons[id].lz4_compressed_bitmap);
  }

  return _builtinIconsDecompressed[id];
}

#endif
