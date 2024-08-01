#!/bin/bash

LVGLDIR="../../thirdparty/lvgl"
TTF_DIR="../"

SYMBOLS_FONT="${LVGLDIR}/scripts/built_in_font/FontAwesome5-Solid+Brands+Regular.woff"
SYMBOLS="61441,61448,61451,61452,61453,61457,61459,61461,61465,61468,61473,61478,61479,61480,61502,61507,61512,61515,61516,61517,61521,61522,61523,61524,61543,61544,61550,61552,61553,61556,61559,61560,61561,61563,61587,61589,61636,61637,61639,61641,61664,61671,61674,61683,61724,61732,61787,61931,62016,62017,62018,62019,62020,62087,62099,62212,62189,62810,63426,63650"

EXTRA_FONT="EdgeTX/extra.ttf"
EXTRA_SYM="0x88-0x94"

ASCII="0x20-0x7F"
DEGREE="0xB0"
BULLET="0x2022"
LATIN1_SUPPLEMENT="0xC0-0xFF"
LATIN1_EXT_A="0x100-0x17F"
LATIN1="${LATIN1_SUPPLEMENT},${LATIN1_EXT_A}"
COMPARE="0x2265"

TW_SYMBOLS=$(python3 get_char_ck.py ../../translations/tw.h)
CN_SYMBOLS=$(python3 get_char_ck.py ../../translations/cn.h)
JP_SYMBOLS=$(python3 get_char_jp.py ../../translations/jp.h)
HE_SYMBOLS=$(python3 get_char_he.py ../../translations/he.h)
RU_SYMBOLS=$(python3 get_char_cyrillic.py ../../translations/ru.h)
UA_SYMBOLS=$(python3 get_char_cyrillic.py ../../translations/ua.h)

# https://yeun.github.io/open-arrow/
ARROWS_FONT="EdgeTX/OpenArrow-Regular.woff"
# 0x80: right, 0x81: left, 0x82: up, 0x83: down
ARROWS="0x21E8=>0x80,0x21E6=>0x81,0x21E7=>0x82,0x21E9=>0x83"

function make_font() {
  local name=$1
  local ttf=$2
  local size=$3
  local chars=$4
  local arg=$5

  lv_font_conv --no-prefilter --bpp 4 --size ${size} \
               --font ${TTF_DIR}${ttf} -r ${ASCII},${DEGREE},${BULLET},${COMPARE}${chars} \
               --font EdgeTX/extra.ttf -r ${EXTRA_SYM} \
               --font ${ARROWS_FONT} -r ${ARROWS} \
               --font ${SYMBOLS_FONT} -r ${SYMBOLS} \
               --format lvgl -o lv_font_${name}_${size}.c --force-fast-kern-format ${arg}
}

function compress_font() {
  local name=$1

  gcc -I ../../thirdparty lz4_font.cpp ../../thirdparty/lz4/lz4hc.c ../../thirdparty/lz4/lz4.c -o lz4_font
  ./lz4_font ${name}
}

function make_font_lz4() {
  local name=$1
  local ttf=$2
  local size=$3
  local chars=$4
  local arg=$5

  lv_font_conv --no-prefilter --bpp 4 --size ${size} \
               --font ${TTF_DIR}${ttf} -r ${ASCII},${DEGREE},${BULLET},${COMPARE}${chars} \
               --font EdgeTX/extra.ttf -r ${EXTRA_SYM} \
               --font ${ARROWS_FONT} -r ${ARROWS} \
               --font ${SYMBOLS_FONT} -r ${SYMBOLS} \
               --format lvgl -o lv_font.inc --force-fast-kern-format ${arg}
  compress_font lv_font_${name}_${size}
}

function make_font_w_extra_sym() {
  local name=$1
  local ttf=$2
  local size=$3
  local chars=$4
  local arg=$5

  lv_font_conv --no-prefilter --bpp 4 --size ${size} \
               --font ${TTF_DIR}${ttf} -r ${ASCII},${DEGREE}${chars} \
               --font EdgeTX/extra.ttf -r ${EXTRA_SYM} \
               --format lvgl -o lv_font.inc --force-fast-kern-format ${arg}
  compress_font lv_font_${name}_${size}
}

function make_font_no_sym() {
  local name=$1
  local ttf=$2
  local size=$3
  local chars=$4
  local arg=$5

  lv_font_conv --no-prefilter --bpp 4 --size ${size} \
               --font ${TTF_DIR}${ttf} -r ${ASCII},${DEGREE}${chars} \
               --format lvgl -o lv_font.inc --force-fast-kern-format ${arg}
  compress_font lv_font_${name}_${size}
}

# LV_SYMBOL_CHARGE, LV_SYMBOL_NEW_LINE, LV_SYMBOL_SD_CARD, LV_SYMBOL_CLOSE
# LV_SYMBOL_FILE, LV_SYMBOL_OK, LV_SYMBOL_WIFI, LV_SYMBOL_USB
BL_SYMBOLS="61671,63650,63426,61453,61787,61452,61931,62087"

function make_bootloader_font() {
  local name=$1
  local ttf=$2
  local size=$3
  lv_font_conv --no-prefilter --bpp 2 --size ${size} \
               --font ${TTF_DIR}${ttf} -r ${ASCII} \
               --font ${SYMBOLS_FONT} -r ${BL_SYMBOLS} \
               --format lvgl -o lv_font_${name}_${size}.c --force-fast-kern-format
}

function make_font_set() {
  local name=$1
  local ttf_normal=$2
  local ttf_bold=$3
  local chars=$4

  make_font_lz4 "${name}" "${ttf_normal}" 9 ${chars} --no-compress
  make_font_lz4 "${name}" "${ttf_normal}" 13 ${chars} --no-compress
  make_font "${name}" "${ttf_normal}" 16 ${chars} --no-compress
  make_font_lz4 "${name}_bold" "${ttf_bold}" 16 ${chars} --no-compress
  make_font_w_extra_sym "${name}" "${ttf_normal}" 24 ${chars} --no-compress
  make_font_no_sym "${name}_bold" "${ttf_bold}" 32 ${chars} --no-compress
  make_font_no_sym "${name}_bold" "${ttf_bold}" 64 "" --no-compress
}

# Regular fonts
make_font_set "roboto" "Roboto/Roboto-Regular.ttf" "Roboto/Roboto-Bold.ttf" ",${LATIN1}"

# Bootloader font
make_bootloader_font "roboto_bl" "Roboto/Roboto-Regular.ttf" 16

# CJK fonts
make_font_set "noto_tw" "Noto/NotoSansCJKsc-Regular.otf" "Noto/NotoSansCJKsc-Bold.otf" ",${TW_SYMBOLS}"
make_font_set "noto_cn" "Noto/NotoSansCJKsc-Regular.otf" "Noto/NotoSansCJKsc-Bold.otf" ",${CN_SYMBOLS}"
make_font_set "noto_jp" "Noto/NotoSansCJKsc-Regular.otf" "Noto/NotoSansCJKsc-Bold.otf" ",${JP_SYMBOLS}"
make_font_set "arimo_he" "Arimo/Arimo-Regular.ttf" "Arimo/Arimo-Bold.ttf" ",${HE_SYMBOLS}"
make_font_set "arimo_ru" "Arimo/Arimo-Regular.ttf" "Arimo/Arimo-Bold.ttf" ",${RU_SYMBOLS}"
make_font_set "arimo_ua" "Arimo/Arimo-Regular.ttf" "Arimo/Arimo-Bold.ttf" ",${UA_SYMBOLS}"

rm lv_font.inc
rm lz4_font
