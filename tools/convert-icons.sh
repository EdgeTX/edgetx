#!/bin/bash

# Convert source SVG files to PNG icons for different screen resolutions
# Usage: ./convert_icons.sh [320x240|480x272|800x480|all] [additional resolutions...]
# Examples:
#   ./convert_icons.sh                    # Generates 480x272 (default)
#   ./convert_icons.sh 320x240            # Generates 320x240
#   ./convert_icons.sh all                # Generates all resolutions
#   ./convert_icons.sh 320x240 480x272    # Generates 320x240 and 480x272
# Requires: resvg command line tool

set -e  # Exit on error

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Check if resvg is installed
if ! command -v resvg &> /dev/null; then
    echo "Error: resvg is not installed or not in PATH"
    echo "Please install resvg to use this script"
    echo ""
    echo "Installation instructions:"
    echo "  - Ubuntu/Debian: sudo apt-get install resvg"
    echo "  - macOS: brew install resvg"
    echo "  - Windows: Download from https://github.com/RazrFalcon/resvg/releases"
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

# Define supported resolutions and their scales
declare -A RESOLUTIONS=(
    ["320x240"]=0.8
    ["480x272"]=1.0
    ["800x480"]=1.375
)

# Helper function to scale dimensions and round to nearest integer
scale() {
    echo "scale=2; ($1 * $SCALE + 0.5) / 1" | bc | cut -d. -f1
}

# Helper function to run resvg with appropriate options
# Automatically applies white background for files starting with mask_
run_resvg() {
    local width=$1
    local height=$2
    local input=$3
    local output=$4
    
    # Check if the output filename starts with "mask_" and apply white background
    local bg_option=""
    if [[ $(basename "$output") == mask_* ]]; then
        bg_option="--background white"
    fi
    
    resvg $bg_option --width "$width" --height "$height" "$input" "$output"
}

