
project(modeledit)

set(${PROJECT_NAME}_NAMES
  channels
  checklistdialog
  colorcustomscreens
  curves
  customfunctions
  expodialog
  flightmodes
  inputs
  heli
  logicalswitches
  mixerdialog
  mixes
  mixerslistwidget
  modeledit
  modeloptions
  setup_timer
  setup_module
  setup_function_switches
  setup
  telemetry
  telemetry_customscreens
)

AddHeadersSources()

# AUTOMOC does not detect so manually process
qt5_wrap_cpp(${PROJECT_NAME}_SRCS
  ${${PROJECT_NAME}_HDRS}
)

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_SRCS}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${CPN_COMMON_LIB}
    Qt5::Multimedia
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${maxLibQt_SOURCE_DIR}/widgets"
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
)
