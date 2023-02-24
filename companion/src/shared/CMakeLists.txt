
project(shared)

set(${PROJECT_NAME}_NAMES
  autobitsetcheckbox
  autocheckbox
  autocombobox
  autodoublespinbox
  autohexspinbox
  autolabel
  autolineedit
  autoprecisioncombobox
  autoslider
  autospinbox
  autotimeedit
  curveimagewidget
  curvedialog
  genericpanel
  verticalscrollarea
)

AddHeadersSources()

# AUTOMOC does not detect so manually process
qt5_wrap_cpp(${PROJECT_NAME}_SRCS
  ${${PROJECT_NAME}_HDRS}
)

# add after moc generation to stop warnings
set(${PROJECT_NAME}_SRCS
  ${${PROJECT_NAME}_SRCS}
  autowidget.h
  autowidget.cpp
  curveimage.h
  curveimage.cpp
)

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_SRCS}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${CPN_COMMON_LIB}
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
)
