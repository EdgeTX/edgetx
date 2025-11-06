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

# Check if bc is installed (needed for scaling calculations)
if ! command -v bc &> /dev/null; then
    echo "Error: bc is not installed or not in PATH"
    echo "Please install bc (basic calculator) to use this script"
    echo ""
    echo "Installation instructions:"
    echo "  - Ubuntu/Debian: sudo apt-get install bc"
    echo "  - macOS: brew install bc"
    echo "  - Windows (Git Bash): bc is usually included"
    exit 1
fi

# Detect Inkscape version and set appropriate flags
# Skip GTK warnings and find the line that starts with "Inkscape"
INKSCAPE_VERSION=$(inkscape --version 2>&1 | grep -i "^Inkscape" | head -n1)
echo "Inkscape version: $INKSCAPE_VERSION"

# Extract major version number - try multiple methods for compatibility
INKSCAPE_MAJOR=$(echo "$INKSCAPE_VERSION" | sed -n 's/[^0-9]*\([0-9]\+\).*/\1/p')

# Default to legacy if we can't detect version
if [ -z "$INKSCAPE_MAJOR" ]; then
    echo "Warning: Could not detect Inkscape version, assuming legacy syntax"
    INKSCAPE_MAJOR=0
fi

# Suppress GTK/D-Bus warnings in headless environment by redirecting stderr to /dev/null for all inkscape calls
export DBUS_FATAL_WARNINGS=0

if [ "$INKSCAPE_MAJOR" -ge 1 ]; then
    # Inkscape 1.0+ uses -o for output
    echo "Using modern Inkscape syntax (version 1.0+)"
else
    # Inkscape 0.x uses --export-png
    echo "Using legacy Inkscape syntax (version 0.x)"
fi

# Parse resolution argument
RESOLUTION=${1:-"320x240"}

case $RESOLUTION in
    "320x240")
        SCALE=0.8
        ;;
    "480x272")
        SCALE=1.0
        ;;
    "800x480")
        SCALE=1.375
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

# Helper function to scale dimensions and round to nearest integer
scale() {
    echo "scale=2; ($1 * $SCALE + 0.5) / 1" | bc | cut -d. -f1
}

# Helper function to run inkscape with appropriate syntax
run_inkscape() {
    local width=$1
    local height=$2
    local input=$3
    local output=$4
    local extra_args=$5
    
    if [ "$INKSCAPE_MAJOR" -ge 1 ]; then
        # Inkscape 1.0+ syntax
        inkscape -w "$width" -h "$height" $extra_args "$input" -o "$output" 2>/dev/null
    else
        # Inkscape 0.x syntax
        inkscape -w "$width" -h "$height" $extra_args "$input" --export-png="$output" 2>/dev/null
    fi
}

echo "Converting SVG files to PNG..."

# Multi-color images (base dimensions for 480x272)
echo "  - Multi-color images..."
run_inkscape $(scale 111) $(scale 59) "$SRC_DIR/bootloader/bmp_plug_usb.svg" "$OUT_DIR/bootloader/bmp_plug_usb.png"
run_inkscape $(scale 52) $(scale 54) "$SRC_DIR/bootloader/bmp_usb_plugged.svg" "$OUT_DIR/bootloader/bmp_usb_plugged.png"
run_inkscape $(scale 90) $(scale 90) "$SRC_DIR/default_theme/alpha_stick_background.svg" "$OUT_DIR/default_theme/alpha_stick_background.png"
run_inkscape $(scale 20) $(scale 20) "$SRC_DIR/default_theme/alpha_stick_pointer.svg" "$OUT_DIR/default_theme/alpha_stick_pointer.png"
run_inkscape $(scale 271) $(scale 257) "$SRC_DIR/splash_logo.svg" "$OUT_DIR/splash_logo.png"

