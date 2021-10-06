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

const uint8_t mask_next[] = {
#include "mask_next.lbm"
};
const uint8_t mask_back[] = {
#include "mask_back.lbm"
};
const uint8_t mask_currentmenu_bg[] = {
#include "mask_currentmenu_bg.lbm"
};
const uint8_t mask_currentmenu_dot[] = {
#include "mask_currentmenu_dot.lbm"
};
const uint8_t mask_currentmenu_shadow[] = {
#include "mask_currentmenu_shadow.lbm"
};
const uint8_t mask_menu_model[] = {
#include "mask_menu_model.lbm"
};
const uint8_t mask_menu_notes[] = {
#include "mask_menu_notes.lbm"
};
const uint8_t mask_menu_radio[] = {
#include "mask_menu_radio.lbm"
};
const uint8_t mask_menu_stats[] = {
#include "mask_menu_stats.lbm"
};
const uint8_t mask_menu_theme[] = {
#include "mask_menu_theme.lbm"
};
const uint8_t mask_model_curves[] = {
#include "mask_model_curves.lbm"
};
const uint8_t mask_model_flight_modes[] = {
#include "mask_model_flight_modes.lbm"
};
const uint8_t mask_model_gvars[] = {
#include "mask_model_gvars.lbm"
};
const uint8_t mask_model_heli[] = {
#include "mask_model_heli.lbm"
};
const uint8_t mask_model_inputs[] = {
#include "mask_model_inputs.lbm"
};
const uint8_t mask_model_logical_switches[] = {
#include "mask_model_logical_switches.lbm"
};
const uint8_t mask_model_lua_scripts[] = {
#include "mask_model_lua_scripts.lbm"
};
const uint8_t mask_model_mixer[] = {
#include "mask_model_mixer.lbm"
};
const uint8_t mask_model_outputs[] = {
#include "mask_model_outputs.lbm"
};
const uint8_t mask_model_setup[] = {
#include "mask_model_setup.lbm"
};
const uint8_t mask_model_special_functions[] = {
#include "mask_model_special_functions.lbm"
};
const uint8_t mask_model_telemetry[] = {
#include "mask_model_telemetry.lbm"
};
const uint8_t mask_menu_model_select[] = {
#include "mask_menu_model_select.lbm" //TODO: someone may want to make proper icon
};
const uint8_t mask_model_select_category[] = {
#include "mask_model_select_category.lbm" //TODO: someone should make proper icon
};
const uint8_t mask_monitor[] = {
#include "mask_monitor.lbm"
};
const uint8_t mask_monitor_channels1[] = {
#include "mask_monitor_channels1.lbm"
};
const uint8_t mask_monitor_channels2[] = {
#include "mask_monitor_channels2.lbm"
};
const uint8_t mask_monitor_channels3[] = {
#include "mask_monitor_channels3.lbm"
};
const uint8_t mask_monitor_channels4[] = {
#include "mask_monitor_channels4.lbm"
};
const uint8_t mask_monitor_gvars[] = {
#include "mask_monitor_gvars.lbm"
};
const uint8_t mask_monitor_inver[] = {
#include "mask_monitor_inver.lbm"
};
const uint8_t mask_monitor_lockch[] = {
#include "mask_monitor_lockch.lbm"
};
const uint8_t mask_monitor_logsw[] = {
#include "mask_monitor_logsw.lbm"
};
const uint8_t mask_monitor_mixes1[] = {
#include "mask_monitor_mixes1.lbm"
};
const uint8_t mask_monitor_mixes2[] = {
#include "mask_monitor_mixes2.lbm"
};
const uint8_t mask_mplex_add[] = {
#include "mask_mplex_add.lbm"
};
const uint8_t mask_mplex_multi[] = {
#include "mask_mplex_multi.lbm"
};
const uint8_t mask_mplex_replace[] = {
#include "mask_mplex_replace.lbm"
};
const uint8_t mask_topleft[] = {
#include "mask_topleft.lbm"
};
const uint8_t mask_edgetx[] = {
#include "mask_edgetx.lbm"
};
const uint8_t mask_opentx_testmode[] = {
#include "mask_opentx_testmode.lbm"
};
const uint8_t mask_radio_calibration[] = {
#include "mask_radio_calibration.lbm"
};
const uint8_t mask_radio_global_functions[] = {
#include "mask_radio_global_functions.lbm"
};
const uint8_t mask_radio_hardware[] = {
#include "mask_radio_hardware.lbm"
};
const uint8_t mask_radio_sd_browser[] = {
#include "mask_radio_sd_browser.lbm"
};
const uint8_t mask_radio_setup[] = {
#include "mask_radio_setup.lbm"
};
const uint8_t mask_radio_tools[] = {
#include "mask_radio_tools.lbm"
};
const uint8_t mask_radio_edit_theme[] = {
#include "mask_radio_edit_theme.lbm"
};

