include_directories(
  ${CMAKE_SOURCE_DIR}
  ${COMPANION_SRC_DIRECTORY}
  ${CMAKE_CURRENT_BINARY_DIR}
)

set(modeledit_NAMES
  modeledit
  setup
  heli
  curves
  telemetry
  expodialog
  mixerdialog
  checklistdialog
)

set(modeledit_SRCS
  flightmodes.cpp
  inputs.cpp
  mixes.cpp
  channels.cpp
  logicalswitches.cpp
  customfunctions.cpp
  mixerslistwidget.cpp
  # node.cpp  ## node and edge are built in common lib because also used by simulator
  # edge.cpp  ## commenting them here avoids a "duplicate target" warning eg. from ninja
  colorcustomscreens.cpp
  telemetry_customscreens.cpp
)

set(modeledit_HDRS
  flightmodes.h
  inputs.h
  mixes.h
  channels.h
  logicalswitches.h
  customfunctions.h
  mixerslistwidget.h
  # node.h
  colorcustomscreens.h
  telemetry_customscreens.h
)

set(modeledit_UIS
  setup_timer.ui
  setup_module.ui
  setup_function_switches.ui
  flightmode.ui
  telemetry_customscreen.ui
  telemetry_sensor.ui
)

foreach(name ${modeledit_NAMES})
  set(modeledit_SRCS ${modeledit_SRCS} ${name}.cpp)
  set(modeledit_HDRS ${modeledit_HDRS} ${name}.h)
  set(modeledit_UIS  ${modeledit_UIS}  ${name}.ui)
endforeach()

qt5_wrap_ui(modeledit_SRCS ${modeledit_UIS})
qt5_wrap_cpp(modeledit_SRCS ${modeledit_HDRS})

add_library(modeledit ${modeledit_SRCS})
target_link_libraries(modeledit PRIVATE datamodels ${CPN_COMMON_LIB} Qt5::Multimedia)
