#!/bin/bash

set -e  # Exit on error

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Define global paths
SRC_LIST="$SCRIPT_DIR/convert-gfx-list.csv"
SRC_DIR="$SCRIPT_DIR/../radio/src/bitmaps/img-src"

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

# Default conversion engine
CONVERSION_ENGINE="rsvg-convert"

# Update mode flag
UPDATE_MODE=false

# Function to check if conversion engine is available
check_conversion_engine() {
    if [ "$CONVERSION_ENGINE" = "resvg" ]; then
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
    elif [ "$CONVERSION_ENGINE" = "rsvg-convert" ]; then
        if ! command -v rsvg-convert &> /dev/null; then
            echo "Error: rsvg-convert is not installed or not in PATH"
            echo "Please install rsvg-convert to use this script"
            echo ""
            echo "Installation instructions:"
            echo "  - Ubuntu/Debian: sudo apt-get install librsvg2-bin"
            echo "  - macOS: brew install librsvg"
            echo "  - Windows (MSYS2): pacman -S mingw-w64-x86_64-librsvg"
            exit 1
        fi
    else
        echo "Error: Unknown conversion engine '$CONVERSION_ENGINE'"
        echo "Supported engines: resvg, rsvg-convert"
        exit 1
    fi
}

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

# Check if CSV file list exists
if [ ! -f "$SRC_LIST" ]; then
    echo "Error: CSV files list not found: $SRC_LIST"
    exit 1
fi

# Check if SVG source directory exists
if [ ! -d "$SRC_DIR" ]; then
    echo "Error: SVG source directory not found: $SRC_DIR"
    exit 1
fi

# Define supported resolutions and their scales
# Using parallel arrays for bash 3.x compatibility (macOS)
RESOLUTIONS_LIST=("320x240" "480x272" "800x480")
RESOLUTIONS_SCALE=("0.8" "1.0" "1.375")

# Helper function to get scale for a resolution
get_scale() {
    local resolution=$1
    for i in "${!RESOLUTIONS_LIST[@]}"; do
        if [[ "${RESOLUTIONS_LIST[$i]}" == "$resolution" ]]; then
            echo "${RESOLUTIONS_SCALE[$i]}"
            return 0
        fi
    done
    return 1
}

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
    # local bg_option=""
    # if [[ $(basename "$output") == mask_* ]]; then
    #     bg_option="--background white"
    # fi
    
    # resvg $bg_option --width "$width" --height "$height" "$input" "$output"
    resvg --width "$width" --height "$height" "$input" "$output"
}

run_rsvg() {
    local width=$1
    local height=$2
    local input=$3
    local output=$4
    
    # Check if the output filename starts with "mask_" and apply white background
    # local bg_option=""
    # if [[ $(basename "$output") == mask_* ]]; then
    #     bg_option="--background white"
    # fi
    
    # rsvg-convert $bg_option --width "$width" --height "$height" "$input" "$output"
    rsvg-convert --width "$width" --height "$height" --output "$output" "$input"
}

