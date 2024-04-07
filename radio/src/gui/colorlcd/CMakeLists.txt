include_directories(
  ${RADIO_SRC_DIR}/gui/colorlcd
  ${RADIO_SRC_DIR}/gui/colorlcd/layouts
  ${CMAKE_CURRENT_BINARY_DIR}/fonts
  ${RADIO_SRC_DIR}/thirdparty/libopenui/src
  ${RADIO_SRC_DIR}/thirdparty/libopenui/thirdparty
)

file(GLOB THEMES_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd
  ${RADIO_SRC_DIR}/gui/colorlcd/themes/*.cpp)

file(GLOB LAYOUTS_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd
  ${RADIO_SRC_DIR}/gui/colorlcd/layouts/*.cpp)

file(GLOB WIDGETS_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd
  ${RADIO_SRC_DIR}/gui/colorlcd/widgets/*.cpp)

set(GUI_SRC
  ${GUI_SRC}
  ${THEMES_SRC}
  ${LAYOUTS_SRC}
  ${WIDGETS_SRC}
  bitmaps.cpp
  channel_bar.cpp
  channel_range.cpp
  color_editor.cpp
  color_list.cpp
  color_picker.cpp
  colors.cpp
  curve.cpp
  file_browser.cpp
  file_preview.cpp
  fonts.cpp
  fullscreen_dialog.cpp
  special_functions.cpp
  layout.cpp
  lcd.cpp
  list_line_button.cpp
  listbox.cpp
  LvglWrapper.cpp
  menu_model.cpp
  menu_radio.cpp
  menu_screen.cpp
  page.cpp
  popups.cpp
  screen_setup.cpp
  screen_user_interface.cpp
  select_fab_carousel.cpp
  sourcechoice.cpp
  startup_shutdown.cpp
  switch_warn_dialog.cpp
  switchchoice.cpp
  tabsgroup.cpp
  theme_manager.cpp
  theme.cpp
  topbar.cpp
  view_about.cpp
  view_channels.cpp
  view_logical_switches.cpp
  view_main_decoration.cpp
  view_main_menu.cpp
  view_main.cpp
  view_statistics.cpp
  view_text.cpp
  widget_settings.cpp
  widget.cpp
  widgets_setup.cpp
  curve_param.cpp
  curveedit.cpp
  fm_matrix.cpp
  gvar_numberedit.cpp
  input_edit_adv.cpp
  input_edit.cpp
  input_source.cpp
  mixer_edit_adv.cpp
  mixer_edit.cpp
  model_curves.cpp
  model_flightmodes.cpp
  model_gvars.cpp
  model_inputs.cpp
  model_logical_switches.cpp
  model_mixes.cpp
  model_outputs.cpp
  model_select.cpp
  model_setup.cpp
  model_telemetry.cpp
  model_templates.cpp
  output_edit.cpp
  preflight_checks.cpp
  throttle_params.cpp
  timeedit.cpp
  timer_setup.cpp
  trainer_setup.cpp
  trims_setup.cpp
  bind_menu_d16.cpp
  custom_failsafe.cpp
  hw_extmodule.cpp
  hw_intmodule.cpp
  module_setup.cpp
  ppm_settings.cpp
  file_carosell.cpp
  hw_inputs.cpp
  hw_serial.cpp
  preview_window.cpp
  radio_calibration.cpp
  radio_diaganas.cpp
  radio_diagkeys.cpp
  radio_hardware.cpp
  radio_sdmanager.cpp
  radio_setup.cpp
  radio_theme.cpp
  radio_tools.cpp
  radio_trainer.cpp
  radio_version.cpp
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
    )
endif()

if(LUA_MIXER)
  add_gui_src(model_mixer_scripts.cpp)
endif()

if(HELI)
  add_gui_src(model_heli.cpp)
endif()

if(PXX2 OR MULTIMODULE)
  add_gui_src(radio_spectrum_analyser.cpp)
endif()

if(CROSSFIRE)
  add_gui_src(crossfire_settings.cpp)
endif()

if(AFHDS2)
  add_gui_src(afhds2a_settings.cpp)
endif()

if(AFHDS3)
  add_gui_src(afhds3_settings.cpp)
  add_gui_src(afhds3_options.cpp)
endif()

if(MULTIMODULE)
  add_gui_src(mpm_settings.cpp)
endif()

if(INTERNAL_MODULE_PXX1)
  add_gui_src(pxx1_settings.cpp)
endif()

if(PXX2)
  add_gui_src(access_settings.cpp)
endif()

if(MULTIMODULE)
  add_gui_src(multi_rfprotos.cpp)
endif()

if(GHOST)
  add_gui_src(radio_ghost_module_config.cpp)
endif()

if(BLUETOOTH)
  add_gui_src(trainer_bluetooth.cpp)
  add_gui_src(hw_bluetooth.cpp)
endif()

set(SRC ${SRC} storage/modelslist.cpp)

option(WINDOWS_INSPECT_BORDERS "Draw bounding boxes around windows" OFF)
if(WINDOWS_INSPECT_BORDERS)
  add_definitions(-DWINDOWS_INSPECT_BORDERS)
endif()

option(UI_PERF_MONITOR "Draw frame rate and CPU usage" OFF)
if(UI_PERF_MONITOR)
  add_definitions(-DUI_PERF_MONITOR)
endif()

# includes libopenui
set(LIBOPENUI_SRC_DIR thirdparty/libopenui)
set(LVGL_SRC_DIR ${LIBOPENUI_SRC_DIR}/thirdparty/lvgl/src)

include(${LIBOPENUI_SRC_DIR}/src/CMakeLists.txt)
include(${LIBOPENUI_SRC_DIR}/thirdparty/CMakeLists.txt)

set(SRC ${SRC} ${LVGL_FONT_SOURCES})

add_definitions(-DLIBOPENUI)

if(USBJ_EX)
  add_gui_src(model_usbjoystick.cpp)
endif()
