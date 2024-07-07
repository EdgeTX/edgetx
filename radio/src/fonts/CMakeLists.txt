#
# LVGL fonts
#

file(GLOB LVGL_FONTS
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/..
  lvgl/lv_font_*.c)

set(LVGL_FONT_SOURCES ${LVGL_FONTS} PARENT_SCOPE)

set(LVGL_FONT_SOURCES_MINIMAL
  fonts/lvgl/lv_font_roboto_bl_16.c
  PARENT_SCOPE
)
