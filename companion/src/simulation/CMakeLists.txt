
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
  hostserialconnector
)

if(SDL2_FOUND)
  set(${PROJECT_NAME}_NAMES
    ${${PROJECT_NAME}_NAMES}
    joystick
    joystickdialog
  )
endif()

AddHeadersSources()

set(${PROJECT_NAME}_SRCS
  ${${PROJECT_NAME}_SRCS}
  simulateduiwidget9X.cpp
  simulateduiwidgetBoxer.cpp
  simulateduiwidgetCommando8.cpp
  simulateduiwidgetEL18.cpp
  simulateduiwidgetFatfishF16.cpp
  simulateduiwidgetJumperT12.cpp
  simulateduiwidgetJumperT12max.cpp
  simulateduiwidgetJumperT14.cpp
  simulateduiwidgetJumperT15.cpp
  simulateduiwidgetJumperT16.cpp
  simulateduiwidgetJumperT18.cpp
  simulateduiwidgetJumperT20.cpp
  simulateduiwidgetJumperTLITE.cpp
  simulateduiwidgetJumperTPRO.cpp
  simulateduiwidgetJumperTPROS.cpp
  simulateduiwidgetJumperBumblebee.cpp
  simulateduiwidgetLR3PRO.cpp
  simulateduiwidgetMT12.cpp
  simulateduiwidgetNV14.cpp
  simulateduiwidgetPL18.cpp
  simulateduiwidgetPocket.cpp
  simulateduiwidgetT8.cpp
  simulateduiwidgetTX12.cpp
  simulateduiwidgetTX16S.cpp
  simulateduiwidgetV16.cpp
  simulateduiwidgetX10.cpp
  simulateduiwidgetX12.cpp
  simulateduiwidgetX7.cpp
  simulateduiwidgetX9.cpp
  simulateduiwidgetX9D2019.cpp
  simulateduiwidgetX9E.cpp
  simulateduiwidgetX9LITE.cpp
  simulateduiwidgetXLITE.cpp
  simulateduiwidgetZorro.cpp
)

set(${PROJECT_NAME}_HDRS
  ${${PROJECT_NAME}_HDRS}
  radiouiaction.h
  simulateduiwidgetGeneric.h
  widgets/buttonswidget.h
  widgets/radiofaderwidget.h
  widgets/radiokeywidget.h
  widgets/radioknobwidget.h
  widgets/radioswitchwidget.h
  widgets/radiotrimwidget.h
  widgets/sliderwidget.h
)

# AUTOMOC does not detect so manually process
qt5_wrap_cpp(${PROJECT_NAME}_SRCS
  ${${PROJECT_NAME}_HDRS}
)

# AUTOUIC does not detect as ui in .h so manually process
qt5_wrap_ui(${PROJECT_NAME}_SRCS
  simulateduiwidgetGeneric.ui
)

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_SRCS}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${CPN_COMMON_LIB}
    Qt5::Svg
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/widgets"
    "${RADIO_SRC_DIR}/fonts/std"
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
)
