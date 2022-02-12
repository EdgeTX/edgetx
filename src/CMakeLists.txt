if(HARDWARE_KEYS)
  add_definitions(-DHARDWARE_KEYS)
endif()

if(HARWARE_TOUCH)
  add_definitions(-DHARDWARE_TOUCH)
endif()

if(SOFTWARE_KEYBOARD)
  add_definitions(-DSOFTWARE_KEYBOARD)
endif()

set(LIBOPENUI_SRC
  libopenui_file.cpp
  bitmapbuffer.cpp
  window.cpp
  layer.cpp
  form.cpp
  button.cpp
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
  expansion_panel.cpp
  )

if(SOFTWARE_KEYBOARD)
  set(LIBOPENUI_SRC
    ${LIBOPENUI_SRC}
    keyboard_text.cpp
    keyboard_number.cpp
    keyboard_base.cpp
    )
endif()

foreach(LIBOPENUI_FILE ${LIBOPENUI_SRC})
  set(LIBOPENUI_SRC_FILES ${LIBOPENUI_SRC_FILES} ${LIBOPENUI_SRC_DIR}/src/${LIBOPENUI_FILE})
endforeach()

set(SRC ${SRC} ${LIBOPENUI_SRC_FILES})

# add_library(libopenui_native OBJECT ${LIBOPENUI_SRC_FILES})
# add_library(libopenui_stm32 OBJECT ${LIBOPENUI_SRC_FILES})

# Dirty hack to include FatFs into libopenui
# TODO: this must go away
#target_include_directories(libopenui PRIVATE
#  ${CMAKE_CURRENT_SOURCE_DIR}/../../FatFs)
