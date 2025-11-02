#!/bin/bash

# Convert source SVG files to PNG icons for different screen resolutions
# Usage: ./convert_icons.sh [320x240|480x272|800x480]
# Requires: inkscape command line tool

set -e  # Exit on error

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check if inkscape is installed
if ! command -v inkscape &> /dev/null; then
    echo "Error: inkscape is not installed or not in PATH"
    echo "Please install inkscape to use this script"
    echo ""
    echo "Installation instructions:"
    echo "  - Ubuntu/Debian: sudo apt-get install inkscape"
    echo "  - macOS: brew install inkscape"
    echo "  - Windows: Download from https://inkscape.org/release/"
    exit 1
fi

# Parse resolution argument
RESOLUTION=${1:-"320x240"}

case $RESOLUTION in
    "320x240")
        SCALE=1.0
        ;;
    "480x272")
        SCALE=1.5
        ;;
    "800x480")
        SCALE=2.5
        ;;
    *)
        echo "Usage: $0 [320x240|480x272|800x480]"
        echo "Example: $0 480x272"
        exit 1
        ;;
esac

echo "Generating icons for resolution: $RESOLUTION (scale: ${SCALE}x)"

# Base paths (relative to script location)
SRC_DIR="$SCRIPT_DIR/../radio/src/bitmaps/img-src"
OUT_DIR="$SCRIPT_DIR/../radio/src/bitmaps/$RESOLUTION"

# Check if directories exist
if [ ! -d "$SRC_DIR" ]; then
    echo "Error: Source directory not found: $SRC_DIR"
    exit 1
fi

if [ ! -d "$OUT_DIR" ]; then
    echo "Error: Output directory not found: $OUT_DIR"
    exit 1
fi

# Helper function to scale dimensions
scale() {
    echo "scale=0; $1 * $SCALE / 1" | bc
}

echo "Converting SVG files to PNG..."

# Multi-color images
echo "  - Multi-color images..."
inkscape -w $(scale 89) -h $(scale 47) $SRC_DIR/bootloader/bmp_plug_usb.svg -o $OUT_DIR/bootloader/bmp_plug_usb.png
inkscape -w $(scale 42) -h $(scale 43) $SRC_DIR/bootloader/bmp_usb_plugged.svg -o $OUT_DIR/bootloader/bmp_usb_plugged.png
inkscape -w $(scale 72) -h $(scale 72) $SRC_DIR/default_theme/alpha_stick_background.svg -o $OUT_DIR/default_theme/alpha_stick_background.png
inkscape -w $(scale 16) -h $(scale 16) $SRC_DIR/default_theme/alpha_stick_pointer.svg -o $OUT_DIR/default_theme/alpha_stick_pointer.png
inkscape -w $(scale 217) -h $(scale 206) $SRC_DIR/splash_logo.svg -o $OUT_DIR/splash_logo.png

