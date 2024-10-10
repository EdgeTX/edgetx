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

#include "libopenui.h"
#include "lz4/lz4.h"
#include "edgetx_helpers.h"

LZ4BitmapBuffer::LZ4BitmapBuffer(uint8_t format, const LZ4Bitmap* lz4Data) :
    BitmapBuffer(format, 0, 0, nullptr)
{
  _width = lz4Data->width;
  _height = lz4Data->height;

  uint32_t pixels = _width * _height;
  data = (uint16_t*)malloc(align32(pixels * sizeof(uint16_t)));

  LZ4_decompress_safe((const char*)lz4Data->data, (char*)data,
                      lz4Data->compressedSize, pixels * sizeof(uint16_t));
  data_end = data + pixels;
}

LZ4BitmapBuffer::~LZ4BitmapBuffer() { free(data); }

#if !defined(BOOT)

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

static const uint8_t mask_menu_model[] = {
#include "mask_menu_model.lbm"
};
static const uint8_t mask_menu_notes[] = {
#include "mask_menu_notes.lbm"
};
static const uint8_t mask_menu_radio[] = {
#include "mask_menu_radio.lbm"
};
static const uint8_t mask_menu_stats[] = {
#include "mask_menu_stats.lbm"
};
static const uint8_t mask_menu_theme[] = {
#include "mask_menu_theme.lbm"
};
static const uint8_t mask_model_curves[] = {
#include "mask_model_curves.lbm"
};
static const uint8_t mask_model_flight_modes[] = {
#include "mask_model_flight_modes.lbm"
};
static const uint8_t mask_model_gvars[] = {
#include "mask_model_gvars.lbm"
};
static const uint8_t mask_model_heli[] = {
#include "mask_model_heli.lbm"
};
static const uint8_t mask_model_inputs[] = {
#include "mask_model_inputs.lbm"
};
static const uint8_t mask_model_logical_switches[] = {
#include "mask_model_logical_switches.lbm"
};
static const uint8_t mask_model_lua_scripts[] = {
#include "mask_model_lua_scripts.lbm"
};
static const uint8_t mask_model_mixer[] = {
#include "mask_model_mixer.lbm"
};
static const uint8_t mask_model_outputs[] = {
#include "mask_model_outputs.lbm"
};
static const uint8_t mask_model_setup[] = {
#include "mask_model_setup.lbm"
};
static const uint8_t mask_model_special_functions[] = {
#include "mask_model_special_functions.lbm"
};
static const uint8_t mask_model_telemetry[] = {
#include "mask_model_telemetry.lbm"
};
static const uint8_t mask_model_usb[] = {
#include "mask_model_usb.lbm"
};
static const uint8_t mask_menu_model_select[] = {
#include "mask_menu_model_select.lbm"  //TODO: someone may want to make proper icon
};
static const uint8_t mask_monitor[] = {
#include "mask_monitor.lbm"
};
static const uint8_t mask_monitor_channels1[] = {
#include "mask_monitor_channels1.lbm"
};
static const uint8_t mask_monitor_channels2[] = {
#include "mask_monitor_channels2.lbm"
};
static const uint8_t mask_monitor_channels3[] = {
#include "mask_monitor_channels3.lbm"
};
static const uint8_t mask_monitor_channels4[] = {
#include "mask_monitor_channels4.lbm"
};
static const uint8_t mask_monitor_logsw[] = {
#include "mask_monitor_logsw.lbm"
};
static const uint8_t mask_edgetx[] = {
#include "mask_edgetx.lbm"
};
static const uint8_t mask_radio_calibration[] = {
#include "mask_radio_calibration.lbm"
};
static const uint8_t mask_radio_global_functions[] = {
#include "mask_radio_global_functions.lbm"
};
static const uint8_t mask_radio_hardware[] = {
#include "mask_radio_hardware.lbm"
};
static const uint8_t mask_radio_sd_browser[] = {
#include "mask_radio_sd_browser.lbm"
};
static const uint8_t mask_radio_setup[] = {
#include "mask_radio_setup.lbm"
};
static const uint8_t mask_radio_tools[] = {
#include "mask_radio_tools.lbm"
};
static const uint8_t mask_radio_edit_theme[] = {
#include "mask_radio_edit_theme.lbm"
};
static const uint8_t mask_radio_trainer[] = {
#include "mask_radio_trainer.lbm"
};
static const uint8_t mask_radio_version[] = {
#include "mask_radio_version.lbm"
};
static const uint8_t mask_stats_analogs[] = {
#include "mask_stats_analogs.lbm"
};
static const uint8_t mask_stats_debug[] = {
#include "mask_stats_debug.lbm"
};
static const uint8_t mask_stats_throttle_graph[] = {
#include "mask_stats_throttle_graph.lbm"
};
static const uint8_t mask_stats_timers[] = {
#include "mask_stats_timers.lbm"
};
static const uint8_t mask_theme_add_view[] = {
#include "mask_theme_add_view.lbm"
};
static const uint8_t mask_theme_setup[] = {
#include "mask_theme_setup.lbm"
};
static const uint8_t mask_theme_view1[] = {
#include "mask_theme_view1.lbm"
};
static const uint8_t mask_theme_view2[] = {
#include "mask_theme_view2.lbm"
};
static const uint8_t mask_theme_view3[] = {
#include "mask_theme_view3.lbm"
};
static const uint8_t mask_theme_view4[] = {
#include "mask_theme_view4.lbm"
};
static const uint8_t mask_theme_view5[] = {
#include "mask_theme_view5.lbm"
};
static const uint8_t mask_theme_view6[] = {
#include "mask_theme_view6.lbm"
};
static const uint8_t mask_theme_view7[] = {
#include "mask_theme_view7.lbm"
};
static const uint8_t mask_theme_view8[] = {
#include "mask_theme_view8.lbm"
};
static const uint8_t mask_theme_view9[] = {
#include "mask_theme_view9.lbm"
};
static const uint8_t mask_theme_view10[] = {
#include "mask_theme_view10.lbm"
};

