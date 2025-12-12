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

// gfx for quick menu
static const uint8_t mask_menu_favs[] = {
#include "mask_icon_menu_favs.lbm"
};

static const uint8_t mask_menu_manage_models[] = {
#include "mask_icon_menu_manage_models.lbm"  //TODO: someone may want to make proper icon
};
static const uint8_t mask_menu_model_setup[] = {
#include "mask_icon_menu_model_setup.lbm"
};
static const uint8_t mask_menu_radio_setup[] = {
#include "mask_icon_menu_radio_setup.lbm"
};
static const uint8_t mask_menu_ui_setup[] = {
#include "mask_icon_menu_ui_setup.lbm"
};
static const uint8_t mask_menu_tools[] = {
#include "mask_icon_menu_tools.lbm"
};
static const uint8_t mask_menu_logo[] = {
#include "mask_menu_edgetx.lbm"
};

// gfx for settings screens

static const uint8_t mask_edgetx[] = {
#include "mask_icon_edgetx.lbm"
};


// gfx model setup from model group

static const uint8_t mask_model_curves[] = {
#include "mask_icon_model_curves.lbm"
};
static const uint8_t mask_model_flight_modes[] = {
#include "mask_icon_model_flight_modes.lbm"
};
static const uint8_t mask_model_general[] = {
#include "mask_icon_model_general.lbm"
};
static const uint8_t mask_model_gvars[] = {
#include "mask_icon_model_gvars.lbm"
};
static const uint8_t mask_model_heli[] = {
#include "mask_icon_model_heli.lbm"
};
static const uint8_t mask_model_inputs[] = {
#include "mask_icon_model_inputs.lbm"
};
static const uint8_t mask_model_logical_switches[] = {
#include "mask_icon_model_logical_switches.lbm"
};
static const uint8_t mask_model_mixer[] = {
#include "mask_icon_model_mixer.lbm"
};
static const uint8_t mask_model_mixer_scripts[] = {
#include "mask_icon_model_mixer_scripts.lbm"
};
static const uint8_t mask_model_notes[] = {
#include "mask_icon_model_notes.lbm"
};
static const uint8_t mask_model_outputs[] = {
#include "mask_icon_model_outputs.lbm"
};
static const uint8_t mask_model_special_functions[] = {
#include "mask_icon_model_special_functions.lbm"
};
static const uint8_t mask_model_telemetry[] = {
#include "mask_icon_model_telemetry.lbm"
};
static const uint8_t mask_model_timers[] = {
#include "mask_icon_model_timers.lbm"
};
static const uint8_t mask_model_usb[] = {
#include "mask_icon_model_usb.lbm"
};

// gfx for radio setup group

static const uint8_t mask_radio_about[] = {
#include "mask_icon_radio_about.lbm"
};
static const uint8_t mask_radio_analogs[] = {
#include "mask_icon_radio_analogs.lbm"
};
static const uint8_t mask_radio_calibration[] = {
#include "mask_icon_radio_calibration.lbm"
};
static const uint8_t mask_radio_general[] = {
#include "mask_icon_radio_general.lbm"
};
static const uint8_t mask_radio_global_functions[] = {
#include "mask_icon_radio_global_functions.lbm"
};
static const uint8_t mask_radio_hardware[] = {
#include "mask_icon_radio_hardware.lbm"
};
static const uint8_t mask_radio_trainer[] = {
#include "mask_icon_radio_trainer.lbm"
};

// gfx for tools group

static const uint8_t mask_tools_apps[] = {
#include "mask_icon_tools_apps.lbm"
};
static const uint8_t mask_tools_debug[] = {
#include "mask_icon_tools_debug.lbm"
};
static const uint8_t mask_tools_monitor_ch[] = {
#include "mask_icon_tools_monitor_ch.lbm"
};
static const uint8_t mask_tools_monitor_ls[] = {
#include "mask_icon_tools_monitor_ls.lbm"
};
static const uint8_t mask_tools_reset[] = {
#include "mask_icon_tools_reset.lbm"
};
static const uint8_t mask_tools_stats[] = {
#include "mask_icon_tools_stats.lbm"
};
static const uint8_t mask_tools_storage[] = {
#include "mask_icon_tools_storage.lbm"
};

// gfx for ui setup group

