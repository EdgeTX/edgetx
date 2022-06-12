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

#include "opentx.h"

const uint8_t _LBM_POINT[] = {
#include "mask_point.lbm"
};
IMPL_LZ4_BITMAP(LBM_POINT);

static const uint8_t mask_next[] = {
#include "mask_next.lbm"
};
static const uint8_t mask_back[] = {
#include "mask_back.lbm"
};
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
static const uint8_t mask_menu_model_select[] = {
#include "mask_menu_model_select.lbm" //TODO: someone may want to make proper icon
};
static const uint8_t mask_model_select_category[] = {
#include "mask_model_select_category.lbm" //TODO: someone should make proper icon
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
static const uint8_t mask_monitor_inver[] = {
#include "mask_monitor_inver.lbm"
};
static const uint8_t mask_monitor_lockch[] = {
#include "mask_monitor_lockch.lbm"
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
static const uint8_t mask_sbar_mixer[] = {
#include "mask_sbar_mixer.lbm"
};
static const uint8_t mask_sbar_output[] = {
#include "mask_sbar_output.lbm"
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
static const uint8_t mask_textline_curve[] = {
#include "mask_textline_curve.lbm"
};
static const uint8_t mask_textline_delay[] = {
#include "mask_textline_delay.lbm"
};
static const uint8_t mask_textline_delayslow[] = {
#include "mask_textline_delayslow.lbm"
};
static const uint8_t mask_textline_fm[] = {
#include "mask_textline_fm.lbm"
};
static const uint8_t mask_textline_label[] = {
#include "mask_textline_label.lbm"
};
static const uint8_t mask_textline_slow[] = {
#include "mask_textline_slow.lbm"
};
static const uint8_t mask_textline_switch[] = {
#include "mask_textline_switch.lbm"
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
static const uint8_t stick_pointer[] = {
#include "alpha_stick_pointer.lbm"
};
static const uint8_t stick_background[] = {
#include "alpha_stick_background.lbm"
};
static const uint8_t trackp_background[] = {
#include "alpha_trackp_background.lbm"
};
static const uint8_t mask_sdfree[] = {
#include "mask_sdfree.lbm"
};
static const uint8_t mask_modelqty[] = {
#include "mask_modelqty.lbm"
};
static const uint8_t mask_modelname[] = {
#include "mask_modelname.lbm"
};
static const uint8_t mask_moveback[] = {
#include "mask_moveback.lbm"
};
static const uint8_t mask_moveico[] = {
#include "mask_moveico.lbm"
};

BitmapBuffer * calibStick = nullptr;
BitmapBuffer * calibStickBackground = nullptr;
BitmapBuffer * calibTrackpBackground = nullptr;
BitmapBuffer * modelselSdFreeBitmap = nullptr;
BitmapBuffer * modelselModelQtyBitmap = nullptr;
BitmapBuffer * modelselModelNameBitmap = nullptr;
BitmapBuffer * modelselModelMoveBackground = nullptr;
BitmapBuffer * modelselModelMoveIcon = nullptr;
BitmapBuffer * chanMonLockedBitmap = nullptr;
BitmapBuffer * chanMonInvertedBitmap = nullptr;
BitmapBuffer * mixerSetupMixerBitmap = nullptr;
BitmapBuffer * mixerSetupToBitmap = nullptr;
BitmapBuffer * mixerSetupOutputBitmap = nullptr;
BitmapBuffer * mixerSetupCurveIcon = nullptr;
BitmapBuffer * mixerSetupSwitchIcon = nullptr;
BitmapBuffer * mixerSetupLabelIcon = nullptr;
BitmapBuffer * mixerSetupDelayIcon = nullptr;
BitmapBuffer * mixerSetupSlowIcon = nullptr;
BitmapBuffer * mixerSetupDelaySlowIcon = nullptr;
BitmapBuffer * mixerSetupFlightmodeIcon = nullptr;

struct _BuiltinBitmap {

  BitmapFormats  type;
  const uint8_t* lz4_compressed_bitmap;
  BitmapBuffer** uncompressed_bitmap;
};

static const _BuiltinBitmap _builtinBitmaps[] = {
    {BMP_ARGB4444, stick_pointer, &calibStick},
    {BMP_ARGB4444, stick_background, &calibStickBackground},
    {BMP_ARGB4444, trackp_background, &calibTrackpBackground},

    {BMP_8BIT, mask_sdfree, &modelselSdFreeBitmap},

    {BMP_8BIT, mask_modelqty, &modelselModelQtyBitmap},
    {BMP_8BIT, mask_modelname, &modelselModelNameBitmap},

    {BMP_8BIT, mask_moveback, &modelselModelMoveBackground},
    {BMP_8BIT, mask_moveico, &modelselModelMoveIcon},
    {BMP_8BIT, mask_moveico, &modelselModelMoveIcon},

    {BMP_8BIT, mask_monitor_lockch, &chanMonLockedBitmap},
    {BMP_8BIT, mask_monitor_inver, &chanMonInvertedBitmap},

    {BMP_8BIT, mask_sbar_mixer, &mixerSetupMixerBitmap},
    {BMP_8BIT, mask_sbar_output, &mixerSetupOutputBitmap},

    {BMP_8BIT, mask_textline_label, &mixerSetupLabelIcon},
    {BMP_8BIT, mask_textline_curve, &mixerSetupCurveIcon},
    {BMP_8BIT, mask_textline_switch, &mixerSetupSwitchIcon},
    {BMP_8BIT, mask_textline_fm, &mixerSetupFlightmodeIcon},
    {BMP_8BIT, mask_textline_slow, &mixerSetupSlowIcon},
    {BMP_8BIT, mask_textline_delay, &mixerSetupDelayIcon},
    {BMP_8BIT, mask_textline_delayslow, &mixerSetupDelaySlowIcon},
};

void loadBuiltinBitmaps()
{
  for (const auto& bm : _builtinBitmaps) {

    delete *bm.uncompressed_bitmap;
    *bm.uncompressed_bitmap = nullptr;
    
    if (bm.type == BMP_ARGB4444 ||
        bm.type == BMP_RGB565) {
      *bm.uncompressed_bitmap = new LZ4Bitmap(bm.type, bm.lz4_compressed_bitmap);
    } else if (bm.type == BMP_8BIT) {
      *bm.uncompressed_bitmap = BitmapBuffer::load8bitMaskLZ4(bm.lz4_compressed_bitmap);
    }
  }
}

struct _BuiltinIcon {
  MenuIcons      id;
  const uint8_t* lz4_compressed_bitmap;
};

static const _BuiltinIcon _builtinIcons[] = {
    {ICON_OPENTX, mask_edgetx},
#if defined(HARDWARE_TOUCH)
    {ICON_NEXT, mask_next},
    {ICON_BACK, mask_back},
#endif
    {ICON_RADIO, mask_menu_radio},
    {ICON_RADIO_SETUP, mask_radio_setup},
    {ICON_RADIO_SD_MANAGER, mask_radio_sd_browser},
    {ICON_RADIO_TOOLS, mask_radio_tools},
    {ICON_RADIO_GLOBAL_FUNCTIONS, mask_radio_global_functions},
    {ICON_RADIO_TRAINER, mask_radio_trainer},
    {ICON_RADIO_HARDWARE, mask_radio_hardware},
    {ICON_RADIO_CALIBRATION, mask_radio_calibration},
    {ICON_RADIO_EDIT_THEME, mask_radio_edit_theme},
    {ICON_RADIO_VERSION, mask_radio_version},
    {ICON_MODEL, mask_menu_model},
    {ICON_MODEL_SETUP, mask_model_setup},
    {ICON_MODEL_HELI, mask_model_heli},
    {ICON_MODEL_FLIGHT_MODES, mask_model_flight_modes},
    {ICON_MODEL_INPUTS, mask_model_inputs},
    {ICON_MODEL_MIXER, mask_model_mixer},
    {ICON_MODEL_NOTES, mask_menu_notes},
    {ICON_MODEL_OUTPUTS, mask_model_outputs},
    {ICON_MODEL_CURVES, mask_model_curves},
    {ICON_MODEL_GVARS, mask_model_gvars},
    {ICON_MODEL_LOGICAL_SWITCHES, mask_model_logical_switches},
    {ICON_MODEL_SPECIAL_FUNCTIONS, mask_model_special_functions},
    {ICON_MODEL_LUA_SCRIPTS, mask_model_lua_scripts},
    {ICON_MODEL_TELEMETRY, mask_model_telemetry},
    {ICON_MODEL_SELECT, mask_menu_model_select},
    {ICON_MODEL_SELECT_CATEGORY, mask_model_select_category},
    {ICON_THEME, mask_menu_theme},
    {ICON_THEME_SETUP, mask_theme_setup},
    {ICON_THEME_VIEW1, mask_theme_view1},
    {ICON_THEME_VIEW2, mask_theme_view2},
    {ICON_THEME_VIEW3, mask_theme_view3},
    {ICON_THEME_VIEW4, mask_theme_view4},
    {ICON_THEME_VIEW5, mask_theme_view5},
    {ICON_THEME_ADD_VIEW, mask_theme_add_view},
    {ICON_STATS, mask_menu_stats},
    {ICON_STATS_THROTTLE_GRAPH, mask_stats_throttle_graph},
    {ICON_STATS_TIMERS, mask_stats_timers},
    {ICON_STATS_ANALOGS, mask_stats_analogs},
    {ICON_STATS_DEBUG, mask_stats_debug},
    {ICON_MONITOR, mask_monitor},
    {ICON_MONITOR_CHANNELS1, mask_monitor_channels1},
    {ICON_MONITOR_CHANNELS2, mask_monitor_channels2},
    {ICON_MONITOR_CHANNELS3, mask_monitor_channels3},
    {ICON_MONITOR_CHANNELS4, mask_monitor_channels4},
    {ICON_MONITOR_LOGICAL_SWITCHES, mask_monitor_logsw},
};

const uint8_t* getBuiltinIcon(MenuIcons id)
{
  return _builtinIcons[id].lz4_compressed_bitmap;
}
