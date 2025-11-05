#!/bin/bash

# Combined tool for EdgeTX radio PNG workflow
# - --list  : scan SVGs and produce csv with dimensions (like png-radio.sh)
# - --build : generate PNGs from csv (like png-generate.sh)
# - --help  : show usage

set -euo pipefail

# Resolve script directory to make relative paths robust
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Base and source directories
BASE_DIR="$SCRIPT_DIR/../radio/src/bitmaps"
SRC_DIR="$BASE_DIR/img-src"

# Resolution folders (note: repository uses 800x480)
RES1="320x240"
RES2="480x272"
RES3="800x480"

# CSV and error file paths   
GFX_LIST_FILE="$SCRIPT_DIR/png_radio_fw_list.csv"

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Background color for mask_* PNGs when building (default white; override via BG_COLOR)
BG_COLOR=${BG_COLOR:-white}

usage() {
  cat <<EOF
This script manages PNG generation for EdgeTX Color LCD radio firmware from SVG sources.

Usage: $(basename "$0") [ --check | --update | --build | --help]

Modes:
  --check  Verify that PNGs exist for all SVGs in ${RES1}, ${RES2}, ${RES3}.
           Prints to console missing files.

  --update Scan SVGs under img-src and produce CSV of PNG dimensions for ${RES1}, ${RES2}, ${RES3}
           Output: ${GFX_LIST_FILE}
           Missing PNGs are printed to console.

  --build  Read ${GFX_LIST_FILE} and generate PNGs in ${BASE_DIR}
           - Target directories <DIR> are taken from CSV header (cols 2-4)
           - Output PNGs placed in <DIR> folders (e.g., 320x240, 480x272, 800x480)
           - Files starting with 'mask_' rendered with background color (${BG_COLOR})
             Others remain transparent.

  --help   Show this help.
EOF
}

require_file_cmd() {
  if ! command -v file &> /dev/null; then
    echo -e "${RED}Error: 'file' command not found. Please install it first.${NC}"
    exit 1
  fi
}

require_src_dir() {
  if [ ! -d "$SRC_DIR" ]; then
    echo -e "${RED}Error: Source SVG directory '$SRC_DIR' does not exist${NC}"
    exit 1
  fi
}

get_dimensions() {
  # Get PNG dimensions WxH using 'file', fallback to sips on macOS
  local png_file="$1"
  if [ ! -f "$png_file" ]; then
    echo ""
    return
  fi
  local dimensions
  dimensions=$(file "$png_file" | grep -oE '[0-9]+ x [0-9]+' | head -1 | sed 's/ x /x/g')
  if [ -z "$dimensions" ] && command -v sips &> /dev/null; then
    local width height
    width=$(sips -g pixelWidth "$png_file" 2>/dev/null | awk '/pixelWidth:/ {print $2}')
    height=$(sips -g pixelHeight "$png_file" 2>/dev/null | awk '/pixelHeight:/ {print $2}')
    if [ -n "$width" ] && [ -n "$height" ]; then
      dimensions="${width}x${height}"
    fi
  fi
  echo "$dimensions"
}

# Utilities used in build mode
trim() { echo "$1" | sed 's/^[[:space:]]*//;s/[[:space:]]*$//'; }
strip_quotes() { echo "$1" | sed 's/^"//;s/"$//'; }
parse_dim() {
  local d; d=$(echo "$1" | sed 's/"//g' | sed 's/ //g')
  if [[ "$d" =~ ^([0-9]+)x([0-9]+)$ ]]; then
    echo "${BASH_REMATCH[1]} ${BASH_REMATCH[2]}"
  else
    echo " "
  fi
}

mode_list() {
  require_src_dir
  require_file_cmd

  echo "Scanning SVG files in $SRC_DIR..."
  echo "CSV output: $GFX_LIST_FILE"
  echo "---------------------------------------------------"

  # Prepare output; remove any stale error file. Only create error file if issues are found.
  > "$GFX_LIST_FILE"
  echo "\"file\"; $RES1; $RES2; $RES3" >> "$GFX_LIST_FILE"

  # Walk all SVG files under img-src (sorted by relative path) and check PNGs in each resolution
  find "$SRC_DIR" -type f -iname "*.svg" -print \
    | sed "s|^$SRC_DIR/||" \
    | LC_ALL=C sort \
    | while IFS= read -r rel_svg_path; do
      # Replace .svg with .png for output file names and name-only for CSV
      rel_png_path="${rel_svg_path%.*}.png"
      rel_name="${rel_svg_path%.*}"

  # Build PNG full paths for each resolution
  png1="$BASE_DIR/$RES1/$rel_png_path"
  png2="$BASE_DIR/$RES2/$rel_png_path"
  png3="$BASE_DIR/$RES3/$rel_png_path"

      # Compute dimensions and log missing files
      dim1=""; dim2=""; dim3=""
      if [ -f "$png1" ]; then
        dim1=$(get_dimensions "$png1")
      else
        echo -e "${RED}Warning: Missing $RES1: $rel_png_path${NC}"
      fi

      if [ -f "$png2" ]; then
        dim2=$(get_dimensions "$png2")
      else
        echo -e "${RED}Warning: Missing $RES2: $rel_png_path${NC}"
      fi

      if [ -f "$png3" ]; then
        dim3=$(get_dimensions "$png3")
      else
        echo -e "${RED}Warning: Missing $RES3: $rel_png_path${NC}"
      fi

      # Write to CSV
      echo "$rel_name; $dim1; $dim2; $dim3" >> "$GFX_LIST_FILE"
    done

  echo "---------------------------------------------------"
  TOTAL_SVG=$(find "$SRC_DIR" -type f -iname "*.svg" | wc -l | xargs)
  echo "Total SVG files processed: $TOTAL_SVG"
  echo "Results saved to: $GFX_LIST_FILE"
  # Missing PNGs (if any) were printed above.
}

