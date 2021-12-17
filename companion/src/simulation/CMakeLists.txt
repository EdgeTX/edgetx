set(simulation_SRCS
  debugoutput.cpp
  filteredtextbuffer.cpp
  radiooutputswidget.cpp
  simulateduiwidget.cpp
  simulateduiwidget9X.cpp
  simulateduiwidgetX9LITE.cpp
  simulateduiwidgetX7.cpp
  simulateduiwidgetX9.cpp
  simulateduiwidgetXLITE.cpp
  simulateduiwidgetX9E.cpp
  simulateduiwidgetX9D2019.cpp
  simulateduiwidgetX10.cpp
  simulateduiwidgetX12.cpp
  simulateduiwidgetJumperT12.cpp
  simulateduiwidgetJumperTLITE.cpp
  simulateduiwidgetJumperT16.cpp
  simulateduiwidgetJumperT18.cpp
  simulateduiwidgetTX12.cpp
  simulateduiwidgetZorro.cpp
  simulateduiwidgetT8.cpp
  simulateduiwidgetTX16S.cpp
  simulateduiwidgetNV14.cpp
  simulatorinterface.cpp
  simulatormainwindow.cpp
  simulatorstartupdialog.cpp
  simulatorwidget.cpp
  telemetrysimu.cpp
  trainersimu.cpp
  widgets/radiowidget.cpp
  widgets/virtualjoystickwidget.cpp
  widgets/lcdwidget.cpp
)

set(simulation_UIS
  debugoutput.ui
  radiooutputswidget.ui
  simulateduiwidget9X.ui
  simulateduiwidgetX9LITE.ui
  simulateduiwidgetX7.ui
  simulateduiwidgetX9.ui
  simulateduiwidgetXLITE.ui
  simulateduiwidgetX9E.ui
  simulateduiwidgetX9D2019.ui
  simulateduiwidgetX10.ui
  simulateduiwidgetX12.ui
  simulateduiwidgetJumperT12.ui
  simulateduiwidgetJumperTLITE.ui
  simulateduiwidgetJumperT16.ui
  simulateduiwidgetJumperT18.ui
  simulateduiwidgetTX12.ui
  simulateduiwidgetZorro.ui
  simulateduiwidgetT8.ui
  simulateduiwidgetTX16S.ui
  simulateduiwidgetNV14.ui
  simulatormainwindow.ui
  simulatorstartupdialog.ui
  simulatorwidget.ui
  telemetrysimu.ui
  trainersimu.ui
)

set(simulation_HDRS
  debugoutput.h
  filteredtextbuffer.h
  radiooutputswidget.h
  radiouiaction.h
  simulateduiwidget.h
  # simulator.h
  simulatorinterface.h
  simulatormainwindow.h
  simulatorstartupdialog.h
  simulatorwidget.h
  telemetrysimu.h
  trainersimu.h
  widgets/buttonswidget.h
  widgets/lcdwidget.h
  widgets/radiowidget.h
  widgets/radiofaderwidget.h
  widgets/radiokeywidget.h
  widgets/radioknobwidget.h
  widgets/radioswitchwidget.h
  widgets/radiotrimwidget.h
  widgets/sliderwidget.h
  widgets/virtualjoystickwidget.h
)

if(SDL_FOUND)
  set(simulation_SRCS
    ${simulation_SRCS}
    joystick.cpp
    joystickdialog.cpp
  )

  set(simulation_HDRS
    ${simulation_HDRS}
    joystick.h
    joystickdialog.h
  )

  set(simulation_UIS
    ${simulation_UIS}
    joystickdialog.ui
  )
endif()

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${COMPANION_SRC_DIRECTORY}
  ${COMPANION_SRC_DIRECTORY}/simulation
  ${COMPANION_SRC_DIRECTORY}/simulation/widgets
  ${RADIO_SRC_DIR}/fonts/std
)

qt5_wrap_ui(simulation_SRCS ${simulation_UIS})
qt5_wrap_cpp(simulation_SRCS ${simulation_HDRS})

add_library(simulation ${simulation_SRCS} ${simulation_HDRS})
target_link_libraries(simulation PRIVATE ${CPN_COMMON_LIB} Qt5::Core Qt5::Widgets Qt5::Svg)
