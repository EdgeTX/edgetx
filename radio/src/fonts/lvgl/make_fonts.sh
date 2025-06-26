#!/bin/bash

# Exit on any error, undefined variables, or pipe failures
set -euo pipefail

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RADIO_SRC_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"
TRANSLATIONS_DIR="${RADIO_SRC_DIR}/translations"

SYMBOLS_FONT="${RADIO_SRC_DIR}/thirdparty/lvgl/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff"
# test if realpath supports --relative-to as macOS BSD version doesn't
if realpath --relative-to="." "." >/dev/null 2>&1; then
    # GNU realpath with --relative-to support
    SYMBOLS_FONT_REL="$(realpath --relative-to="${SCRIPT_DIR}" "${SYMBOLS_FONT}")"
else
    # BSD realpath or no realpath - use hardcoded relative path
    SYMBOLS_FONT_REL="../../thirdparty/lvgl/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff"
fi
SYMBOLS="61441,61448,61451,61452,61453,61457,61459,61461,61465,61468,61473,61478,61479,61480,61502,61507,61512,61515,61516,61517,61521,61522,61523,61524,61543,61544,61550,61552,61553,61556,61559,61560,61561,61563,61587,61589,61636,61637,61639,61641,61664,61671,61674,61683,61724,61732,61787,61931,62016,62017,62018,62019,62020,62087,62099,62212,62189,62810,63426,63650"

EXTRA_FONT="EdgeTX/extra.ttf"
EXTRA_SYM="0x88-0x96"

# https://yeun.github.io/open-arrow/
ARROWS_FONT="EdgeTX/OpenArrow-Regular.woff"
# 0x80: right, 0x81: left, 0x82: up, 0x83: down
ARROWS="0x21E8=>0x80,0x21E6=>0x81,0x21E7=>0x82,0x21E9=>0x83"

LATIN_FONT="Roboto/Roboto-Regular.ttf"
LATIN_FONT_BOLD="Roboto/Roboto-Bold.ttf"

ASCII="0x20-0x7F"
DEGREE="0xB0"
BULLET="0x2022"
LATIN1_SUPPLEMENT="0xC0-0xFF"
LATIN1_EXT_A="0x100-0x17F"
LATIN1="${LATIN1_SUPPLEMENT},${LATIN1_EXT_A}"
COMPARE="0x2265"

# LV_SYMBOL_CHARGE, LV_SYMBOL_NEW_LINE, LV_SYMBOL_SD_CARD, LV_SYMBOL_CLOSE
# LV_SYMBOL_FILE, LV_SYMBOL_OK, LV_SYMBOL_WIFI, LV_SYMBOL_USB
BL_SYMBOLS="61671,63650,63426,61453,61787,61452,61931,62087"

check_dependencies() {
    # Check if lv_font_conv is available
    if ! command -v lv_font_conv >/dev/null 2>&1; then
        echo "ERROR: lv_font_conv not found. Please install it from https://github.com/lvgl/lv_font_conv or npm registry" >&2
        exit 1
    fi
    
    # Check if we're on macOS and provide helpful info about realpath
    if [[ "$(uname)" == "Darwin" ]]; then
        if ! realpath --relative-to="." "." >/dev/null 2>&1; then
            echo "INFO: Using BSD realpath (no --relative-to support). Using hardcoded relative paths." >&2
            echo "INFO: To install GNU coreutils (optional): brew install coreutils" >&2
        fi
    fi
}

get_translation_symbols() {
    TW_SYMBOLS=$(python3 get_char_ck.py "${TRANSLATIONS_DIR}/tw.h" 2>/dev/null || echo "")
    CN_SYMBOLS=$(python3 get_char_ck.py "${TRANSLATIONS_DIR}/cn.h" 2>/dev/null || echo "")
    JP_SYMBOLS=$(python3 get_char_jp.py "${TRANSLATIONS_DIR}/jp.h" 2>/dev/null || echo "")
    HE_SYMBOLS=$(python3 get_char_he.py "${TRANSLATIONS_DIR}/he.h" 2>/dev/null || echo "")
    KO_SYMBOLS=$(python3 get_char_ko.py "${TRANSLATIONS_DIR}/ko.h" 2>/dev/null || echo "")
    RU_SYMBOLS=$(python3 get_char_cyrillic.py "${TRANSLATIONS_DIR}/ru.h" 2>/dev/null || echo "")
    UA_SYMBOLS=$(python3 get_char_cyrillic.py "${TRANSLATIONS_DIR}/ua.h" 2>/dev/null || echo "")

    # Export variables for later use
    export TW_SYMBOLS CN_SYMBOLS JP_SYMBOLS HE_SYMBOLS KO_SYMBOLS RU_SYMBOLS UA_SYMBOLS
}