# Grey scale icons
echo "  - Greyscale mask icons..."
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_btn_close.svg" "$OUT_DIR/default_theme/mask_btn_close.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 26) $(scale 26) "$SRC_DIR/default_theme/mask_btn_next.svg" "$OUT_DIR/default_theme/mask_btn_next.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 26) $(scale 26) "$SRC_DIR/default_theme/mask_btn_prev.svg" "$OUT_DIR/default_theme/mask_btn_prev.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 96) $(scale 96) "$SRC_DIR/default_theme/mask_busy.svg" "$OUT_DIR/default_theme/mask_busy.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 34) $(scale 51) "$SRC_DIR/default_theme/mask_currentmenu_bg.svg" "$OUT_DIR/default_theme/mask_currentmenu_bg.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 13) $(scale 13) "$SRC_DIR/default_theme/mask_currentmenu_dot.svg" "$OUT_DIR/default_theme/mask_currentmenu_dot.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 36) $(scale 53) "$SRC_DIR/default_theme/mask_currentmenu_shadow.svg" "$OUT_DIR/default_theme/mask_currentmenu_shadow.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_edgetx.svg" "$OUT_DIR/default_theme/mask_edgetx.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 96) $(scale 96) "$SRC_DIR/default_theme/mask_error.svg" "$OUT_DIR/default_theme/mask_error.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_menu_model.svg" "$OUT_DIR/default_theme/mask_menu_model.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_menu_model_select.svg" "$OUT_DIR/default_theme/mask_menu_model_select.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_menu_notes.svg" "$OUT_DIR/default_theme/mask_menu_notes.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_menu_radio.svg" "$OUT_DIR/default_theme/mask_menu_radio.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_menu_stats.svg" "$OUT_DIR/default_theme/mask_menu_stats.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_menu_theme.svg" "$OUT_DIR/default_theme/mask_menu_theme.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_curves.svg" "$OUT_DIR/default_theme/mask_model_curves.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_flight_modes.svg" "$OUT_DIR/default_theme/mask_model_flight_modes.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 26) $(scale 26) "$SRC_DIR/default_theme/mask_model_grid_large.svg" "$OUT_DIR/default_theme/mask_model_grid_large.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 26) $(scale 26) "$SRC_DIR/default_theme/mask_model_grid_small.svg" "$OUT_DIR/default_theme/mask_model_grid_small.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_gvars.svg" "$OUT_DIR/default_theme/mask_model_gvars.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_heli.svg" "$OUT_DIR/default_theme/mask_model_heli.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_inputs.svg" "$OUT_DIR/default_theme/mask_model_inputs.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 26) $(scale 26) "$SRC_DIR/default_theme/mask_model_list_one.svg" "$OUT_DIR/default_theme/mask_model_list_one.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 26) $(scale 26) "$SRC_DIR/default_theme/mask_model_list_two.svg" "$OUT_DIR/default_theme/mask_model_list_two.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_logical_switches.svg" "$OUT_DIR/default_theme/mask_model_logical_switches.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_lua_scripts.svg" "$OUT_DIR/default_theme/mask_model_lua_scripts.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_mixer.svg" "$OUT_DIR/default_theme/mask_model_mixer.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_outputs.svg" "$OUT_DIR/default_theme/mask_model_outputs.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_setup.svg" "$OUT_DIR/default_theme/mask_model_setup.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_special_functions.svg" "$OUT_DIR/default_theme/mask_model_special_functions.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_telemetry.svg" "$OUT_DIR/default_theme/mask_model_telemetry.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_usb.svg" "$OUT_DIR/default_theme/mask_model_usb.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_monitor.svg" "$OUT_DIR/default_theme/mask_monitor.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 11) $(scale 16) "$SRC_DIR/default_theme/mask_monitor_inver.svg" "$OUT_DIR/default_theme/mask_monitor_inver.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 11) $(scale 16) "$SRC_DIR/default_theme/mask_monitor_lockch.svg" "$OUT_DIR/default_theme/mask_monitor_lockch.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_monitor_logsw.svg" "$OUT_DIR/default_theme/mask_monitor_logsw.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 25) $(scale 17) "$SRC_DIR/default_theme/mask_mplex_add.svg" "$OUT_DIR/default_theme/mask_mplex_add.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 25) $(scale 17) "$SRC_DIR/default_theme/mask_mplex_multi.svg" "$OUT_DIR/default_theme/mask_mplex_multi.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 25) $(scale 17) "$SRC_DIR/default_theme/mask_mplex_replace.svg" "$OUT_DIR/default_theme/mask_mplex_replace.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_calibration.svg" "$OUT_DIR/default_theme/mask_radio_calibration.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_edit_theme.svg" "$OUT_DIR/default_theme/mask_radio_edit_theme.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_global_functions.svg" "$OUT_DIR/default_theme/mask_radio_global_functions.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_hardware.svg" "$OUT_DIR/default_theme/mask_radio_hardware.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_sd_browser.svg" "$OUT_DIR/default_theme/mask_radio_sd_browser.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_setup.svg" "$OUT_DIR/default_theme/mask_radio_setup.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_tools.svg" "$OUT_DIR/default_theme/mask_radio_tools.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_trainer.svg" "$OUT_DIR/default_theme/mask_radio_trainer.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_version.svg" "$OUT_DIR/default_theme/mask_radio_version.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 96) $(scale 96) "$SRC_DIR/default_theme/mask_shutdown.svg" "$OUT_DIR/default_theme/mask_shutdown.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_stats_analogs.svg" "$OUT_DIR/default_theme/mask_stats_analogs.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_stats_debug.svg" "$OUT_DIR/default_theme/mask_stats_debug.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_stats_timers.svg" "$OUT_DIR/default_theme/mask_stats_timers.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 17) $(scale 17) "$SRC_DIR/default_theme/mask_textline_curve.svg" "$OUT_DIR/default_theme/mask_textline_curve.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 17) $(scale 17) "$SRC_DIR/default_theme/mask_textline_fm.svg" "$OUT_DIR/default_theme/mask_textline_fm.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_add_view.svg" "$OUT_DIR/default_theme/mask_theme_add_view.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_setup.svg" "$OUT_DIR/default_theme/mask_theme_setup.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view1.svg" "$OUT_DIR/default_theme/mask_theme_view1.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view10.svg" "$OUT_DIR/default_theme/mask_theme_view10.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view2.svg" "$OUT_DIR/default_theme/mask_theme_view2.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view3.svg" "$OUT_DIR/default_theme/mask_theme_view3.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view4.svg" "$OUT_DIR/default_theme/mask_theme_view4.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view5.svg" "$OUT_DIR/default_theme/mask_theme_view5.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view6.svg" "$OUT_DIR/default_theme/mask_theme_view6.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view7.svg" "$OUT_DIR/default_theme/mask_theme_view7.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view8.svg" "$OUT_DIR/default_theme/mask_theme_view8.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view9.svg" "$OUT_DIR/default_theme/mask_theme_view9.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_tools_apps.svg" "$OUT_DIR/default_theme/mask_tools_apps.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_tools_reset.svg" "$OUT_DIR/default_theme/mask_tools_reset.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 122) $(scale 25) "$SRC_DIR/default_theme/mask_top_logo.svg" "$OUT_DIR/default_theme/mask_top_logo.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 45) $(scale 45) "$SRC_DIR/default_theme/mask_topleft.svg" "$OUT_DIR/default_theme/mask_topleft.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 45) $(scale 45) "$SRC_DIR/default_theme/mask_topright.svg" "$OUT_DIR/default_theme/mask_topright.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 15) $(scale 15) "$SRC_DIR/default_theme/mask_trim.svg" "$OUT_DIR/default_theme/mask_trim.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 17) $(scale 17) "$SRC_DIR/default_theme/mask_trim_shadow.svg" "$OUT_DIR/default_theme/mask_trim_shadow.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 18) $(scale 17) "$SRC_DIR/mask_antenna.svg" "$OUT_DIR/mask_antenna.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 13) $(scale 13) "$SRC_DIR/mask_dot.svg" "$OUT_DIR/mask_dot.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 4) $(scale 20) "$SRC_DIR/mask_round_title_left.svg" "$OUT_DIR/mask_round_title_left.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 4) $(scale 20) "$SRC_DIR/mask_round_title_right.svg" "$OUT_DIR/mask_round_title_right.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 75) $(scale 75) "$SRC_DIR/mask_shutdown_circle0.svg" "$OUT_DIR/mask_shutdown_circle0.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 75) $(scale 75) "$SRC_DIR/mask_shutdown_circle1.svg" "$OUT_DIR/mask_shutdown_circle1.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 75) $(scale 75) "$SRC_DIR/mask_shutdown_circle2.svg" "$OUT_DIR/mask_shutdown_circle2.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 75) $(scale 75) "$SRC_DIR/mask_shutdown_circle3.svg" "$OUT_DIR/mask_shutdown_circle3.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 62) $(scale 62) "$SRC_DIR/mask_timer.svg" "$OUT_DIR/mask_timer.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 180) $(scale 70) "$SRC_DIR/mask_timer_bg.svg" "$OUT_DIR/mask_timer_bg.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 18) $(scale 18) "$SRC_DIR/mask_topmenu_gps_18.svg" "$OUT_DIR/mask_topmenu_gps_18.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 22) $(scale 10) "$SRC_DIR/mask_topmenu_usb.svg" "$OUT_DIR/mask_topmenu_usb.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 24) $(scale 12) "$SRC_DIR/mask_txbat.svg" "$OUT_DIR/mask_txbat.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 5) $(scale 15) "$SRC_DIR/mask_txbat_charging.svg" "$OUT_DIR/mask_txbat_charging.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 211) $(scale 110) "$SRC_DIR/mask_usb_symbol.svg" "$OUT_DIR/mask_usb_symbol.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 18) $(scale 16) "$SRC_DIR/volume/mask_volume_0.svg" "$OUT_DIR/volume/mask_volume_0.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 15) $(scale 16) "$SRC_DIR/volume/mask_volume_1.svg" "$OUT_DIR/volume/mask_volume_1.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 19) $(scale 16) "$SRC_DIR/volume/mask_volume_2.svg" "$OUT_DIR/volume/mask_volume_2.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 24) $(scale 16) "$SRC_DIR/volume/mask_volume_3.svg" "$OUT_DIR/volume/mask_volume_3.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 30) $(scale 16) "$SRC_DIR/volume/mask_volume_4.svg" "$OUT_DIR/volume/mask_volume_4.png" "-b FFFFFF --export-png-color-mode=RGB_8"
run_inkscape $(scale 15) $(scale 16) "$SRC_DIR/volume/mask_volume_scale.svg" "$OUT_DIR/volume/mask_volume_scale.png" "-b FFFFFF --export-png-color-mode=RGB_8"

echo "Done! Icons generated for $RESOLUTION"
