if(HARDWARE_KEYS)
  add_definitions(-DHARDWARE_KEYS)
endif()

if(HARWARE_TOUCH)
  add_definitions(-DHARDWARE_TOUCH)
endif()

set(LIBOPENUI_SOURCES
  libopenui_file.cpp
  bitmapbuffer.cpp
  bitmapbuffer_fileio.cpp
  bitmapbuffer_draw_extra.cpp
  window.cpp
  layer.cpp
  form.cpp
  button.cpp
  button_matrix.cpp
  static.cpp
  toggleswitch.cpp
  slider.cpp
  mainwindow.cpp
  menu.cpp
  menutoolbar.cpp
  choice.cpp
  filechoice.cpp
  numberedit.cpp
  textedit.cpp
  progress.cpp
  table.cpp
  modal_window.cpp
  dialog.cpp
  keyboard_text.cpp
  keyboard_number.cpp
  keyboard_base.cpp
  )

list(TRANSFORM LIBOPENUI_SOURCES PREPEND ${LIBOPENUI_SRC_DIR}/src/)

set(LIBOPENUI_SOURCES ${LIBOPENUI_SOURCES}
  ${LIBOPENUI_SRC_DIR}/thirdparty/lz4/lz4.c
)
