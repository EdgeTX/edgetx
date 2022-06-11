if(HARDWARE_KEYS)
  add_definitions(-DHARDWARE_KEYS)
endif()

if(HARWARE_TOUCH)
  add_definitions(-DHARDWARE_TOUCH)
endif()

set(LIBOPENUI_SRC
  libopenui_file.cpp
  bitmapbuffer.cpp
  window.cpp
  layer.cpp
  form.cpp
  button.cpp
  button_matrix.cpp
  static.cpp
  checkbox.cpp
  carousel.cpp
  slider.cpp
  mainwindow.cpp
  menu.cpp
  menutoolbar.cpp
  choice.cpp
  choiceex.cpp
  filechoice.cpp
  numberedit.cpp
  textedit.cpp
  coloredit.cpp
  progress.cpp
  table.cpp
  modal_window.cpp
  dialog.cpp
  keyboard_text.cpp
  keyboard_number.cpp
  keyboard_base.cpp
  # LVGL widgets
  widgets/window_base.c
  widgets/simple_btn.c
  widgets/field_edit.c
  )

foreach(LIBOPENUI_FILE ${LIBOPENUI_SRC})
  set(LIBOPENUI_SRC_FILES ${LIBOPENUI_SRC_FILES} ${LIBOPENUI_SRC_DIR}/src/${LIBOPENUI_FILE})
endforeach()

set(SRC ${SRC} ${LIBOPENUI_SRC_FILES}
  ${LIBOPENUI_SRC_DIR}/thirdparty/lz4/lz4.c
  )

# add_library(libopenui_native OBJECT ${LIBOPENUI_SRC_FILES})
# add_library(libopenui_stm32 OBJECT ${LIBOPENUI_SRC_FILES})

# Dirty hack to include FatFs into libopenui
# TODO: this must go away
#target_include_directories(libopenui PRIVATE
#  ${CMAKE_CURRENT_SOURCE_DIR}/../../FatFs)
