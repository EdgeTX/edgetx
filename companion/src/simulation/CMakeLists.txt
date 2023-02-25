
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
  telemetrysimu
  trainersimu
  widgets/lcdwidget
  widgets/radiowidget
  widgets/virtualjoystickwidget
)

AddHeadersSources()

set(${PROJECT_NAME}_SRCS
  ${${PROJECT_NAME}_SRCS}
  simulateduiwidget9X.cpp
  simulateduiwidgetX7.cpp
  simulateduiwidgetX9.cpp
  simulateduiwidgetX9D2019.cpp
  simulateduiwidgetX9E.cpp
  simulateduiwidgetX9LITE.cpp
  simulateduiwidgetX10.cpp
  simulateduiwidgetX12.cpp
  simulateduiwidgetXLITE.cpp
  simulateduiwidgetBoxer.cpp
  simulateduiwidgetCommando8.cpp
  simulateduiwidgetLR3PRO.cpp
  simulateduiwidgetJumperT12.cpp
  simulateduiwidgetJumperTLITE.cpp
  simulateduiwidgetJumperTPRO.cpp
  simulateduiwidgetJumperT16.cpp
  simulateduiwidgetJumperT18.cpp
  simulateduiwidgetNV14.cpp
  simulateduiwidgetT8.cpp
  simulateduiwidgetTX12.cpp
  simulateduiwidgetTX16S.cpp
  simulateduiwidgetZorro.cpp
)

set(${PROJECT_NAME}_HDRS
  ${${PROJECT_NAME}_HDRS}
  radiouiaction.h
  widgets/buttonswidget.h
  widgets/radiofaderwidget.h
  widgets/radiokeywidget.h
  widgets/radioknobwidget.h
  widgets/radioswitchwidget.h
  widgets/radiotrimwidget.h
  widgets/sliderwidget.h
)

if(SDL2_FOUND)
  set(${PROJECT_NAME}_SRCS
    ${${PROJECT_NAME}_SRCS}
    joystick.cpp
    joystickdialog.cpp
  )

  set(${PROJECT_NAME}_HDRS
    ${${PROJECT_NAME}_HDRS}
    joystick.h
    joystickdialog.h
  )
endif()

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
