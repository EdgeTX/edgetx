set(GUI_SRC
  ${GUI_SRC}
  lcd.cpp
  splash.cpp
  popups.cpp
  widgets.cpp
  startup_shutdown.cpp
  menu_model.cpp
  model_select.cpp
  model_setup.cpp
  model_failsafe.cpp
  model_logical_switches.cpp
  model_special_functions.cpp
  model_outputs.cpp
  model_telemetry.cpp
  model_telemetry_sensor.cpp
  menu_radio.cpp
  radio_setup.cpp
  radio_calibration.cpp
  view_main.cpp
  view_statistics.cpp
  )

if(FLIGHT_MODES)
  set(GUI_SRC
    ${GUI_SRC}
    model_flightmodes.cpp
    )
endif()

if(GVARS)
  set(GUI_SRC ${GUI_SRC}
    model_gvars.cpp
  )
endif()

if(LUA_MIXER)
  set(GUI_SRC
    ${GUI_SRC}
    model_custom_scripts.cpp
    )
endif()

include(gui/common/stdlcd/CMakeLists.txt)

set(BOOTLOADER_SRC ${BOOTLOADER_SRC}
  ${RADIO_SRC_DIR}/gui/212x64/lcd.cpp
  ${RADIO_SRC_DIR}/gui/common/stdlcd/fonts.cpp
  ${RADIO_SRC_DIR}/gui/common/stdlcd/utf8.cpp
)