# Grey scale icons
echo "  - Greyscale mask icons..."
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_btn_close.svg -o $OUT_DIR/default_theme/mask_btn_close.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 21) -h $(scale 21) $SRC_DIR/default_theme/mask_btn_next.svg -o $OUT_DIR/default_theme/mask_btn_next.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 21) -h $(scale 21) $SRC_DIR/default_theme/mask_btn_prev.svg -o $OUT_DIR/default_theme/mask_btn_prev.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 77) -h $(scale 77) $SRC_DIR/default_theme/mask_busy.svg -o $OUT_DIR/default_theme/mask_busy.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 27) -h $(scale 41) $SRC_DIR/default_theme/mask_currentmenu_bg.svg -o $OUT_DIR/default_theme/mask_currentmenu_bg.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 10) -h $(scale 10) $SRC_DIR/default_theme/mask_currentmenu_dot.svg -o $OUT_DIR/default_theme/mask_currentmenu_dot.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 29) -h $(scale 42) $SRC_DIR/default_theme/mask_currentmenu_shadow.svg -o $OUT_DIR/default_theme/mask_currentmenu_shadow.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_edgetx.svg -o $OUT_DIR/default_theme/mask_edgetx.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 77) -h $(scale 77) $SRC_DIR/default_theme/mask_error.svg -o $OUT_DIR/default_theme/mask_error.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_menu_model.svg -o $OUT_DIR/default_theme/mask_menu_model.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_menu_model_select.svg -o $OUT_DIR/default_theme/mask_menu_model_select.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_menu_notes.svg -o $OUT_DIR/default_theme/mask_menu_notes.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_menu_radio.svg -o $OUT_DIR/default_theme/mask_menu_radio.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_menu_stats.svg -o $OUT_DIR/default_theme/mask_menu_stats.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_menu_theme.svg -o $OUT_DIR/default_theme/mask_menu_theme.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_model_curves.svg -o $OUT_DIR/default_theme/mask_model_curves.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_model_flight_modes.svg -o $OUT_DIR/default_theme/mask_model_flight_modes.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 21) -h $(scale 21) $SRC_DIR/default_theme/mask_model_grid_large.svg -o $OUT_DIR/default_theme/mask_model_grid_large.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 21) -h $(scale 21) $SRC_DIR/default_theme/mask_model_grid_small.svg -o $OUT_DIR/default_theme/mask_model_grid_small.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_model_gvars.svg -o $OUT_DIR/default_theme/mask_model_gvars.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_model_heli.svg -o $OUT_DIR/default_theme/mask_model_heli.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_model_inputs.svg -o $OUT_DIR/default_theme/mask_model_inputs.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 21) -h $(scale 21) $SRC_DIR/default_theme/mask_model_list_one.svg -o $OUT_DIR/default_theme/mask_model_list_one.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 21) -h $(scale 21) $SRC_DIR/default_theme/mask_model_list_two.svg -o $OUT_DIR/default_theme/mask_model_list_two.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_model_logical_switches.svg -o $OUT_DIR/default_theme/mask_model_logical_switches.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_model_lua_scripts.svg -o $OUT_DIR/default_theme/mask_model_lua_scripts.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_model_mixer.svg -o $OUT_DIR/default_theme/mask_model_mixer.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_model_outputs.svg -o $OUT_DIR/default_theme/mask_model_outputs.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_model_setup.svg -o $OUT_DIR/default_theme/mask_model_setup.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_model_special_functions.svg -o $OUT_DIR/default_theme/mask_model_special_functions.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_model_telemetry.svg -o $OUT_DIR/default_theme/mask_model_telemetry.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_model_usb.svg -o $OUT_DIR/default_theme/mask_model_usb.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_monitor.svg -o $OUT_DIR/default_theme/mask_monitor.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 9) -h $(scale 13) $SRC_DIR/default_theme/mask_monitor_inver.svg -o $OUT_DIR/default_theme/mask_monitor_inver.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 9) -h $(scale 13) $SRC_DIR/default_theme/mask_monitor_lockch.svg -o $OUT_DIR/default_theme/mask_monitor_lockch.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_monitor_logsw.svg -o $OUT_DIR/default_theme/mask_monitor_logsw.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 20) -h $(scale 14) $SRC_DIR/default_theme/mask_mplex_add.svg -o $OUT_DIR/default_theme/mask_mplex_add.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 20) -h $(scale 14) $SRC_DIR/default_theme/mask_mplex_multi.svg -o $OUT_DIR/default_theme/mask_mplex_multi.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 20) -h $(scale 14) $SRC_DIR/default_theme/mask_mplex_replace.svg -o $OUT_DIR/default_theme/mask_mplex_replace.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_radio_calibration.svg -o $OUT_DIR/default_theme/mask_radio_calibration.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_radio_edit_theme.svg -o $OUT_DIR/default_theme/mask_radio_edit_theme.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_radio_global_functions.svg -o $OUT_DIR/default_theme/mask_radio_global_functions.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_radio_hardware.svg -o $OUT_DIR/default_theme/mask_radio_hardware.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_radio_sd_browser.svg -o $OUT_DIR/default_theme/mask_radio_sd_browser.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_radio_setup.svg -o $OUT_DIR/default_theme/mask_radio_setup.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_radio_tools.svg -o $OUT_DIR/default_theme/mask_radio_tools.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_radio_trainer.svg -o $OUT_DIR/default_theme/mask_radio_trainer.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_radio_version.svg -o $OUT_DIR/default_theme/mask_radio_version.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 77) -h $(scale 77) $SRC_DIR/default_theme/mask_shutdown.svg -o $OUT_DIR/default_theme/mask_shutdown.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_stats_analogs.svg -o $OUT_DIR/default_theme/mask_stats_analogs.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_stats_debug.svg -o $OUT_DIR/default_theme/mask_stats_debug.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_stats_timers.svg -o $OUT_DIR/default_theme/mask_stats_timers.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 14) -h $(scale 14) $SRC_DIR/default_theme/mask_textline_curve.svg -o $OUT_DIR/default_theme/mask_textline_curve.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 14) -h $(scale 14) $SRC_DIR/default_theme/mask_textline_fm.svg -o $OUT_DIR/default_theme/mask_textline_fm.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_theme_add_view.svg -o $OUT_DIR/default_theme/mask_theme_add_view.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_theme_setup.svg -o $OUT_DIR/default_theme/mask_theme_setup.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_theme_view1.svg -o $OUT_DIR/default_theme/mask_theme_view1.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_theme_view10.svg -o $OUT_DIR/default_theme/mask_theme_view10.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_theme_view2.svg -o $OUT_DIR/default_theme/mask_theme_view2.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_theme_view3.svg -o $OUT_DIR/default_theme/mask_theme_view3.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_theme_view4.svg -o $OUT_DIR/default_theme/mask_theme_view4.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_theme_view5.svg -o $OUT_DIR/default_theme/mask_theme_view5.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_theme_view6.svg -o $OUT_DIR/default_theme/mask_theme_view6.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_theme_view7.svg -o $OUT_DIR/default_theme/mask_theme_view7.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_theme_view8.svg -o $OUT_DIR/default_theme/mask_theme_view8.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_theme_view9.svg -o $OUT_DIR/default_theme/mask_theme_view9.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_tools_apps.svg -o $OUT_DIR/default_theme/mask_tools_apps.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 24) $SRC_DIR/default_theme/mask_tools_reset.svg -o $OUT_DIR/default_theme/mask_tools_reset.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 98) -h $(scale 20) $SRC_DIR/default_theme/mask_top_logo.svg -o $OUT_DIR/default_theme/mask_top_logo.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 36) -h $(scale 36) $SRC_DIR/default_theme/mask_topleft.svg -o $OUT_DIR/default_theme/mask_topleft.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 36) -h $(scale 36) $SRC_DIR/default_theme/mask_topright.svg -o $OUT_DIR/default_theme/mask_topright.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 12) -h $(scale 12) $SRC_DIR/default_theme/mask_trim.svg -o $OUT_DIR/default_theme/mask_trim.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 14) -h $(scale 14) $SRC_DIR/default_theme/mask_trim_shadow.svg -o $OUT_DIR/default_theme/mask_trim_shadow.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 14) -h $(scale 14) $SRC_DIR/mask_antenna.svg -o $OUT_DIR/mask_antenna.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 10) -h $(scale 10) $SRC_DIR/mask_dot.svg -o $OUT_DIR/mask_dot.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 3) -h $(scale 16) $SRC_DIR/mask_round_title_left.svg -o $OUT_DIR/mask_round_title_left.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 3) -h $(scale 16) $SRC_DIR/mask_round_title_right.svg -o $OUT_DIR/mask_round_title_right.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 60) -h $(scale 60) $SRC_DIR/mask_shutdown_circle0.svg -o $OUT_DIR/mask_shutdown_circle0.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 60) -h $(scale 60) $SRC_DIR/mask_shutdown_circle1.svg -o $OUT_DIR/mask_shutdown_circle1.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 60) -h $(scale 60) $SRC_DIR/mask_shutdown_circle2.svg -o $OUT_DIR/mask_shutdown_circle2.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 60) -h $(scale 60) $SRC_DIR/mask_shutdown_circle3.svg -o $OUT_DIR/mask_shutdown_circle3.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 50) -h $(scale 50) $SRC_DIR/mask_timer.svg -o $OUT_DIR/mask_timer.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 144) -h $(scale 56) $SRC_DIR/mask_timer_bg.svg -o $OUT_DIR/mask_timer_bg.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 14) -h $(scale 14) $SRC_DIR/mask_topmenu_gps_18.svg -o $OUT_DIR/mask_topmenu_gps_18.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 18) -h $(scale 8) $SRC_DIR/mask_topmenu_usb.svg -o $OUT_DIR/mask_topmenu_usb.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 19) -h $(scale 10) $SRC_DIR/mask_txbat.svg -o $OUT_DIR/mask_txbat.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 4) -h $(scale 12) $SRC_DIR/mask_txbat_charging.svg -o $OUT_DIR/mask_txbat_charging.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 169) -h $(scale 88) $SRC_DIR/mask_usb_symbol.svg -o $OUT_DIR/mask_usb_symbol.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 14) -h $(scale 13) $SRC_DIR/volume/mask_volume_0.svg -o $OUT_DIR/volume/mask_volume_0.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 12) -h $(scale 13) $SRC_DIR/volume/mask_volume_1.svg -o $OUT_DIR/volume/mask_volume_1.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 15) -h $(scale 13) $SRC_DIR/volume/mask_volume_2.svg -o $OUT_DIR/volume/mask_volume_2.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 19) -h $(scale 13) $SRC_DIR/volume/mask_volume_3.svg -o $OUT_DIR/volume/mask_volume_3.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 24) -h $(scale 13) $SRC_DIR/volume/mask_volume_4.svg -o $OUT_DIR/volume/mask_volume_4.png
inkscape -b FFFFFF --export-png-color-mode=RGB_8 -w $(scale 12) -h $(scale 13) $SRC_DIR/volume/mask_volume_scale.svg -o $OUT_DIR/volume/mask_volume_scale.png

echo "Done! Icons generated for $RESOLUTION"