static const uint8_t mask_ui_view_add[] = {
#include "mask_icon_ui_view_add.lbm"
};
static const uint8_t mask_ui_view1[] = {
#include "mask_icon_ui_view1.lbm"
};
static const uint8_t mask_ui_view2[] = {
#include "mask_icon_ui_view2.lbm"
};
static const uint8_t mask_ui_view3[] = {
#include "mask_icon_ui_view3.lbm"
};
static const uint8_t mask_ui_view4[] = {
#include "mask_icon_ui_view4.lbm"
};
static const uint8_t mask_ui_view5[] = {
#include "mask_icon_ui_view5.lbm"
};
static const uint8_t mask_ui_view6[] = {
#include "mask_icon_ui_view6.lbm"
};
static const uint8_t mask_ui_view7[] = {
#include "mask_icon_ui_view7.lbm"
};
static const uint8_t mask_ui_view8[] = {
#include "mask_icon_ui_view8.lbm"
};
static const uint8_t mask_ui_view9[] = {
#include "mask_icon_ui_view9.lbm"
};
static const uint8_t mask_ui_view10[] = {
#include "mask_icon_ui_view10.lbm"
};
static const uint8_t mask_ui_themes[] = {
#include "mask_icon_ui_themes.lbm"
};
static const uint8_t mask_ui_topbar_setup[] = {
#include "mask_icon_ui_topbar_setup.lbm"
};

// gfx used in lines

static const uint8_t mask_inline_add[] = {
#include "mask_inline_add.lbm"
};
static const uint8_t mask_inline_curve[] = {
#include "mask_inline_curve.lbm"
};
static const uint8_t mask_inline_dot[] = {
#include "mask_inline_dot.lbm"
};
static const uint8_t mask_inline_fm[] = {
#include "mask_inline_fm.lbm"
};
static const uint8_t mask_inline_inverted[] = {
#include "mask_inline_inverted.lbm"
};
static const uint8_t mask_inline_locked[] = {
#include "mask_inline_locked.lbm"
};
static const uint8_t mask_inline_multiply[] = {
#include "mask_inline_multiply.lbm"
};
static const uint8_t mask_inline_replace[] = {
#include "mask_inline_replace.lbm"
};

// gfx for info screens

static const uint8_t mask_info_busy[] = {
#include "mask_info_busy.lbm"
};
static const uint8_t mask_info_error[] = {
#include "mask_info_error.lbm"
};
static const uint8_t mask_info_shutdown[] = {
#include "mask_info_shutdown.lbm"
};
static const uint8_t mask_info_shutdown_circle0[] = {
#include "mask_info_shutdown_circle0.lbm"
};
static const uint8_t mask_info_shutdown_circle1[] = {
#include "mask_info_shutdown_circle1.lbm"
};
static const uint8_t mask_info_shutdown_circle2[] = {
#include "mask_info_shutdown_circle2.lbm"
};
static const uint8_t mask_info_shutdown_circle3[] = {
#include "mask_info_shutdown_circle3.lbm"
};
static const uint8_t mask_info_usb_plugged[] = {
#include "mask_info_usb_plugged.lbm"
};


// gfx for ui elements

const uint8_t mask_round_title_left[]{
#include "mask_ui_bg_tile_top_left.lbm"
};
const uint8_t mask_round_title_right[]{
#include "mask_ui_bg_tile_top_right.lbm"
};
const uint8_t mask_topleft_bg[] = {
#include "mask_ui_bg_topbar_left.lbm"
};
const uint8_t mask_topright_bg[] = {
#include "mask_ui_bg_topbar_right.lbm"
};
static const uint8_t mask_btn_close[] = {
#include "mask_ui_btn_close.lbm"
};
static const uint8_t mask_btn_next[] = {
#include "mask_ui_btn_next.lbm"
};
static const uint8_t mask_btn_prev[] = {
#include "mask_ui_btn_prev.lbm"
};
static const uint8_t mask_btn_grid_large[] = {
#include "mask_ui_btn_grid_large.lbm"
};
static const uint8_t mask_btn_grid_small[] = {
#include "mask_ui_btn_grid_small.lbm"
};
static const uint8_t mask_btn_list_one[] = {
#include "mask_ui_btn_list_one.lbm"
};
static const uint8_t mask_btn_list_two[] = {
#include "mask_ui_btn_list_two.lbm"
};