# Process a single resolution
process_resolution() {
    local RESOLUTION=$1
    local SCALE=${RESOLUTIONS[$RESOLUTION]}
    
    if [ -z "$SCALE" ]; then
        echo "Error: Unsupported resolution '$RESOLUTION'"
        echo "Supported resolutions: ${!RESOLUTIONS[@]}"
        return 1
    fi

    echo "Generating icons for resolution: $RESOLUTION (scale: ${SCALE}x)"

    # Base paths (relative to script location)
    SRC_DIR="$SCRIPT_DIR/../radio/src/bitmaps/img-src"
    OUT_DIR="$SCRIPT_DIR/../radio/src/bitmaps/$RESOLUTION"

    # Check if directories exist
    if [ ! -d "$SRC_DIR" ]; then
        echo "Error: Source directory not found: $SRC_DIR"
        return 1
    fi

    if [ ! -d "$OUT_DIR" ]; then
        echo "Error: Output directory not found: $OUT_DIR"
        return 1
    fi

    echo "Converting SVG files to PNG..."

    # Multi-color images (base dimensions for 480x272)
    echo "  - Multi-color images..."
    run_resvg $(scale 111) $(scale 59) "$SRC_DIR/bootloader/bmp_plug_usb.svg" "$OUT_DIR/bootloader/bmp_plug_usb.png"
    run_resvg $(scale 52) $(scale 54) "$SRC_DIR/bootloader/bmp_usb_plugged.svg" "$OUT_DIR/bootloader/bmp_usb_plugged.png"
    run_resvg $(scale 90) $(scale 90) "$SRC_DIR/default_theme/alpha_stick_background.svg" "$OUT_DIR/default_theme/alpha_stick_background.png"
    run_resvg $(scale 20) $(scale 20) "$SRC_DIR/default_theme/alpha_stick_pointer.svg" "$OUT_DIR/default_theme/alpha_stick_pointer.png"
    run_resvg $(scale 271) $(scale 257) "$SRC_DIR/splash_logo.svg" "$OUT_DIR/splash_logo.png"

    # Grey scale icons
    echo "  - Greyscale mask icons..."
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_btn_close.svg" "$OUT_DIR/default_theme/mask_btn_close.png"
    run_resvg $(scale 26) $(scale 26) "$SRC_DIR/default_theme/mask_btn_next.svg" "$OUT_DIR/default_theme/mask_btn_next.png"
    run_resvg $(scale 26) $(scale 26) "$SRC_DIR/default_theme/mask_btn_prev.svg" "$OUT_DIR/default_theme/mask_btn_prev.png"
    run_resvg $(scale 96) $(scale 96) "$SRC_DIR/default_theme/mask_busy.svg" "$OUT_DIR/default_theme/mask_busy.png"
    run_resvg $(scale 34) $(scale 51) "$SRC_DIR/default_theme/mask_currentmenu_bg.svg" "$OUT_DIR/default_theme/mask_currentmenu_bg.png"
    run_resvg $(scale 13) $(scale 13) "$SRC_DIR/default_theme/mask_currentmenu_dot.svg" "$OUT_DIR/default_theme/mask_currentmenu_dot.png"
    run_resvg $(scale 36) $(scale 53) "$SRC_DIR/default_theme/mask_currentmenu_shadow.svg" "$OUT_DIR/default_theme/mask_currentmenu_shadow.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_edgetx.svg" "$OUT_DIR/default_theme/mask_edgetx.png"
    run_resvg $(scale 96) $(scale 96) "$SRC_DIR/default_theme/mask_error.svg" "$OUT_DIR/default_theme/mask_error.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_menu_model.svg" "$OUT_DIR/default_theme/mask_menu_model.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_menu_model_select.svg" "$OUT_DIR/default_theme/mask_menu_model_select.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_menu_notes.svg" "$OUT_DIR/default_theme/mask_menu_notes.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_menu_radio.svg" "$OUT_DIR/default_theme/mask_menu_radio.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_menu_stats.svg" "$OUT_DIR/default_theme/mask_menu_stats.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_menu_theme.svg" "$OUT_DIR/default_theme/mask_menu_theme.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_curves.svg" "$OUT_DIR/default_theme/mask_model_curves.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_flight_modes.svg" "$OUT_DIR/default_theme/mask_model_flight_modes.png"
    run_resvg $(scale 26) $(scale 26) "$SRC_DIR/default_theme/mask_model_grid_large.svg" "$OUT_DIR/default_theme/mask_model_grid_large.png"
    run_resvg $(scale 26) $(scale 26) "$SRC_DIR/default_theme/mask_model_grid_small.svg" "$OUT_DIR/default_theme/mask_model_grid_small.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_gvars.svg" "$OUT_DIR/default_theme/mask_model_gvars.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_heli.svg" "$OUT_DIR/default_theme/mask_model_heli.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_inputs.svg" "$OUT_DIR/default_theme/mask_model_inputs.png"
    run_resvg $(scale 26) $(scale 26) "$SRC_DIR/default_theme/mask_model_list_one.svg" "$OUT_DIR/default_theme/mask_model_list_one.png"
    run_resvg $(scale 26) $(scale 26) "$SRC_DIR/default_theme/mask_model_list_two.svg" "$OUT_DIR/default_theme/mask_model_list_two.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_logical_switches.svg" "$OUT_DIR/default_theme/mask_model_logical_switches.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_lua_scripts.svg" "$OUT_DIR/default_theme/mask_model_lua_scripts.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_mixer.svg" "$OUT_DIR/default_theme/mask_model_mixer.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_outputs.svg" "$OUT_DIR/default_theme/mask_model_outputs.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_setup.svg" "$OUT_DIR/default_theme/mask_model_setup.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_special_functions.svg" "$OUT_DIR/default_theme/mask_model_special_functions.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_telemetry.svg" "$OUT_DIR/default_theme/mask_model_telemetry.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_model_usb.svg" "$OUT_DIR/default_theme/mask_model_usb.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_monitor.svg" "$OUT_DIR/default_theme/mask_monitor.png"
    run_resvg $(scale 11) $(scale 16) "$SRC_DIR/default_theme/mask_monitor_inver.svg" "$OUT_DIR/default_theme/mask_monitor_inver.png"
    run_resvg $(scale 11) $(scale 16) "$SRC_DIR/default_theme/mask_monitor_lockch.svg" "$OUT_DIR/default_theme/mask_monitor_lockch.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_monitor_logsw.svg" "$OUT_DIR/default_theme/mask_monitor_logsw.png"
    run_resvg $(scale 25) $(scale 17) "$SRC_DIR/default_theme/mask_mplex_add.svg" "$OUT_DIR/default_theme/mask_mplex_add.png"
    run_resvg $(scale 25) $(scale 17) "$SRC_DIR/default_theme/mask_mplex_multi.svg" "$OUT_DIR/default_theme/mask_mplex_multi.png"
    run_resvg $(scale 25) $(scale 17) "$SRC_DIR/default_theme/mask_mplex_replace.svg" "$OUT_DIR/default_theme/mask_mplex_replace.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_calibration.svg" "$OUT_DIR/default_theme/mask_radio_calibration.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_edit_theme.svg" "$OUT_DIR/default_theme/mask_radio_edit_theme.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_global_functions.svg" "$OUT_DIR/default_theme/mask_radio_global_functions.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_hardware.svg" "$OUT_DIR/default_theme/mask_radio_hardware.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_sd_browser.svg" "$OUT_DIR/default_theme/mask_radio_sd_browser.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_setup.svg" "$OUT_DIR/default_theme/mask_radio_setup.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_tools.svg" "$OUT_DIR/default_theme/mask_radio_tools.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_trainer.svg" "$OUT_DIR/default_theme/mask_radio_trainer.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_radio_version.svg" "$OUT_DIR/default_theme/mask_radio_version.png"
    run_resvg $(scale 96) $(scale 96) "$SRC_DIR/default_theme/mask_shutdown.svg" "$OUT_DIR/default_theme/mask_shutdown.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_stats_analogs.svg" "$OUT_DIR/default_theme/mask_stats_analogs.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_stats_debug.svg" "$OUT_DIR/default_theme/mask_stats_debug.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_stats_timers.svg" "$OUT_DIR/default_theme/mask_stats_timers.png"
    run_resvg $(scale 17) $(scale 17) "$SRC_DIR/default_theme/mask_textline_curve.svg" "$OUT_DIR/default_theme/mask_textline_curve.png"
    run_resvg $(scale 17) $(scale 17) "$SRC_DIR/default_theme/mask_textline_fm.svg" "$OUT_DIR/default_theme/mask_textline_fm.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_add_view.svg" "$OUT_DIR/default_theme/mask_theme_add_view.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_setup.svg" "$OUT_DIR/default_theme/mask_theme_setup.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view1.svg" "$OUT_DIR/default_theme/mask_theme_view1.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view10.svg" "$OUT_DIR/default_theme/mask_theme_view10.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view2.svg" "$OUT_DIR/default_theme/mask_theme_view2.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view3.svg" "$OUT_DIR/default_theme/mask_theme_view3.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view4.svg" "$OUT_DIR/default_theme/mask_theme_view4.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view5.svg" "$OUT_DIR/default_theme/mask_theme_view5.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view6.svg" "$OUT_DIR/default_theme/mask_theme_view6.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view7.svg" "$OUT_DIR/default_theme/mask_theme_view7.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view8.svg" "$OUT_DIR/default_theme/mask_theme_view8.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_theme_view9.svg" "$OUT_DIR/default_theme/mask_theme_view9.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_tools_apps.svg" "$OUT_DIR/default_theme/mask_tools_apps.png"
    run_resvg $(scale 30) $(scale 30) "$SRC_DIR/default_theme/mask_tools_reset.svg" "$OUT_DIR/default_theme/mask_tools_reset.png"
    run_resvg $(scale 122) $(scale 25) "$SRC_DIR/default_theme/mask_top_logo.svg" "$OUT_DIR/default_theme/mask_top_logo.png"
    run_resvg $(scale 45) $(scale 45) "$SRC_DIR/default_theme/mask_topleft.svg" "$OUT_DIR/default_theme/mask_topleft.png"
    run_resvg $(scale 45) $(scale 45) "$SRC_DIR/default_theme/mask_topright.svg" "$OUT_DIR/default_theme/mask_topright.png"
    run_resvg $(scale 15) $(scale 15) "$SRC_DIR/default_theme/mask_trim.svg" "$OUT_DIR/default_theme/mask_trim.png"
    run_resvg $(scale 17) $(scale 17) "$SRC_DIR/default_theme/mask_trim_shadow.svg" "$OUT_DIR/default_theme/mask_trim_shadow.png"
    run_resvg $(scale 18) $(scale 17) "$SRC_DIR/mask_antenna.svg" "$OUT_DIR/mask_antenna.png"
    run_resvg $(scale 13) $(scale 13) "$SRC_DIR/mask_dot.svg" "$OUT_DIR/mask_dot.png"
    run_resvg $(scale 4) $(scale 20) "$SRC_DIR/mask_round_title_left.svg" "$OUT_DIR/mask_round_title_left.png"
    run_resvg $(scale 4) $(scale 20) "$SRC_DIR/mask_round_title_right.svg" "$OUT_DIR/mask_round_title_right.png"
    run_resvg $(scale 75) $(scale 75) "$SRC_DIR/mask_shutdown_circle0.svg" "$OUT_DIR/mask_shutdown_circle0.png"
    run_resvg $(scale 75) $(scale 75) "$SRC_DIR/mask_shutdown_circle1.svg" "$OUT_DIR/mask_shutdown_circle1.png"
    run_resvg $(scale 75) $(scale 75) "$SRC_DIR/mask_shutdown_circle2.svg" "$OUT_DIR/mask_shutdown_circle2.png"
    run_resvg $(scale 75) $(scale 75) "$SRC_DIR/mask_shutdown_circle3.svg" "$OUT_DIR/mask_shutdown_circle3.png"
    run_resvg $(scale 62) $(scale 62) "$SRC_DIR/mask_timer.svg" "$OUT_DIR/mask_timer.png"
    run_resvg $(scale 180) $(scale 70) "$SRC_DIR/mask_timer_bg.svg" "$OUT_DIR/mask_timer_bg.png"
    run_resvg $(scale 18) $(scale 18) "$SRC_DIR/mask_topmenu_gps_18.svg" "$OUT_DIR/mask_topmenu_gps_18.png"
    run_resvg $(scale 22) $(scale 10) "$SRC_DIR/mask_topmenu_usb.svg" "$OUT_DIR/mask_topmenu_usb.png"
    run_resvg $(scale 24) $(scale 12) "$SRC_DIR/mask_txbat.svg" "$OUT_DIR/mask_txbat.png"
    run_resvg $(scale 5) $(scale 15) "$SRC_DIR/mask_txbat_charging.svg" "$OUT_DIR/mask_txbat_charging.png"
    run_resvg $(scale 211) $(scale 110) "$SRC_DIR/mask_usb_symbol.svg" "$OUT_DIR/mask_usb_symbol.png"
    run_resvg $(scale 18) $(scale 16) "$SRC_DIR/volume/mask_volume_0.svg" "$OUT_DIR/volume/mask_volume_0.png"
    run_resvg $(scale 15) $(scale 16) "$SRC_DIR/volume/mask_volume_1.svg" "$OUT_DIR/volume/mask_volume_1.png"
    run_resvg $(scale 19) $(scale 16) "$SRC_DIR/volume/mask_volume_2.svg" "$OUT_DIR/volume/mask_volume_2.png"
    run_resvg $(scale 24) $(scale 16) "$SRC_DIR/volume/mask_volume_3.svg" "$OUT_DIR/volume/mask_volume_3.png"
    run_resvg $(scale 30) $(scale 16) "$SRC_DIR/volume/mask_volume_4.svg" "$OUT_DIR/volume/mask_volume_4.png"
    run_resvg $(scale 15) $(scale 16) "$SRC_DIR/volume/mask_volume_scale.svg" "$OUT_DIR/volume/mask_volume_scale.png"

    echo "Done! Icons generated for $RESOLUTION"
}

# Parse command line arguments
REQUESTED_RESOLUTIONS=()

if [ $# -eq 0 ]; then
    # No arguments, use default
    REQUESTED_RESOLUTIONS=("480x272")
elif [ "$1" = "all" ]; then
    # Process all supported resolutions
    REQUESTED_RESOLUTIONS=("${!RESOLUTIONS[@]}")
else
    # Use provided resolutions
    REQUESTED_RESOLUTIONS=("$@")
fi

# Process each requested resolution
for resolution in "${REQUESTED_RESOLUTIONS[@]}"; do
    process_resolution "$resolution" || exit 1
done

echo "All done!"
