
project(firmwares)

set(${PROJECT_NAME}_NAMES
  adjustmentreference
  curvedata
  curvereference
  customfunctiondata
  customisation_data
  datahelpers
  firmware
  firmwarefactories
  flightmodedata
  generalsettings
  gvardata
  heli_data
  input_data
  logicalswitchdata
  mixdata
  modeldata
  moduledata
  multiprotocols
  output_data
  radiodata
  radiodataconversionstate
  rawsource
  rawswitch
  sensordata
  sourcenumref
  telem_data
  timerdata
  edgetx/edgetxinterface
  edgetx/yaml_calibdata
  edgetx/yaml_curvedata
  edgetx/yaml_customfunctiondata
  edgetx/yaml_expodata
  edgetx/yaml_generalsettings
  edgetx/yaml_logicalswitchdata
  edgetx/yaml_mixdata
  edgetx/yaml_modeldata
  edgetx/yaml_moduledata
  edgetx/yaml_ops
  edgetx/yaml_rawsource
  edgetx/yaml_rawswitch
  edgetx/yaml_screendata
  edgetx/yaml_sensordata
  edgetx/yaml_switchconfig
  edgetx/yaml_trainerdata
  edgetx/yaml_usbjoystickdata
)

AddHeadersSources()

# these have no cpp
set(${PROJECT_NAME}_HDRS
  ${${PROJECT_NAME}_HDRS}
  eeprominterface.h
  opentx/opentxinterface.h
)

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_HDRS}
  ${${PROJECT_NAME}_SRCS}
)

target_link_libraries(${PROJECT_NAME}
  ${CPN_COMMON_LIB}
  yaml-cpp
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_LIST_DIR}/opentx"
    "${CMAKE_CURRENT_LIST_DIR}/edgetx"
)
