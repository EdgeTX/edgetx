if(HARDWARE_KEYS)
  add_definitions(-DHARDWARE_KEYS)
endif()

if(HARWARE_TOUCH)
  add_definitions(-DHARDWARE_TOUCH)
endif()

include_directories(
  ${RADIO_SRC_DIR}/gui/colorlcd
  ${RADIO_SRC_DIR}/gui/colorlcd/layouts
  ${RADIO_SRC_DIR}/gui/colorlcd/libui
  ${RADIO_SRC_DIR}/gui/colorlcd/model
  ${RADIO_SRC_DIR}/gui/colorlcd/radio
  ${RADIO_SRC_DIR}/gui/colorlcd/module
  ${RADIO_SRC_DIR}/gui/colorlcd/mainview
  ${RADIO_SRC_DIR}/gui/colorlcd/themes
  ${RADIO_SRC_DIR}/gui/colorlcd/controls
)

file(GLOB THEMES_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd
  ${RADIO_SRC_DIR}/gui/colorlcd/themes/*.cpp)

file(GLOB LAYOUTS_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd
  ${RADIO_SRC_DIR}/gui/colorlcd/layouts/*.cpp)

file(GLOB WIDGETS_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd
  ${RADIO_SRC_DIR}/gui/colorlcd/widgets/*.cpp)

file(GLOB LIBUI_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd
  ${RADIO_SRC_DIR}/gui/colorlcd/libui/*.cpp)

file(GLOB MAINVIEW_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd
  ${RADIO_SRC_DIR}/gui/colorlcd/mainview/*.cpp)

file(GLOB CONTROLS_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd
  ${RADIO_SRC_DIR}/gui/colorlcd/controls/*.cpp)

set(GUI_SRC
  ${GUI_SRC}
  ${LIBUI_SRC}
  ${THEMES_SRC}
  ${LAYOUTS_SRC}
  ${WIDGETS_SRC}
  ${MAINVIEW_SRC}
  ${CONTROLS_SRC}
  bitmaps.cpp
  colors.cpp
  fonts.cpp
  lcd.cpp
  LvglWrapper.cpp
  startup_shutdown.cpp

  model/curveedit.cpp
  model/input_edit_adv.cpp
  model/input_edit.cpp
  model/menu_model.cpp
  model/mixer_edit_adv.cpp
  model/mixer_edit.cpp
  model/model_curves.cpp
  model/model_flightmodes.cpp
  model/model_gvars.cpp
  model/model_inputs.cpp
  model/model_logical_switches.cpp
  model/model_mixes.cpp
  model/model_outputs.cpp
  model/model_select.cpp
  model/model_setup.cpp
  model/model_telemetry.cpp
  model/model_templates.cpp
  model/output_edit.cpp
  model/preflight_checks.cpp
  model/special_functions.cpp
  model/throttle_params.cpp
  model/timer_setup.cpp
  model/trainer_setup.cpp
  model/trims_setup.cpp

  module/bind_menu_d16.cpp
  module/custom_failsafe.cpp
  module/module_setup.cpp
  module/ppm_settings.cpp

  radio/hw_extmodule.cpp
  radio/hw_inputs.cpp
  radio/hw_intmodule.cpp
  radio/hw_serial.cpp
  radio/menu_radio.cpp
  radio/preview_window.cpp
  radio/radio_calibration.cpp
  radio/radio_diaganas.cpp
  radio/radio_diagkeys.cpp
  radio/radio_hardware.cpp
  radio/radio_sdmanager.cpp
  radio/radio_setup.cpp
  radio/radio_theme.cpp
  radio/radio_tools.cpp
  radio/radio_trainer.cpp
  radio/radio_version.cpp
  )

macro(add_gui_src src)
  set(GUI_SRC
    ${GUI_SRC}
    ${src}
    )
endmacro(add_gui_src)

if(LUA)
  add_gui_src(standalone_lua.cpp)
  set(SRC ${SRC}
    lua/lua_widget.cpp
    lua/lua_widget_factory.cpp
    lua/widgets.cpp
    lua/lua_lvgl_widget.cpp
    )
endif()

if(LUA_MIXER)
  add_gui_src(model/model_mixer_scripts.cpp)
endif()

if(FUNCTION_SWITCHES)
  add_gui_src(model/function_switches.cpp)
  add_gui_src(radio/radio_diagcustswitches.cpp)
endif()

if(GVARS)
  add_gui_src(model/model_gvars.cpp)
endif()

if(HELI)
  add_gui_src(model/model_heli.cpp)
endif()

if(PXX2 OR MULTIMODULE)
  add_gui_src(radio/radio_spectrum_analyser.cpp)
endif()

if(CROSSFIRE)
  add_gui_src(module/crossfire_settings.cpp)
endif()

if(AFHDS2)
  add_gui_src(module/afhds2a_settings.cpp)
endif()

if(AFHDS3)
  add_gui_src(module/afhds3_settings.cpp)
  add_gui_src(module/afhds3_options.cpp)
endif()

if(MULTIMODULE)
  add_gui_src(module/mpm_settings.cpp)
  add_gui_src(module/multi_rfprotos.cpp)
endif()

if(INTERNAL_MODULE_PXX1)
  add_gui_src(module/pxx1_settings.cpp)
endif()

if(PXX2)
  add_gui_src(module/access_settings.cpp)
endif()

if(GHOST)
  add_gui_src(radio/radio_ghost_module_config.cpp)
endif()

if(BLUETOOTH)
  add_gui_src(model/trainer_bluetooth.cpp)
  add_gui_src(radio/hw_bluetooth.cpp)
endif()

if(USBJ_EX)
  add_gui_src(model/model_usbjoystick.cpp)
endif()

set(SRC ${SRC} storage/modelslist.cpp)

option(UI_PERF_MONITOR "Draw frame rate and CPU usage" OFF)
if(UI_PERF_MONITOR)
  add_definitions(-DUI_PERF_MONITOR)
endif()

# include lvgl
include(${RADIO_SRC_DIR}/gui/colorlcd/CMakeListsLVGL.txt)

list(TRANSFORM LVGL_SOURCES_MINIMAL PREPEND ${RADIO_SRC_DIR}/)
list(TRANSFORM LVGL_FONT_SOURCES_MINIMAL PREPEND ${RADIO_SRC_DIR}/)

set(BOOTLOADER_SRC ${BOOTLOADER_SRC}
  ${LVGL_SOURCES_MINIMAL}
  ${LVGL_FONT_SOURCES_MINIMAL}
  ${RADIO_SRC_DIR}/gui/colorlcd/lcd.cpp
  ${RADIO_SRC_DIR}/gui/colorlcd/fonts.cpp
  ${RADIO_SRC_DIR}/gui/colorlcd/colors.cpp
  ${RADIO_SRC_DIR}/gui/colorlcd/bitmaps.cpp
  ${RADIO_SRC_DIR}/gui/colorlcd/libui/bitmapbuffer.cpp
  ${RADIO_SRC_DIR}/thirdparty/lz4/lz4.c
)

set(SRC ${SRC}
  ${LVGL_SOURCES}
  ${LVGL_FONT_SOURCES}
  ${RADIO_SRC_DIR}/thirdparty/lz4/lz4.c
)

add_definitions(-DLIBOPENUI)
