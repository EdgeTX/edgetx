
set(firmwares_SRCS
  adjustmentreference.cpp
  boards.cpp
  curvedata.cpp
  curvereference.cpp
  customfunctiondata.cpp
  eeprominterface.cpp
  generalsettings.cpp
  gvardata.cpp
  input_data.cpp
  flightmodedata.cpp
  heli_data.cpp
  logicalswitchdata.cpp
  mixdata.cpp
  modeldata.cpp
  moduledata.cpp
  multiprotocols.cpp
  output_data.cpp
  radiodata.cpp
  radiodataconversionstate.cpp
  rawsource.cpp
  rawswitch.cpp
  sensordata.cpp
  telem_data.cpp
  timerdata.cpp
  afhds3.cpp
  ersky9x/ersky9xeeprom.cpp
  ersky9x/ersky9xinterface.cpp
  opentx/opentxeeprom.cpp
  opentx/opentxinterface.cpp
  edgetx/edgetxinterface.cpp
  edgetx/yaml_ops.cpp
  edgetx/yaml_generalsettings.cpp
  edgetx/yaml_calibdata.cpp
  edgetx/yaml_trainerdata.cpp
  edgetx/yaml_switchconfig.cpp
  edgetx/yaml_modeldata.cpp
  edgetx/yaml_rawswitch.cpp
  edgetx/yaml_rawsource.cpp
  edgetx/yaml_mixdata.cpp
  edgetx/yaml_expodata.cpp
  edgetx/yaml_curvedata.cpp
  edgetx/yaml_moduledata.cpp
  edgetx/yaml_logicalswitchdata.cpp
  edgetx/yaml_customfunctiondata.cpp
  edgetx/yaml_sensordata.cpp
  edgetx/yaml_screendata.cpp
  datahelpers.cpp
  customisation_data.cpp
  )

string(REPLACE ".cpp" ".h" firmwares_HDRS "${firmwares_SRCS}")

list(APPEND firmwares_HDRS
  eepromimportexport.h
  moduledata.h
  customisation_data.h
  )

set(firmwares_QT
  curvereference.h
  )

qt5_wrap_cpp(firmwares_SRCS ${firmwares_QT})

add_library(firmwares ${firmwares_SRCS} ${firmwares_HDRS})

target_link_libraries(firmwares ${CPN_COMMON_LIB} datamodels Qt5::Core Qt5::Xml Qt5::Widgets)
target_link_libraries(firmwares yaml-cpp)

target_include_directories(firmwares
  PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${COMPANION_SRC_DIRECTORY}/thirdparty/yaml-cpp/include
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/er9x"
    "${CMAKE_CURRENT_LIST_DIR}/ersky9x"
    "${CMAKE_CURRENT_LIST_DIR}/opentx"
    "${CMAKE_CURRENT_LIST_DIR}/edgetx"
  )