# Process a single resolution
process_resolution() {
    local RESOLUTION=$1
    local SCALE
    SCALE=$(get_scale "$RESOLUTION")
    
    if [ -z "$SCALE" ]; then
        echo "Error: Unsupported resolution '$RESOLUTION'"
        echo "Supported resolutions: ${RESOLUTIONS_LIST[@]}"
        return 1
    fi

    echo "Generating icons for resolution: $RESOLUTION (scale: ${SCALE}x)"

    # Base paths (relative to script location)
    OUT_DIR="$SCRIPT_DIR/../radio/src/bitmaps/$RESOLUTION"

    # Check if output directory exists
    if [ ! -d "$OUT_DIR" ]; then
        echo "Error: Output directory not found: $OUT_DIR"
        return 1
    fi

    if [ "$UPDATE_MODE" = true ]; then
        echo "Converting SVG files to PNG using $CONVERSION_ENGINE (update mode - only changed files)..."
    else
        echo "Converting SVG files to PNG using $CONVERSION_ENGINE..."
    fi

    # Read CSV file and process each line (skip header)
    local line_num=0
    local has_errors=0
    local processed_count=0
    local skipped_count=0
    
    while IFS=';' read -r file width height csv_date || [ -n "$file" ]; do
        line_num=$((line_num + 1))
        
        # Skip header line
        if [ $line_num -eq 1 ]; then
            continue
        fi
        
        # Skip empty lines
        if [ -z "$file" ]; then
            continue
        fi
        
        # Check if width or height is missing
        if [ -z "$width" ] || [ -z "$height" ]; then
            echo -e "  ${RED}Error: Skipping $file.svg - missing dimension(s) (width: '$width', height: '$height')${NC}"
            has_errors=1
            continue
        fi
        
        # In update mode, check if SVG is newer than CSV date or PNG file
        if [ "$UPDATE_MODE" = true ]; then
            svg_path="$SRC_DIR/$file.svg"
            png_path="$OUT_DIR/$file.png"
            
            if [ ! -f "$svg_path" ]; then
                echo -e "  ${RED}Error: SVG file not found: $file.svg${NC}"
                has_errors=1
                continue
            fi
            
            # Get SVG modification date
            svg_mod_date=$(stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" "$svg_path" 2>/dev/null)
            if [ -z "$svg_mod_date" ]; then
                # Fallback for Linux
                svg_mod_date=$(stat -c "%y" "$svg_path" 2>/dev/null | cut -d'.' -f1)
            fi
            
            # Check if PNG exists and get its modification date
            should_process=false
            
            if [ ! -f "$png_path" ]; then
                # PNG doesn't exist, needs to be generated
                echo "  Adding new: $file.svg (PNG missing for $RESOLUTION)"
                should_process=true
            elif [ -z "$csv_date" ]; then
                # CSV date is empty, treat as newly added file
                echo "  Adding new: $file.svg (no previous date in CSV)"
                should_process=true
            else
                # PNG exists and CSV has a date - compare SVG date with CSV date
                # This allows regeneration if SVG is newer, regardless of which resolution was previously built
                if [[ "$svg_mod_date" > "$csv_date" ]]; then
                    echo "  Updating: $file.svg (SVG: $svg_mod_date, CSV: $csv_date)"
                    should_process=true
                else
                    # Also check if PNG is older than SVG (handles case where PNG was deleted)
                    png_mod_date=$(stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" "$png_path" 2>/dev/null)
                    if [ -z "$png_mod_date" ]; then
                        # Fallback for Linux
                        png_mod_date=$(stat -c "%y" "$png_path" 2>/dev/null | cut -d'.' -f1)
                    fi
                    
                    if [ -n "$png_mod_date" ] && [[ "$png_mod_date" < "$svg_mod_date" ]]; then
                        echo "  Updating: $file.svg (PNG older than SVG for $RESOLUTION)"
                        should_process=true
                    else
                        skipped_count=$((skipped_count + 1))
                        continue
                    fi
                fi
            fi
            
            if [ "$should_process" = false ]; then
                skipped_count=$((skipped_count + 1))
                continue
            fi
        fi
        
        # Run conversion using selected engine
        if [ "$CONVERSION_ENGINE" = "resvg" ]; then
            run_resvg $(scale "$width") $(scale "$height") "$SRC_DIR/$file.svg" "$OUT_DIR/$file.png"
        else
            run_rsvg $(scale "$width") $(scale "$height") "$SRC_DIR/$file.svg" "$OUT_DIR/$file.png"
        fi
        
        processed_count=$((processed_count + 1))
    done < "$SRC_LIST"

    if [ "$UPDATE_MODE" = true ]; then
        echo "  Processed: $processed_count, Skipped (up-to-date): $skipped_count"
    fi

    if [ $has_errors -eq 1 ]; then
        echo ""
        echo -e "${RED}Warning: Some files were skipped due to missing dimensions${NC}"
        echo "Please run --validate png to see all entries with missing dimensions"
    fi

    echo "Done! Icons generated for $RESOLUTION"
}

# Validate that all SVG sources exist in CSV and vice versa
validate_svg() {
    echo "Validating SVG source files"
    echo "---------------------------"
    
    local has_errors=0
    local has_dimension_errors=0
    
    # Part 0: Check for missing dimensions in CSV
    echo ""
    echo "Checking CSV for missing dimensions..."
    local line_num=0
    local missing_dimensions=0
    
    while IFS=';' read -r file width height || [ -n "$file" ]; do
        line_num=$((line_num + 1))
        
        # Skip header line
        if [ $line_num -eq 1 ]; then
            continue
        fi
        
        # Skip empty lines
        if [ -z "$file" ]; then
            continue
        fi
        
        # Check if width or height is empty
        if [ -z "$width" ] || [ -z "$height" ]; then
            echo -e "  ${RED}CSV entry missing dimensions: $file (width: '$width', height: '$height')${NC}"
            has_dimension_errors=1
            has_errors=1
            missing_dimensions=$((missing_dimensions + 1))
        fi
    done < "$SRC_LIST"
    
    if [ $missing_dimensions -gt 0 ]; then
        echo "  Total entries with missing dimensions: $missing_dimensions"
    else
        echo -e "  ${GREEN}All CSV entries have dimensions${NC}"
    fi
    
    # Part 1: Check CSV entries against SVG directory
    echo ""
    echo "Comparing CSV files list with SVG directory..."
    line_num=0
    local missing_in_dir=0
    local found_in_dir=0
    
    while IFS=';' read -r file width height || [ -n "$file" ]; do
        line_num=$((line_num + 1))
        
        # Skip header line
        if [ $line_num -eq 1 ]; then
            continue
        fi
        
        # Skip empty lines
        if [ -z "$file" ]; then
            continue
        fi
        
        # Check if SVG exists
        if [ ! -f "$SRC_DIR/$file.svg" ]; then
            echo -e "  ${RED}CSV entry missing SVG file: $file.svg${NC}"
            has_errors=1
            missing_in_dir=$((missing_in_dir + 1))
        else
            found_in_dir=$((found_in_dir + 1))
        fi
    done < "$SRC_LIST"
    
    echo "  CSV entries - Found: $found_in_dir, Missing in SVG dir: $missing_in_dir"
    
    # Part 2: Check SVG directory against CSV entries
    
    # Part 2: Check SVG directory against CSV entries
    echo ""
    echo "Comparing SVG directory with CSV files list..."
    
    # Build a list of files from CSV (without .svg extension)
    local csv_files=()
    line_num=0
    while IFS=';' read -r file width height || [ -n "$file" ]; do
        line_num=$((line_num + 1))
        
        # Skip header line
        if [ $line_num -eq 1 ]; then
            continue
        fi
        
        # Skip empty lines
        if [ -z "$file" ]; then
            continue
        fi
        
        csv_files+=("$file")
    done < "$SRC_LIST"
    
    # Find all SVG files in directory and check if they're in CSV
    local missing_in_csv=0
    local found_in_csv=0
    
    find "$SRC_DIR" -name "*.svg" -type f | while read -r svg_path; do
        # Get relative path from SRC_DIR and remove .svg extension
        local rel_path="${svg_path#$SRC_DIR/}"
        local file_base="${rel_path%.svg}"
        
        # Check if this file is in CSV list
        local in_csv=0
        for csv_file in "${csv_files[@]}"; do
            if [ "$csv_file" = "$file_base" ]; then
                in_csv=1
                break
            fi
        done
        
        if [ $in_csv -eq 0 ]; then
            echo -e "  ${RED}SVG file missing in CSV list: $file_base.svg${NC}"
            has_errors=1
            missing_in_csv=$((missing_in_csv + 1))
        else
            found_in_csv=$((found_in_csv + 1))
        fi
    done
    
    # Count total SVG files
    local total_svg_files=$(find "$SRC_DIR" -name "*.svg" -type f | wc -l | tr -d ' ')
    local svg_missing_in_csv=$((total_svg_files - found_in_dir))
    
    echo "  SVG files - Total: $total_svg_files, Missing in CSV: $svg_missing_in_csv"
    
    if [ $has_errors -eq 0 ] && [ $svg_missing_in_csv -eq 0 ]; then
        echo ""
        echo -e "${GREEN}SVG source files validation passed: All files match${NC}"
        return 0
    else
        echo ""
        echo -e "${RED}SVG source files validation failed${NC}"
        return 1
    fi
}

# Validate that all PNGs exist for all resolutions
validate_png() {
    echo "Validating PNG files"
    echo "--------------------"
    
    local has_errors=0
    
    # Check each resolution
    for resolution in "${RESOLUTIONS_LIST[@]}"; do
        echo ""
        echo "Checking resolution: $resolution"
        OUT_DIR="$SCRIPT_DIR/../radio/src/bitmaps/$resolution"
        
        if [ ! -d "$OUT_DIR" ]; then
            echo "  Warning: Output directory not found: $OUT_DIR"
            has_errors=1
            continue
        fi
        
        # Read CSV file and check each entry
        local line_num=0
        local missing_count=0
        local found_count=0
        
        while IFS=';' read -r file width height || [ -n "$file" ]; do
            line_num=$((line_num + 1))
            
            # Skip header line
            if [ $line_num -eq 1 ]; then
                continue
            fi
            
            # Skip empty lines
            if [ -z "$file" ]; then
                continue
            fi
            
            # Skip entries with missing dimensions (they're validated in SVG validation)
            if [ -z "$width" ] || [ -z "$height" ]; then
                continue
            fi
            
            # Check if PNG exists
            if [ ! -f "$OUT_DIR/$file.png" ]; then
                echo -e "  ${RED}Missing PNG: $file.png${NC}"
                has_errors=1
                missing_count=$((missing_count + 1))
            else
                found_count=$((found_count + 1))
            fi
        done < "$SRC_LIST"
        
        echo "  Found: $found_count, Missing: $missing_count"
    done
    
    echo ""
    if [ $has_errors -eq 0 ]; then
        echo -e "${GREEN}PNG validation passed: All PNG files exist${NC}"
        return 0
    else
        echo -e "${RED}PNG validation failed: Some PNG files are missing${NC}"
        return 1
    fi
}

# Validate both SVG sources and PNG files
validate_all() {
    
    local svg_result=0
    local png_result=0
    
    # Temporarily disable exit on error to run both validations
    set +e
    
    echo ""

    validate_svg
    svg_result=$?
    
    echo ""
    
    validate_png
    png_result=$?
    
    # Re-enable exit on error
    set -e
    echo ""

}

# Parse command line arguments
REQUESTED_RESOLUTIONS=()
VALIDATE_MODE=""

if [ $# -eq 0 ]; then
    # No arguments, display usage
    echo "Converts source SVG files to PNG icons for different screen resolutions"
    echo ""
    echo "Usage: ./convert_gfx.sh --validate [svg|png|all]"
    echo "       ./convert_gfx.sh --make [320x240|480x272|800x480|all] [additional resolutions...] [--update] [--resvg]"
    echo "       ./convert_gfx.sh --help"
    echo ""
    echo "Options:"
    echo "  --update          Only regenerate PNGs for SVGs newer than CSV date (only with --make)"
    echo "  --resvg           Use resvg engine instead of rsvg-convert (default, only with --make)"
    echo ""
    echo "Examples:"
    echo "  ./convert_gfx.sh                                  Displays this help message"
    echo "  ./convert_gfx.sh --validate svg                   Validates SVG source files"
    echo "  ./convert_gfx.sh --validate png                   Validates PNG files"
    echo "  ./convert_gfx.sh --validate all                   Validates both SVG and PNG files"
    echo "  ./convert_gfx.sh --make 320x240                   Generates 320x240 PNGs"
    echo "  ./convert_gfx.sh --make 320x240 480x272           Generates 320x240 and 480x272 PNGs"
    echo "  ./convert_gfx.sh --make all                       Generates all resolutions PNGs"
    echo "  ./convert_gfx.sh --make 320x240 --update          Updates only changed SVGs for 320x240"
    echo "  ./convert_gfx.sh --make 320x240 480x272 --update  Updates only changed SVGs for 320x240 and 480x272"
    echo "  ./convert_gfx.sh --make all --update              Updates only changed SVGs for all resolutions"
    echo "  ./convert_gfx.sh --make 320x240 --resvg           Generates 320x240 PNGs (using resvg)"
    echo "  ./convert_gfx.sh --make 320x240 --update --resvg  Updates only changed SVGs for 320x240 (using resvg)"
    echo ""
    echo "Requires: rsvg-convert (default) or resvg command line tool"
    echo ""
    exit 0
fi

# Parse options
while [ $# -gt 0 ]; do
    case "$1" in
        --validate)
            shift
            if [ $# -eq 0 ]; then
                echo "Error: --validate requires an argument (svg, png, or all)"
                exit 1
            fi
            if [ "$1" != "svg" ] && [ "$1" != "png" ] && [ "$1" != "all" ]; then
                echo "Error: --validate argument must be 'svg', 'png', or 'all'"
                exit 1
            fi
            VALIDATE_MODE="$1"
            shift
            # Check for invalid --resvg with --validate
            if [ $# -gt 0 ]; then
                echo "Error: --resvg can only be used with --make"
                exit 1
            fi
            ;;
        --make)
            shift
            if [ $# -eq 0 ]; then
                echo "Error: --make requires at least one resolution argument"
                exit 1
            elif [ "$1" = "all" ]; then
                # Process all supported resolutions
                REQUESTED_RESOLUTIONS=("${RESOLUTIONS_LIST[@]}")
                shift
            else
                # Use provided resolutions
                while [ $# -gt 0 ] && [ "$1" != "--update" ] && [ "$1" != "--resvg" ]; do
                    REQUESTED_RESOLUTIONS+=("$1")
                    shift
                done
            fi
            # Check for --update parameter
            if [ $# -gt 0 ] && [ "$1" = "--update" ]; then
                UPDATE_MODE=true
                shift
            fi
            # Check if --resvg is the last parameter
            if [ $# -gt 0 ] && [ "$1" = "--resvg" ]; then
                CONVERSION_ENGINE="resvg"
                shift
            fi
            # No more parameters should remain
            if [ $# -gt 0 ]; then
                echo "Error: Invalid parameters. Use --update before --resvg"
                exit 1
            fi
            ;;
        --update)
            echo "Error: --update can only be used with --make"
            exit 1
            ;;
        --resvg)
            echo "Error: --resvg can only be used as the last parameter with --make"
            exit 1
            ;;
        *)
            echo "Error: Unknown parameter '$1'"
            echo "Use ./convert_gfx.sh without parameters to see usage information"
            exit 1
            ;;
    esac
done

# Check if the selected conversion engine is available
check_conversion_engine

# Execute based on mode
if [ -n "$VALIDATE_MODE" ]; then
    case "$VALIDATE_MODE" in
        svg)
            validate_svg
            exit $?
            ;;
        png)
            validate_png
            exit $?
            ;;
        all)
            validate_all
            exit $?
            ;;
    esac
fi

# Process each requested resolution
for resolution in "${REQUESTED_RESOLUTIONS[@]}"; do
    process_resolution "$resolution" || exit 1
done

# Update CSV modification dates if in update mode
if [ "$UPDATE_MODE" = true ]; then
    echo ""
    echo "Updating CSV modification dates..."
    
    TEMP_CSV="$SRC_LIST.tmp"
    line_num=0
    
    while IFS=';' read -r file width height csv_date || [ -n "$file" ]; do
        line_num=$((line_num + 1))
        
        # Keep header line as-is
        if [ $line_num -eq 1 ]; then
            echo "$file;$width;$height;${csv_date:-modified}" >> "$TEMP_CSV"
            continue
        fi
        
        # Keep empty lines
        if [ -z "$file" ]; then
            echo "" >> "$TEMP_CSV"
            continue
        fi
        
        # Get current SVG modification date
        svg_path="$SRC_DIR/$file.svg"
        if [ -f "$svg_path" ]; then
            mod_date=$(stat -f "%Sm" -t "%Y-%m-%d %H:%M:%S" "$svg_path" 2>/dev/null)
            if [ -z "$mod_date" ]; then
                # Fallback for Linux
                mod_date=$(stat -c "%y" "$svg_path" 2>/dev/null | cut -d'.' -f1)
            fi
            echo "$file;$width;$height;$mod_date" >> "$TEMP_CSV"
        else
            # Keep existing line if SVG doesn't exist
            echo "$file;$width;$height;$csv_date" >> "$TEMP_CSV"
        fi
    done < "$SRC_LIST"
    
    # Replace original CSV with updated one
    mv "$TEMP_CSV" "$SRC_LIST"
    echo "CSV modification dates updated."
fi

echo "All done!"