static const uint8_t mask_chan_locked[] = {
#include "mask_monitor_lockch.lbm"
};
static const uint8_t mask_chan_inverted[] = {
#include "mask_monitor_inver.lbm"
};

static const uint8_t mask_shutdown_circle0[] = {
#include "mask_shutdown_circle0.lbm"
};
static const uint8_t mask_shutdown_circle1[] = {
#include "mask_shutdown_circle1.lbm"
};
static const uint8_t mask_shutdown_circle2[] = {
#include "mask_shutdown_circle2.lbm"
};
static const uint8_t mask_shutdown_circle3[] = {
#include "mask_shutdown_circle3.lbm"
};

static const uint8_t mask_shutdown[] = {
#include "mask_shutdown.lbm"
};

const uint8_t mask_topleft_bg[] = {
#include "mask_topleft.lbm"
};

const uint8_t mask_currentmenu_bg[] = {
#include "mask_currentmenu_bg.lbm"
};
const uint8_t mask_currentmenu_shadow[] = {
#include "mask_currentmenu_shadow.lbm"
};
const uint8_t mask_currentmenu_dot[] = {
#include "mask_currentmenu_dot.lbm"
};

static const uint8_t mask_dot[] = {
#include "mask_dot.lbm"
};

static const uint8_t mask_topmenu_usb[] = {
#include "mask_topmenu_usb.lbm"
};
static const uint8_t mask_topmenu_vol0[] = {
#include "mask_volume_0.lbm"
};
static const uint8_t mask_topmenu_vol1[] = {
#include "mask_volume_1.lbm"
};
static const uint8_t mask_topmenu_vol2[] = {
#include "mask_volume_2.lbm"
};
static const uint8_t mask_topmenu_vol3[] = {
#include "mask_volume_3.lbm"
};
static const uint8_t mask_topmenu_vol4[] = {
#include "mask_volume_4.lbm"
};
static const uint8_t mask_topmenu_vol_scale[] = {
#include "mask_volume_scale.lbm"
};
static const uint8_t mask_topmenu_txbatt[] = {
#include "mask_txbat.lbm"
};
#if defined(USB_CHARGER)
static const uint8_t mask_topmenu_txbatt_charging[] = {
#include "mask_txbat_charging.lbm"
};
#endif
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
static const uint8_t mask_topmenu_antenna[] = {
#include "mask_antenna.lbm"
};
#endif
#if defined(INTERNAL_GPS)
static const uint8_t mask_topmenu_gps[] = {
#include "mask_topmenu_gps_18.lbm"
};
#endif

