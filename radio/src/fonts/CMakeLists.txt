string(TOLOWER ${TRANSLATIONS} subset)

#
# libopenui fonts
#

if( (${subset} STREQUAL "cn") OR (${subset} STREQUAL "tw"))
  # Use this font for CJK languages
  set(FONT "Noto/NotoSansCJKsc-Regular")
  set(FONT_BOLD "Noto/NotoSansCJKsc-Bold")
  set(FONT_OFFSET -3)
else()
  # Use this font for non-CJK languages
  set(FONT "Roboto/Roboto-Regular")
  set(FONT_BOLD "Roboto/Roboto-Bold")
  set(FONT_OFFSET 0)

  #set(FONT "Kanit/Kanit-Regular")
  #set(FONT_BOLD "Kanit/Kanit-Bold")

  #set(FONT "Ubuntu/Ubuntu-Regular")
  #set(FONT_BOLD "Ubuntu/Ubuntu-Bold")
endif()

add_truetype_font_target(9      9  ${subset} none "${FONT_ARGS}")
add_truetype_font_target(13     13 ${subset} none "${FONT_ARGS}")
add_truetype_font_target(15     15 ${subset} none "${FONT_ARGS}")
add_truetype_font_target(16     16 ${subset} none "${FONT_ARGS}")
add_truetype_font_target(16en   16 en        none "${FONT_ARGS}")
add_truetype_font_target(bold16 16 ${subset} bold "${FONT_ARGS}")
add_truetype_font_target(17     17 ${subset} none "${FONT_ARGS}")
add_truetype_font_target(17en   17 en        none "${FONT_ARGS}")
add_truetype_font_target(bold17 17 ${subset} bold "${FONT_ARGS}")
add_truetype_font_target(24     24 ${subset} none "${FONT_ARGS}")
if( (${subset} STREQUAL "cn") OR (${subset} STREQUAL "tw"))
  add_truetype_font_target(32     30 ${subset} bold "${FONT_ARGS}")
else()
  add_truetype_font_target(32     32 ${subset} bold "${FONT_ARGS}")
endif()
add_truetype_font_target(64     64 en        bold "${FONT_ARGS}")

add_custom_target(truetype_fonts DEPENDS ${truetype_fonts_targets})

#
# LVGL fonts
#

set(LVGL_FONT_DIR fonts/lvgl)
set(LVGL_FONT_SOURCES
  ${LVGL_FONT_DIR}/lv_font_roboto_9.c
  ${LVGL_FONT_DIR}/lv_font_roboto_13.c
  ${LVGL_FONT_DIR}/lv_font_roboto_14.c
  ${LVGL_FONT_DIR}/lv_font_roboto_15.c
  ${LVGL_FONT_DIR}/lv_font_roboto_16.c
  ${LVGL_FONT_DIR}/lv_font_roboto_17.c
  ${LVGL_FONT_DIR}/lv_font_roboto_24.c
  ${LVGL_FONT_DIR}/lv_font_roboto_bold_16.c
  ${LVGL_FONT_DIR}/lv_font_roboto_bold_17.c
  ${LVGL_FONT_DIR}/lv_font_roboto_bold_32.c
  ${LVGL_FONT_DIR}/lv_font_roboto_bold_64.c
  PARENT_SCOPE)