const uint8_t mask_radio_trainer[] = {
#include "mask_radio_trainer.lbm"
};
const uint8_t mask_radio_version[] = {
#include "mask_radio_version.lbm"
};
const uint8_t mask_sbar_mixer[] = {
#include "mask_sbar_mixer.lbm"
};
const uint8_t mask_sbar_output[] = {
#include "mask_sbar_output.lbm"
};
const uint8_t mask_sbar_to[] = {
#include "mask_sbar_to.lbm"
};
const uint8_t mask_stats_analogs[] = {
#include "mask_stats_analogs.lbm"
};
const uint8_t mask_stats_debug[] = {
#include "mask_stats_debug.lbm"
};
const uint8_t mask_stats_lua[] = {
#include "mask_stats_lua.lbm"
};
const uint8_t mask_stats_throttle_graph[] = {
#include "mask_stats_throttle_graph.lbm"
};
const uint8_t mask_stats_timers[] = {
#include "mask_stats_timers.lbm"
};
const uint8_t mask_stats_value[] = {
#include "mask_stats_value.lbm"
};
const uint8_t mask_switches[] = {
#include "mask_switches.lbm"
};
const uint8_t mask_textline_curve[] = {
#include "mask_textline_curve.lbm"
};
const uint8_t mask_textline_delay[] = {
#include "mask_textline_delay.lbm"
};
const uint8_t mask_textline_delayslow[] = {
#include "mask_textline_delayslow.lbm"
};
const uint8_t mask_textline_fm[] = {
#include "mask_textline_fm.lbm"
};
const uint8_t mask_textline_label[] = {
#include "mask_textline_label.lbm"
};
const uint8_t mask_textline_slow[] = {
#include "mask_textline_slow.lbm"
};
const uint8_t mask_textline_switch[] = {
#include "mask_textline_switch.lbm"
};
const uint8_t mask_theme_add_view[] = {
#include "mask_theme_add_view.lbm"
};
const uint8_t mask_theme_setup[] = {
#include "mask_theme_setup.lbm"
};
const uint8_t mask_theme_view1[] = {
#include "mask_theme_view1.lbm"
};
const uint8_t mask_theme_view2[] = {
#include "mask_theme_view2.lbm"
};
const uint8_t mask_theme_view3[] = {
#include "mask_theme_view3.lbm"
};
const uint8_t mask_theme_view4[] = {
#include "mask_theme_view4.lbm"
};
const uint8_t mask_theme_view5[] = {
#include "mask_theme_view5.lbm"
};
const uint8_t mask_theme_view6[] = {
#include "mask_theme_view6.lbm"
};
const uint8_t mask_theme_view7[] = {
#include "mask_theme_view7.lbm"
};
const uint8_t mask_theme_view8[] = {
#include "mask_theme_view8.lbm"
};
const uint8_t stick_pointer[] = {
#include "alpha_stick_pointer.lbm"
};
const uint8_t stick_background[] = {
#include "alpha_stick_background.lbm"
};
const uint8_t trackp_background[] = {
#include "alpha_trackp_background.lbm"
};
const uint8_t mask_sdfree[] = {
#include "mask_sdfree.lbm"
};
const uint8_t mask_modelqty[] = {
#include "mask_modelqty.lbm"
};
const uint8_t mask_modelname[] = {
#include "mask_modelname.lbm"
};
const uint8_t mask_moveback[] = {
#include "mask_moveback.lbm"
};
const uint8_t mask_moveico[] = {
#include "mask_moveico.lbm"
};

