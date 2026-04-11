
project(simulation)

set(${PROJECT_NAME}_NAMES
  debugoutput
  filteredtextbuffer
  radiooutputswidget
  simulateduiwidget
  simulatorinterface
  simulatormainwindow
  simulatorstartupdialog
  simulatorwidget
  simulatedgps
  telemetrysimu
  telemetryprovidercrossfire
  telemetryproviderfrsky
  telemetryproviderfrskyhub
  trainersimu
  widgets/lcdwidget
  widgets/radiowidget
  widgets/virtualjoystickwidget
  serialportsdialog
  hostserialbackend_serialport
  hostserialbackend_localsocket
  hostserialconnector
  simulateduiwidgetGeneric
  wasmsimulatorinterface
)

if(TARGET SDL2::SDL2)
  list(APPEND ${PROJECT_NAME}_NAMES
    joystick
    joystickdialog
  )
endif()

AddHeadersSources()

set(${PROJECT_NAME}_HDRS
  ${${PROJECT_NAME}_HDRS}
  hostserialbackend.h
  radiouiaction.h
  widgets/buttonswidget.h
  widgets/radiofaderwidget.h
  widgets/radiokeywidget.h
  widgets/radioknobwidget.h
  widgets/radioswitchwidget.h
  widgets/radiotrimwidget.h
  widgets/sliderwidget.h
)

# AUTOMOC does not detect so manually process
qt_wrap_cpp(${PROJECT_NAME}_SRCS
  ${${PROJECT_NAME}_HDRS}
)

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_SRCS}
)

include(FetchWamr)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${CPN_COMMON_LIB}
    Qt::Svg
    vmlib
)

if(TARGET SDL2::SDL2)
  target_link_libraries(${PROJECT_NAME} PUBLIC SDL2::SDL2)
endif()

target_include_directories(${PROJECT_NAME}
  PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/widgets"
    "${RADIO_SRC_DIR}/fonts/std"
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
)
