include(gui/${GUI_DIR}/CMakeLists.txt)
include_directories(gui gui/${GUI_DIR})

if(NOT GUI_DIR STREQUAL colorlcd)
  set(SRC
    ${SRC}
    gui/navigation/common.cpp
    gui/navigation/navigation.cpp
    )
  include_directories(gui gui/common/stdlcd)
endif()

set(SRC
  ${SRC}
  gui/screenshot.cpp
  )
