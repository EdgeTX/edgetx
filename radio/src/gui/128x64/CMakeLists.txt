
if(RADIO_CALIBRATION_HALL)
  set(RADIO_CALIBRATION_SRC radio_calibration_hall.cpp)
else()
  set(RADIO_CALIBRATION_SRC radio_calibration.cpp)
endif()

set(GUI_SRC
  ${GUI_SRC}
  draw_functions.cpp
  splash.cpp
  popups.cpp
  widgets.cpp
  startup_shutdown.cpp
  model_select.cpp
  menu_model.cpp
  model_setup.cpp
  model_failsafe.cpp
  model_logical_switches.cpp
  model_special_functions.cpp
  model_outputs.cpp
  model_telemetry.cpp
  model_telemetry_sensor.cpp
  menu_radio.cpp
  radio_setup.cpp
  ${RADIO_CALIBRATION_SRC}
  view_main.cpp
  view_statistics.cpp
  )

if(FLIGHT_MODES)
  set(GUI_SRC
    ${GUI_SRC}
    model_flightmodes.cpp
    )
endif()

if(LUA_MIXER)
  set(GUI_SRC
    ${GUI_SRC}
    model_custom_scripts.cpp
    )
endif()

include(gui/common/stdlcd/CMakeLists.txt)
