#!/bin/bash

# Convert source SVG files to PNG icons
#   - requires command line inkscape app

# multi color
inkscape -w 89 -h 47 ../img-src/bootloader/bmp_plug_usb.svg -o ./bootloader/bmp_plug_usb.png
inkscape -w 42 -h 43 ../img-src/bootloader/bmp_usb_plugged.svg -o ./bootloader/bmp_usb_plugged.png
inkscape -w 72 -h 72 ../img-src/default_theme/alpha_stick_background.svg -o ./default_theme/alpha_stick_background.png
inkscape -w 16 -h 16 ../img-src/default_theme/alpha_stick_pointer.svg -o ./default_theme/alpha_stick_pointer.png
inkscape -w 217 -h 206 ../img-src/splash_logo.svg -o ./splash_logo.png

# grey scale icons
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_btn_close.svg -o ./default_theme/mask_btn_close.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 21 -h 21 ../img-src/default_theme/mask_btn_next.svg -o ./default_theme/mask_btn_next.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 21 -h 21 ../img-src/default_theme/mask_btn_prev.svg -o ./default_theme/mask_btn_prev.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 77 -h 77 ../img-src/default_theme/mask_busy.svg -o ./default_theme/mask_busy.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 27 -h 41 ../img-src/default_theme/mask_currentmenu_bg.svg -o ./default_theme/mask_currentmenu_bg.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 10 -h 10 ../img-src/default_theme/mask_currentmenu_dot.svg -o ./default_theme/mask_currentmenu_dot.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 29 -h 42 ../img-src/default_theme/mask_currentmenu_shadow.svg -o ./default_theme/mask_currentmenu_shadow.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_edgetx.svg -o ./default_theme/mask_edgetx.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 77 -h 77 ../img-src/default_theme/mask_error.svg -o ./default_theme/mask_error.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_menu_model.svg -o ./default_theme/mask_menu_model.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_menu_model_select.svg -o ./default_theme/mask_menu_model_select.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_menu_notes.svg -o ./default_theme/mask_menu_notes.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_menu_radio.svg -o ./default_theme/mask_menu_radio.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_menu_stats.svg -o ./default_theme/mask_menu_stats.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_menu_theme.svg -o ./default_theme/mask_menu_theme.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_model_curves.svg -o ./default_theme/mask_model_curves.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_model_flight_modes.svg -o ./default_theme/mask_model_flight_modes.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 21 -h 21 ../img-src/default_theme/mask_model_grid_large.svg -o ./default_theme/mask_model_grid_large.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 21 -h 21 ../img-src/default_theme/mask_model_grid_small.svg -o ./default_theme/mask_model_grid_small.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_model_gvars.svg -o ./default_theme/mask_model_gvars.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_model_heli.svg -o ./default_theme/mask_model_heli.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_model_inputs.svg -o ./default_theme/mask_model_inputs.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 21 -h 21 ../img-src/default_theme/mask_model_list_one.svg -o ./default_theme/mask_model_list_one.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 21 -h 21 ../img-src/default_theme/mask_model_list_two.svg -o ./default_theme/mask_model_list_two.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_model_logical_switches.svg -o ./default_theme/mask_model_logical_switches.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_model_lua_scripts.svg -o ./default_theme/mask_model_lua_scripts.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_model_mixer.svg -o ./default_theme/mask_model_mixer.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_model_outputs.svg -o ./default_theme/mask_model_outputs.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_model_setup.svg -o ./default_theme/mask_model_setup.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_model_special_functions.svg -o ./default_theme/mask_model_special_functions.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_model_telemetry.svg -o ./default_theme/mask_model_telemetry.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_model_usb.svg -o ./default_theme/mask_model_usb.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_monitor.svg -o ./default_theme/mask_monitor.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 9 -h 13 ../img-src/default_theme/mask_monitor_inver.svg -o ./default_theme/mask_monitor_inver.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 9 -h 13 ../img-src/default_theme/mask_monitor_lockch.svg -o ./default_theme/mask_monitor_lockch.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_monitor_logsw.svg -o ./default_theme/mask_monitor_logsw.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 20 -h 14 ../img-src/default_theme/mask_mplex_add.svg -o ./default_theme/mask_mplex_add.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 20 -h 14 ../img-src/default_theme/mask_mplex_multi.svg -o ./default_theme/mask_mplex_multi.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 20 -h 14 ../img-src/default_theme/mask_mplex_replace.svg -o ./default_theme/mask_mplex_replace.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_radio_calibration.svg -o ./default_theme/mask_radio_calibration.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_radio_edit_theme.svg -o ./default_theme/mask_radio_edit_theme.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_radio_global_functions.svg -o ./default_theme/mask_radio_global_functions.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_radio_hardware.svg -o ./default_theme/mask_radio_hardware.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_radio_sd_browser.svg -o ./default_theme/mask_radio_sd_browser.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_radio_setup.svg -o ./default_theme/mask_radio_setup.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_radio_tools.svg -o ./default_theme/mask_radio_tools.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_radio_trainer.svg -o ./default_theme/mask_radio_trainer.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_radio_version.svg -o ./default_theme/mask_radio_version.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 77 -h 77 ../img-src/default_theme/mask_shutdown.svg -o ./default_theme/mask_shutdown.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_stats_analogs.svg -o ./default_theme/mask_stats_analogs.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_stats_debug.svg -o ./default_theme/mask_stats_debug.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_stats_timers.svg -o ./default_theme/mask_stats_timers.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 14 -h 14 ../img-src/default_theme/mask_textline_curve.svg -o ./default_theme/mask_textline_curve.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 14 -h 14 ../img-src/default_theme/mask_textline_fm.svg -o ./default_theme/mask_textline_fm.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_theme_add_view.svg -o ./default_theme/mask_theme_add_view.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_theme_setup.svg -o ./default_theme/mask_theme_setup.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_theme_view1.svg -o ./default_theme/mask_theme_view1.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_theme_view10.svg -o ./default_theme/mask_theme_view10.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_theme_view2.svg -o ./default_theme/mask_theme_view2.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_theme_view3.svg -o ./default_theme/mask_theme_view3.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_theme_view4.svg -o ./default_theme/mask_theme_view4.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_theme_view5.svg -o ./default_theme/mask_theme_view5.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_theme_view6.svg -o ./default_theme/mask_theme_view6.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_theme_view7.svg -o ./default_theme/mask_theme_view7.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_theme_view8.svg -o ./default_theme/mask_theme_view8.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_theme_view9.svg -o ./default_theme/mask_theme_view9.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_tools_apps.svg -o ./default_theme/mask_tools_apps.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 24 ../img-src/default_theme/mask_tools_reset.svg -o ./default_theme/mask_tools_reset.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 98 -h 20 ../img-src/default_theme/mask_top_logo.svg -o ./default_theme/mask_top_logo.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 36 -h 36 ../img-src/default_theme/mask_topleft.svg -o ./default_theme/mask_topleft.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 36 -h 36 ../img-src/default_theme/mask_topright.svg -o ./default_theme/mask_topright.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 12 -h 12 ../img-src/default_theme/mask_trim.svg -o ./default_theme/mask_trim.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 14 -h 14 ../img-src/default_theme/mask_trim_shadow.svg -o ./default_theme/mask_trim_shadow.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 14 -h 14 ../img-src/mask_antenna.svg -o ./mask_antenna.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 10 -h 10 ../img-src/mask_dot.svg -o ./mask_dot.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 3 -h 16 ../img-src/mask_round_title_left.svg -o ./mask_round_title_left.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 3 -h 16 ../img-src/mask_round_title_right.svg -o ./mask_round_title_right.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 60 -h 60 ../img-src/mask_shutdown_circle0.svg -o ./mask_shutdown_circle0.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 60 -h 60 ../img-src/mask_shutdown_circle1.svg -o ./mask_shutdown_circle1.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 60 -h 60 ../img-src/mask_shutdown_circle2.svg -o ./mask_shutdown_circle2.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 60 -h 60 ../img-src/mask_shutdown_circle3.svg -o ./mask_shutdown_circle3.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 50 -h 50 ../img-src/mask_timer.svg -o ./mask_timer.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 144 -h 56 ../img-src/mask_timer_bg.svg -o ./mask_timer_bg.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 14 -h 14 ../img-src/mask_topmenu_gps_18.svg -o ./mask_topmenu_gps_18.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 18 -h 8 ../img-src/mask_topmenu_usb.svg -o ./mask_topmenu_usb.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 19 -h 10 ../img-src/mask_txbat.svg -o ./mask_txbat.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 4 -h 12 ../img-src/mask_txbat_charging.svg -o ./mask_txbat_charging.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 169 -h 88 ../img-src/mask_usb_symbol.svg -o ./mask_usb_symbol.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 14 -h 13 ../img-src/volume/mask_volume_0.svg -o ./volume/mask_volume_0.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 12 -h 13 ../img-src/volume/mask_volume_1.svg -o ./volume/mask_volume_1.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 15 -h 13 ../img-src/volume/mask_volume_2.svg -o ./volume/mask_volume_2.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 19 -h 13 ../img-src/volume/mask_volume_3.svg -o ./volume/mask_volume_3.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 24 -h 13 ../img-src/volume/mask_volume_4.svg -o ./volume/mask_volume_4.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w 12 -h 13 ../img-src/volume/mask_volume_scale.svg -o ./volume/mask_volume_scale.png