function make_font() {
  local name=$1
  local latin_ttf=$2
  local ttf=$3
  local size=$4
  local sfx=$5
  local dir=$6
  local chars=$7

  echo "Creating font: ${name}_${sfx} (size: ${size})"
  
  # Use relative paths for lv_font_conv to avoid absolute paths in generated files
  lv_font_conv --no-prefilter --bpp 4 --size "${size}" \
               --font "../${latin_ttf}" -r "${ASCII},${DEGREE},${BULLET},${COMPARE}" \
               --font "../${ttf}" -r "${chars}" \
               --font "${EXTRA_FONT}" -r "${EXTRA_SYM}" \
               --font "${ARROWS_FONT}" -r "${ARROWS}" \
               --font "${SYMBOLS_FONT_REL}" -r "${SYMBOLS}" \
               --format lvgl -o "${dir}/lv_font_${name}_${sfx}.c" --force-fast-kern-format --no-compress
}

function compress_font() {
  local name=$1

  # Compile the compression tool
  gcc -I "${RADIO_SRC_DIR}/thirdparty" \
      "${SCRIPT_DIR}/lz4_font.cpp" \
      "${RADIO_SRC_DIR}/thirdparty/lz4/lz4hc.c" \
      "${RADIO_SRC_DIR}/thirdparty/lz4/lz4.c" \
      -o "${SCRIPT_DIR}/lz4_font"
  
  "${SCRIPT_DIR}/lz4_font" "${name}"
}

function make_font_lz4() {
  local name=$1
  local latin_ttf=$2
  local ttf=$3
  local size=$4
  local sfx=$5
  local dir=$6
  local chars=$7

  echo "Creating compressed font: ${name}_${sfx} (size: ${size})"
  
  # Use relative paths for lv_font_conv
  lv_font_conv --no-prefilter --bpp 4 --size "${size}" \
               --font "../${latin_ttf}" -r "${ASCII},${DEGREE},${BULLET},${COMPARE}" \
               --font "../${ttf}" -r "${chars}" \
               --font "${EXTRA_FONT}" -r "${EXTRA_SYM}" \
               --font "${ARROWS_FONT}" -r "${ARROWS}" \
               --font "${SYMBOLS_FONT_REL}" -r "${SYMBOLS}" \
               --format lvgl -o "lv_font.inc" --force-fast-kern-format --no-compress
  compress_font "${dir}/lv_font_${name}_${sfx}"
}

function make_font_w_extra_sym() {
  local name=$1
  local latin_ttf=$2
  local ttf=$3
  local size=$4
  local sfx=$5
  local dir=$6
  local chars=$7

  echo "Creating font with extra symbols: ${name}_${sfx} (size: ${size})"
  
  lv_font_conv --no-prefilter --bpp 4 --size "${size}" \
               --font "../${latin_ttf}" -r "${ASCII},${DEGREE}" \
               --font "../${ttf}" -r "${chars}" \
               --font "${EXTRA_FONT}" -r "${EXTRA_SYM}" \
               --format lvgl -o "lv_font.inc" --force-fast-kern-format --no-compress
  compress_font "${dir}/lv_font_${name}_${sfx}"
}

function make_font_no_sym() {
  local name=$1
  local latin_ttf=$2
  local ttf=$3
  local size=$4
  local sfx=$5
  local dir=$6
  local chars=$7

  echo "Creating font without symbols: ${name}_${sfx} (size: ${size})"
  
  lv_font_conv --no-prefilter --bpp 4 --size "${size}" \
               --font "../${latin_ttf}" -r "${ASCII},${DEGREE}" \
               --font "../${ttf}" -r "${chars}" \
               --format lvgl -o "lv_font.inc" --force-fast-kern-format --no-compress
  compress_font "${dir}/lv_font_${name}_${sfx}"
}

function make_font_no_sym_no_trans() {
  local name=$1
  local latin_ttf=$2
  local size=$3
  local sfx=$4
  local dir=$5

  echo "Creating basic font: ${name}_${sfx} (size: ${size})"
  
  lv_font_conv --no-prefilter --bpp 4 --size "${size}" \
               --font "../${latin_ttf}" -r "${ASCII},${DEGREE}" \
               --format lvgl -o "lv_font.inc" --force-fast-kern-format --no-compress
  compress_font "${dir}/lv_font_${name}_${sfx}"
}

function make_bootloader_font() {
  local name=$1
  local ttf=$2
  local size=$3
  local dir=$4

  echo "Creating bootloader font: ${name} (size: ${size})"
  
  lv_font_conv --no-prefilter --bpp 1 --size "${size}" --no-compress \
               --font "../${ttf}" -r "${ASCII}" \
               --font "${SYMBOLS_FONT_REL}" -r "${BL_SYMBOLS}" \
               --format lvgl -o "${dir}/lv_font_${name}.c" --force-fast-kern-format --no-compress
}