static const uint8_t mask_error[] = {
#include "mask_error.lbm"
};
static const uint8_t mask_busy[] = {
#include "mask_busy.lbm"
};

static const uint8_t mask_usb_plugged[] = {
#include "mask_usb_symbol.lbm"
};

static const uint8_t mask_timer[] = {
#include "mask_timer.lbm"
};
static const uint8_t mask_timer_bg[] = {
#include "mask_timer_bg.lbm"
};

static const uint8_t mask_textline_curve[] = {
#include "mask_textline_curve.lbm"
};
static const uint8_t mask_textline_fm[] = {
#include "mask_textline_fm.lbm"
};

static const uint8_t mask_mplex_add[] = {
#include "mask_mplex_add.lbm"
};
static const uint8_t mask_mplex_multi[] = {
#include "mask_mplex_multi.lbm"
};
static const uint8_t mask_mplex_replace[] = {
#include "mask_mplex_replace.lbm"
};

const uint8_t mask_round_title_left[]{
#include "mask_round_title_left.lbm"
};
const uint8_t mask_round_title_right[]{
#include "mask_round_title_right.lbm"
};

static const uint8_t mask_model_grid_large[] = {
#include "mask_model_grid_large.lbm"
};
static const uint8_t mask_model_grid_small[] = {
#include "mask_model_grid_small.lbm"
};
static const uint8_t mask_model_list_one[] = {
#include "mask_model_list_one.lbm"
};
static const uint8_t mask_model_list_two[] = {
#include "mask_model_list_two.lbm"
};

struct _BuiltinIcon {
  const uint8_t* lz4_compressed_bitmap;
};

#define BI(icon, mask) \
  {                    \
    mask               \
  }

// Note: Order must match EdgeTxIcon enum
static const _BuiltinIcon _builtinIcons[EDGETX_ICONS_COUNT] = {
    BI(ICON_EDGETX, mask_edgetx),
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
    BI(ICON_STATS_THROTTLE_GRAPH, mask_stats_throttle_graph),
    BI(ICON_STATS_TIMERS, mask_stats_timers),
    BI(ICON_STATS_ANALOGS, mask_stats_analogs),
    BI(ICON_STATS_DEBUG, mask_stats_debug),
    BI(ICON_MONITOR, mask_monitor),
    BI(ICON_MONITOR_CHANNELS1, mask_monitor_channels1),
    BI(ICON_MONITOR_CHANNELS2, mask_monitor_channels2),
    BI(ICON_MONITOR_CHANNELS3, mask_monitor_channels3),
    BI(ICON_MONITOR_CHANNELS4, mask_monitor_channels4),
    BI(ICON_MONITOR_LOGICAL_SWITCHES, mask_monitor_logsw),

    BI(ICON_CHAN_MONITOR_LOCKED, mask_chan_locked),
    BI(ICON_CHAN_MONITOR_INVERTED, mask_chan_inverted),

    BI(ICON_SHUTDOWN_CIRCLE0, mask_shutdown_circle0),
    BI(ICON_SHUTDOWN_CIRCLE1, mask_shutdown_circle1),
    BI(ICON_SHUTDOWN_CIRCLE2, mask_shutdown_circle2),
    BI(ICON_SHUTDOWN_CIRCLE3, mask_shutdown_circle3),
    BI(ICON_SHUTDOWN, mask_shutdown),

    BI(ICON_TOPLEFT_BG, mask_topleft_bg),
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
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
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
