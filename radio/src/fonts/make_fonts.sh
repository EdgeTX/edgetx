#!/bin/bash

function make_font() {
  local name=$1
  local ttf=$2
  local size=$3
  lv_font_conv --no-prefilter --bpp 4 --size ${size} --font ${ttf} -r 0x20-0x7F,0xB0,0x2022,0xDF,0xE4,0xF6,0xFC,0xC4,0xD6,0xDC --font EdgeTX_extra.ttf -r 0x80-0x94 --format lvgl -o lv_font_${name}_${size}.c --force-fast-kern-format --lv-include ../thirdparty/lvgl/src/lvgl.h
}

make_font "roboto" "Roboto/Roboto-Regular.ttf" 9 
make_font "roboto" "Roboto/Roboto-Regular.ttf" 13 
make_font "roboto" "Roboto/Roboto-Regular.ttf" 15  
make_font "roboto" "Roboto/Roboto-Regular.ttf" 16 
make_font "roboto" "Roboto/Roboto-Regular.ttf" 17 
make_font "roboto" "Roboto/Roboto-Regular.ttf" 24 
make_font "roboto_bold" "Roboto/Roboto-Bold.ttf" 16 
make_font "roboto_bold" "Roboto/Roboto-Bold.ttf" 17 
make_font "roboto_bold" "Roboto/Roboto-Bold.ttf" 16 
make_font "roboto_bold" "Roboto/Roboto-Bold.ttf" 32 
make_font "roboto_bold" "Roboto/Roboto-Bold.ttf" 64 

