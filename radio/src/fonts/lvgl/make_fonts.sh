#!/bin/bash

LVGLDIR="../../thirdparty/libopenui/thirdparty/lvgl"
TTF_DIR="../"

SYMBOLS_FONT="${LVGLDIR}/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff"
SYMBOLS="61441,61448,61451,61452,61452,61453,61457,61459,61461,61465,61468,61473,61478,61479,61480,61502,61507,61512,61515,61516,61517,61521,61522,61523,61524,61543,61544,61550,61552,61553,61556,61559,61560,61561,61563,61587,61589,61636,61637,61639,61641,61664,61671,61674,61683,61724,61732,61787,61931,62016,62017,62018,62019,62020,62087,62099,62212,62189,62810,63426,63650"

EXTRA_FONT="EdgeTX/extra.ttf"
EXTRA_SYM="0x80-0x94"

ASCII="0x20-0x7F"
DEGREE="0xB0"
BULLET="0x2022"
LATIN1="0xC0-0xFF"

function make_font() {
  local name=$1
  local ttf=$2
  local size=$3
  local arg=$4
  lv_font_conv --no-prefilter --bpp 4 --size ${size} \
               --font ${TTF_DIR}${ttf} -r ${ASCII},${DEGREE},${BULLET},${LATIN1} \
               --font EdgeTX/extra.ttf -r ${EXTRA_SYM} \
               --font ${SYMBOLS_FONT} -r ${SYMBOLS} \
               --format lvgl -o lv_font_${name}_${size}.c --force-fast-kern-format ${arg}
}

function make_font_no_sym() {
  local name=$1
  local ttf=$2
  local size=$3
  local arg=$4
  lv_font_conv --no-prefilter --bpp 4 --size ${size} \
               --font ${TTF_DIR}${ttf} -r ${ASCII},${LATIN1} \
               --format lvgl -o lv_font_${name}_${size}.c --force-fast-kern-format ${arg}
}

# LV_SYMBOL_CHARGE, LV_SYMBOL_NEW_LINE, LV_SYMBOL_SD_CARD, LV_SYMBOL_CLOSE
BL_SYMBOLS="61671,63650,63426,61453"

function make_bootloader_font() {
  local name=$1
  local ttf=$2
  local size=$3
  lv_font_conv --no-prefilter --bpp 2 --size ${size} \
               --font ${TTF_DIR}${ttf} -r ${ASCII} \
               --font ${SYMBOLS_FONT} -r ${BL_SYMBOLS} \
               --format lvgl -o lv_font_${name}_${size}.c --force-fast-kern-format
}

make_font "roboto" "Roboto/Roboto-Regular.ttf" 9 --no-compress
make_font "roboto" "Roboto/Roboto-Regular.ttf" 13 --no-compress
make_font "roboto" "Roboto/Roboto-Regular.ttf" 14 --no-compress
make_font "roboto" "Roboto/Roboto-Regular.ttf" 15 --no-compress
make_font "roboto" "Roboto/Roboto-Regular.ttf" 16 --no-compress
make_font "roboto" "Roboto/Roboto-Regular.ttf" 17 --no-compress
make_font "roboto_bold" "Roboto/Roboto-Bold.ttf" 16
make_font "roboto_bold" "Roboto/Roboto-Bold.ttf" 17
make_font_no_sym "roboto" "Roboto/Roboto-Regular.ttf" 24
make_font_no_sym "roboto_bold" "Roboto/Roboto-Bold.ttf" 32
make_font_no_sym "roboto_bold" "Roboto/Roboto-Bold.ttf" 64
make_bootloader_font "roboto_bl" "Roboto/Roboto-Regular.ttf" 16