mode_check() {
  # Verify presence of PNGs only; no CSV or error file created
  require_src_dir

  echo "Checking PNG presence for SVGs in $SRC_DIR..."
  echo "Resolutions: $RES1, $RES2, $RES3"
  echo "---------------------------------------------------"

  local any_missing=0

  # Sorted list of relative SVG paths
  find "$SRC_DIR" -type f -iname "*.svg" -print \
    | sed "s|^$SRC_DIR/||" \
    | LC_ALL=C sort \
    | while IFS= read -r rel_svg_path; do
      rel_png_path="${rel_svg_path%.*}.png"

      png1="$BASE_DIR/$RES1/$rel_png_path"
      png2="$BASE_DIR/$RES2/$rel_png_path"
      png3="$BASE_DIR/$RES3/$rel_png_path"

      missing_local=0
      if [ ! -f "$png1" ]; then
        echo -e "${RED}Missing $RES1: $rel_png_path${NC}"
        missing_local=1
      fi
      if [ ! -f "$png2" ]; then
        echo -e "${RED}Missing $RES2: $rel_png_path${NC}"
        missing_local=1
      fi
      if [ ! -f "$png3" ]; then
        echo -e "${RED}Missing $RES3: $rel_png_path${NC}"
        missing_local=1
      fi

      if [ $missing_local -ne 0 ]; then
        any_missing=1
      fi
    done

  echo "---------------------------------------------------"
  if [ ${any_missing} -eq 0 ]; then
    echo -e "${GREEN}All PNGs present for the scanned SVGs.${NC}"
  fi
}

mode_build() {
  # Build PNGs from CSV
  local CSV_FILE="$GFX_LIST_FILE"

  if [ ! -f "$CSV_FILE" ]; then
    echo -e "${RED}Error: CSV file not found: $CSV_FILE${NC}"
    exit 1
  fi
  require_src_dir

  if ! command -v rsvg-convert &> /dev/null; then
    echo -e "${RED}Error: rsvg-convert not found in PATH. Install librsvg (provides rsvg-convert).${NC}"
    exit 1
  fi

  echo "Reading CSV: $CSV_FILE"

  # Read header to extract directories
    IFS=';' read -r h1 h2 h3 h4 < "$CSV_FILE"
  DIR1=$(strip_quotes "$(trim "$h2")")
  DIR2=$(strip_quotes "$(trim "$h3")")
  DIR3=$(strip_quotes "$(trim "$h4")")
  echo "Target dirs from header: $DIR1, $DIR2, $DIR3"

  # Process each CSV data line (skip header)
    tail -n +2 "$CSV_FILE" | while IFS=';' read -r file dim1 dim2 dim3 rest; do
    # Trim and remove quotes
    file=$(strip_quotes "$(trim "$file")")
    dim1=$(strip_quotes "$(trim "$dim1")")
    dim2=$(strip_quotes "$(trim "$dim2")")
    dim3=$(strip_quotes "$(trim "$dim3")")

    # Skip empty file name
    [ -z "$file" ] && continue

    # SVG path (file may include subdirs)
    svg_path="$SRC_DIR/${file}.svg"
    if [ ! -f "$svg_path" ]; then
      echo -e "${RED}Warning: SVG not found: $svg_path${NC}"
      continue
    fi

    # Determine if background should be white (for mask_* files) or transparent
    basename_file=$(basename "$file")
    if [[ "$basename_file" == mask_* ]]; then
      use_bg="$BG_COLOR"
    else
      use_bg=""
    fi

    # For each target dir/dimension, generate PNG
    for idx in 1 2 3; do
      case $idx in
        1) target_dir="$DIR1"; dim="$dim1" ;;
        2) target_dir="$DIR2"; dim="$dim2" ;;
        3) target_dir="$DIR3"; dim="$dim3" ;;
      esac

      # If no dimension, skip
      if [ -z "$dim" ]; then
        echo -e "${RED}Warning: Skipping $file for $target_dir: no dimensions specified${NC}"
        continue
      fi

      # Parse WxH
      read -r w h <<< "$(parse_dim "$dim")"
      if [ -z "$w" ] || [ -z "$h" ]; then
        echo -e "${RED}Warning: Invalid dimension for $file in $target_dir: '$dim'${NC}"
        continue
      fi

      # Write outputs directly into the target directories (e.g. 320x240, 480x272, 800x480)
      out_dir="$BASE_DIR/$target_dir/$(dirname "$file")"
      if [ "$(dirname "$file")" = "." ]; then
        out_dir="$BASE_DIR/$target_dir"
      fi
      mkdir -p "$out_dir"

      out_file="$out_dir/$(basename "$file").png"

      if [ -n "$use_bg" ]; then
        echo "Converting $svg_path -> $out_file (${w}x${h}, bg=$use_bg)"
        rsvg-convert "$svg_path" -w "$w" -h "$h" -b "$use_bg" -o "$out_file"
      else
        echo "Converting $svg_path -> $out_file (${w}x${h}, transparent)"
        rsvg-convert "$svg_path" -w "$w" -h "$h" -o "$out_file"
      fi
    done
  done

  echo "All conversions attempted."
}

# Main
case "${1:-}" in
  --check)
    mode_check
    ;;
  --update)
    mode_list
    ;;
  --build)
    mode_build
    ;;
  --help|*)
    usage
    ;;
esac