// gfx for widgets

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
static const uint8_t mask_topmenu_antenna[] = {
#include "mask_widget_antenna.lbm"
};
#endif
#if defined(INTERNAL_GPS)
static const uint8_t mask_topmenu_gps[] = {
#include "mask_widget_gps.lbm"
};
#endif
static const uint8_t mask_widget_timer[] = {
#include "mask_widget_timer.lbm"
};
static const uint8_t mask_widget_timer_bg[] = {
#include "mask_widget_timer_bg.lbm"
};
static const uint8_t mask_widget_trim[] = {
#include "mask_widget_trim.lbm"
};
static const uint8_t mask_widget_trim_shadow[] = {
#include "mask_widget_trim_shadow.lbm"
};
static const uint8_t mask_widget_txbatt[] = {
#include "mask_widget_txbat.lbm"
};
#if defined(USB_CHARGER)
static const uint8_t mask_widget_txbatt_charging[] = {
#include "mask_widget_txbat_charging.lbm"
};
#endif
static const uint8_t mask_widget_usb[] = {
#include "mask_widget_usb.lbm"
};
static const uint8_t mask_widget_volume0[] = {
#include "mask_widget_volume0.lbm"
};
static const uint8_t mask_widget_volume1[] = {
#include "mask_widget_volume1.lbm"
};
static const uint8_t mask_widget_volume2[] = {
#include "mask_widget_volume2.lbm"
};
static const uint8_t mask_widget_volume3[] = {
#include "mask_widget_volume3.lbm"
};
static const uint8_t mask_widget_volume4[] = {
#include "mask_widget_volume4.lbm"
};
static const uint8_t mask_widget_volume_scale[] = {
#include "mask_widget_volume_scale.lbm"
};

// const uint8_t mask_currentmenu_bg[] = {
// #include "mask_currentmenu_bg.lbm"
// };
// const uint8_t mask_currentmenu_shadow[] = {
// #include "mask_currentmenu_shadow.lbm"
// };
// const uint8_t mask_currentmenu_dot[] = {
// #include "mask_currentmenu_dot.lbm"
// };

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
    BI(ICON_QM_FAVORITES, mask_menu_favs),
    BI(ICON_RADIO, mask_menu_radio_setup),
    BI(ICON_RADIO_SETUP, mask_radio_general),
    BI(ICON_RADIO_SD_MANAGER, mask_tools_storage),
    BI(ICON_RADIO_TOOLS, mask_menu_tools),
    BI(ICON_RADIO_GLOBAL_FUNCTIONS, mask_radio_global_functions),
    BI(ICON_RADIO_TRAINER, mask_radio_trainer),
    BI(ICON_RADIO_HARDWARE, mask_radio_hardware),
    BI(ICON_RADIO_CALIBRATION, mask_radio_calibration),
    BI(ICON_RADIO_EDIT_THEME, mask_ui_themes),
    BI(ICON_RADIO_VERSION, mask_radio_about),
    BI(ICON_MODEL, mask_menu_model_setup),
    BI(ICON_MODEL_SETUP, mask_model_general),
    BI(ICON_MODEL_HELI, mask_model_heli),
    BI(ICON_MODEL_FLIGHT_MODES, mask_model_flight_modes),
    BI(ICON_MODEL_INPUTS, mask_model_inputs),
    BI(ICON_MODEL_MIXER, mask_model_mixer),
    BI(ICON_MODEL_NOTES, mask_model_notes),
    BI(ICON_MODEL_OUTPUTS, mask_model_outputs),
    BI(ICON_MODEL_CURVES, mask_model_curves),
    BI(ICON_MODEL_GVARS, mask_model_gvars),
    BI(ICON_MODEL_LOGICAL_SWITCHES, mask_model_logical_switches),
    BI(ICON_MODEL_SPECIAL_FUNCTIONS, mask_model_special_functions),
    BI(ICON_MODEL_LUA_SCRIPTS, mask_model_mixer_scripts),
    BI(ICON_MODEL_TELEMETRY, mask_model_telemetry),
    BI(ICON_MODEL_USB, mask_model_usb),
    BI(ICON_MODEL_SELECT, mask_menu_manage_models),
    BI(ICON_THEME, mask_menu_ui_setup),
    BI(ICON_THEME_SETUP, mask_ui_topbar_setup),
    BI(ICON_THEME_VIEW1, mask_ui_view1),
    BI(ICON_THEME_VIEW2, mask_ui_view2),
    BI(ICON_THEME_VIEW3, mask_ui_view3),
    BI(ICON_THEME_VIEW4, mask_ui_view4),
    BI(ICON_THEME_VIEW5, mask_ui_view5),
    BI(ICON_THEME_VIEW6, mask_ui_view6),
    BI(ICON_THEME_VIEW7, mask_ui_view7),
    BI(ICON_THEME_VIEW8, mask_ui_view8),
    BI(ICON_THEME_VIEW9, mask_ui_view9),
    BI(ICON_THEME_VIEW10, mask_ui_view10),
    BI(ICON_THEME_ADD_VIEW, mask_ui_view_add),
    BI(ICON_STATS, mask_tools_stats),
    BI(ICON_STATS_TIMERS, mask_model_timers),
    BI(ICON_STATS_ANALOGS, mask_radio_analogs),
    BI(ICON_STATS_DEBUG, mask_tools_debug),
    BI(ICON_MONITOR, mask_tools_monitor_ch),
    BI(ICON_MONITOR_LOGICAL_SWITCHES, mask_tools_monitor_ls),

    BI(ICON_CHAN_MONITOR_LOCKED, mask_inline_locked),
    BI(ICON_CHAN_MONITOR_INVERTED, mask_inline_inverted),

    BI(ICON_SHUTDOWN_CIRCLE0, mask_info_shutdown_circle0),
    BI(ICON_SHUTDOWN_CIRCLE1, mask_info_shutdown_circle1),
    BI(ICON_SHUTDOWN_CIRCLE2, mask_info_shutdown_circle2),
    BI(ICON_SHUTDOWN_CIRCLE3, mask_info_shutdown_circle3),
    BI(ICON_SHUTDOWN, mask_info_shutdown),

    BI(ICON_TOPLEFT_BG, mask_topleft_bg),
    BI(ICON_TOPRIGHT_BG, mask_topright_bg),
    // BI(ICON_CURRENTMENU_BG, mask_currentmenu_bg),
    // BI(ICON_CURRENTMENU_SHADOW, mask_currentmenu_shadow),
    // BI(ICON_CURRENTMENU_DOT, mask_currentmenu_dot),

    BI(ICON_DOT, mask_inline_dot),

    BI(ICON_TOPMENU_USB, mask_widget_usb),
    BI(ICON_TOPMENU_VOLUME_0, mask_widget_volume0),
    BI(ICON_TOPMENU_VOLUME_1, mask_widget_volume1),
    BI(ICON_TOPMENU_VOLUME_2, mask_widget_volume2),
    BI(ICON_TOPMENU_VOLUME_3, mask_widget_volume3),
    BI(ICON_TOPMENU_VOLUME_4, mask_widget_volume4),
    BI(ICON_TOPMENU_VOLUME_SCALE, mask_widget_volume_scale),
    BI(ICON_TOPMENU_TXBATT, mask_widget_txbatt),
