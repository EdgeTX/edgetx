set(FONT "STD" CACHE STRING "Choose font : STD or SQT5")
set_property(CACHE FONT PROPERTY STRINGS "STD" "SQT5")
string(TOLOWER "${FONT}" FONT_DIR)

set(STICKS_ARGS --rows 4)

add_bitmaps_target(taranis_xbm ${RADIO_SRC_DIR}/bitmaps/sticks.xbm 1bit "${STICKS_ARGS}")  # 1bit format, 1byte size, split into 4 rows
add_fonts_target(taranis_fonts ${RADIO_SRC_DIR}/fonts/${FONT_DIR}/*.png)
add_bitmaps_target(taranis_bitmaps ${RADIO_SRC_DIR}/bitmaps/212x64/*.png 4bits "--rle")
add_dependencies(taranis_bitmaps taranis_fonts taranis_xbm)