function make_font_set() {
  local name=$1
  local ttf_normal=$2
  local ttf_bold=$3
  local chars=$4

  if [[ -z "$chars" ]]; then
    echo "WARNING: No characters found for ${name} font set. Skipping." >&2
    return 0
  fi

  echo "Creating font set for: ${name}"

  # Standard LCD fonts (480x272, 480x320, 320x480)
  make_font_lz4 "${name}" "${LATIN_FONT}" "${ttf_normal}" 9 "XXS" "std" "${chars}"
  make_font_lz4 "${name}" "${LATIN_FONT}" "${ttf_normal}" 13 "XS" "std" "${chars}"
  make_font "${name}" "${LATIN_FONT}" "${ttf_normal}" 16 "STD" "std" "${chars}"
  make_font_lz4 "${name}_bold" "${LATIN_FONT_BOLD}" "${ttf_bold}" 16 "STD" "std" "${chars}"
  make_font_w_extra_sym "${name}" "${LATIN_FONT}" "${ttf_normal}" 24 "L" "std" "${chars}"
  make_font_no_sym "${name}_bold" "${LATIN_FONT_BOLD}" "${ttf_bold}" 32 "XL" "std" "${chars}"

  # Small LCD fonts (320x240)
  make_font_lz4 "${name}" "${LATIN_FONT}" "${ttf_normal}" 8 "XXS" "sml" "${chars}"
  make_font_lz4 "${name}" "${LATIN_FONT}" "${ttf_normal}" 10 "XS" "sml" "${chars}"
  make_font "${name}" "${LATIN_FONT}" "${ttf_normal}" 13 "STD" "sml" "${chars}"
  make_font_lz4 "${name}_bold" "${LATIN_FONT_BOLD}" "${ttf_bold}" 13 "STD" "sml" "${chars}"
  make_font_w_extra_sym "${name}" "${LATIN_FONT}" "${ttf_normal}" 19 "L" "sml" "${chars}"
  make_font_no_sym "${name}_bold" "${LATIN_FONT_BOLD}" "${ttf_bold}" 25 "XL" "sml" "${chars}"

  # Large LCD fonts (800x480)
  make_font_lz4 "${name}" "${LATIN_FONT}" "${ttf_normal}" 13 "XXS" "lrg" "${chars}"
  make_font_lz4 "${name}" "${LATIN_FONT}" "${ttf_normal}" 19 "XS" "lrg" "${chars}"
  make_font "${name}" "${LATIN_FONT}" "${ttf_normal}" 24 "STD" "lrg" "${chars}"
  make_font_lz4 "${name}_bold" "${LATIN_FONT_BOLD}" "${ttf_bold}" 24 "STD" "lrg" "${chars}"
  make_font_w_extra_sym "${name}" "${LATIN_FONT}" "${ttf_normal}" 36 "L" "lrg" "${chars}"
  make_font_no_sym "${name}_bold" "${LATIN_FONT_BOLD}" "${ttf_bold}" 48 "XL" "lrg" "${chars}"
}

# Main execution starts here
main() {
    echo "Starting font generation..."

    # Change to script directory for python and other scripts to work correctly
    cd "${SCRIPT_DIR}"

    # Check dependencies and setup
    check_dependencies
    get_translation_symbols
    
    # Bootloader fonts
    echo "Generating bootloader fonts..."
    make_bootloader_font "bl" "Roboto/Roboto-Regular-BL.ttf" 16 "std" # 480x272, 480x320, 320x480
    make_bootloader_font "bl" "Roboto/Roboto-Regular-BL.ttf" 14 "sml" # 320x240
    make_bootloader_font "bl" "Roboto/Roboto-Regular-BL.ttf" 24 "lrg" # 800x480

    # XXL fonts (no translation chars)
    echo "Generating XXL fonts..."
    make_font_no_sym_no_trans "en_bold" "${LATIN_FONT_BOLD}" 64 "XXL" "std"
    make_font_no_sym_no_trans "en_bold" "${LATIN_FONT_BOLD}" 50 "XXL" "sml"
    make_font_no_sym_no_trans "en_bold" "${LATIN_FONT_BOLD}" 96 "XXL" "lrg"

    # Language fonts
    echo "Generating language font sets..."
    make_font_set "en" "${LATIN_FONT}" "${LATIN_FONT_BOLD}" "${LATIN1}"
    make_font_set "tw" "Noto/NotoSansCJKsc-Regular.otf" "Noto/NotoSansCJKsc-Bold.otf" "${TW_SYMBOLS}"
    make_font_set "cn" "Noto/NotoSansCJKsc-Regular.otf" "Noto/NotoSansCJKsc-Bold.otf" "${CN_SYMBOLS}"
    make_font_set "jp" "Noto/NotoSansCJKsc-Regular.otf" "Noto/NotoSansCJKsc-Bold.otf" "${JP_SYMBOLS}"
    make_font_set "he" "Arimo/Arimo-Regular.ttf" "Arimo/Arimo-Bold.ttf" "${HE_SYMBOLS}"
    make_font_set "ru" "Arimo/Arimo-Regular.ttf" "Arimo/Arimo-Bold.ttf" "${RU_SYMBOLS}"
    make_font_set "ua" "Arimo/Arimo-Regular.ttf" "Arimo/Arimo-Bold.ttf" "${UA_SYMBOLS}"
    make_font_set "ko" "Nanum/NanumBarunpenR.ttf" "Nanum/NanumBarunpenB.ttf" "${KO_SYMBOLS}"

    # Clean up temporary files
    echo "Cleaning up temporary files..."
    rm -f "${SCRIPT_DIR}/lv_font.inc"
    rm -f "${SCRIPT_DIR}/lz4_font"
    
    echo "Font generation completed successfully!"
}

# Run main function
main "$@"