#if defined(USB_CHARGER)
    BI(ICON_TOPMENU_TXBATT_CHARGE, mask_widget_txbatt_charging),
#endif
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
    BI(ICON_TOPMENU_ANTENNA, mask_topmenu_antenna),
#endif
#if defined(INTERNAL_GPS)
    BI(ICON_TOPMENU_GPS, mask_topmenu_gps),
#endif

    BI(ICON_ERROR, mask_info_error),
    BI(ICON_BUSY, mask_info_busy),

    BI(ICON_USB_PLUGGED, mask_info_usb_plugged),

    BI(ICON_TIMER_BG, mask_widget_timer_bg),
    BI(ICON_TIMER, mask_widget_timer),

    BI(ICON_TEXTLINE_CURVE, mask_inline_curve),
    BI(ICON_TEXTLINE_FM, mask_inline_fm),

    BI(ICON_MPLEX_ADD, mask_inline_add),
    BI(ICON_MPLEX_MULTIPLY, mask_inline_multiply),
    BI(ICON_MPLEX_REPLACE, mask_inline_replace),

    BI(ICON_ROUND_TITLE_LEFT, mask_round_title_left),
    BI(ICON_ROUND_TITLE_RIGHT, mask_round_title_right),
    BI(ICON_MODEL_GRID_LARGE, mask_btn_grid_large),
    BI(ICON_MODEL_GRID_SMALL, mask_btn_grid_small),
    BI(ICON_MODEL_LIST_TWO, mask_btn_list_two),
    BI(ICON_MODEL_LIST_ONE, mask_btn_list_one),

    BI(ICON_TRIM, mask_widget_trim),
    BI(ICON_TRIM_SHADOW, mask_widget_trim_shadow),

    BI(ICON_TOOLS_APPS, mask_tools_apps),
    BI(ICON_TOOLS_RESET, mask_tools_reset),

    BI(ICON_BTN_CLOSE, mask_btn_close),
    BI(ICON_BTN_NEXT, mask_btn_next),
    BI(ICON_BTN_PREV, mask_btn_prev),
    BI(ICON_TOP_LOGO, mask_menu_logo),
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
