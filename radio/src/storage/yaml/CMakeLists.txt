include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SRC ${SRC} storage/yaml/yaml_tree_walker.cpp
               storage/yaml/yaml_parser.cpp
               storage/yaml/yaml_bits.cpp
               storage/yaml/yaml_datastructs.cpp)

if(STORAGE_MODELSLIST)
  set(SRC ${SRC} storage/yaml/yaml_labelslist.cpp)
  set(SRC ${SRC} storage/yaml/yaml_modelslist.cpp)
  add_definitions(-DSTORAGE_MODELSLIST)
endif()

# Warning: this file must be kept in sync with 'yaml_datastructs.cpp'
#          in the same directory.

if((PCB STREQUAL X10) OR (PCB STREQUAL X12S) OR (PCB STREQUAL STM32H7S78_DK))
  if(PCBREV STREQUAL T15)
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_t15.cpp)
  elseif(PCBREV STREQUAL F16)
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_f16.cpp)
  else()
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x10.cpp)
  endif()
elseif(PCB STREQUAL NV14)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_nv14.cpp)
elseif(PCB STREQUAL PL18)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_pl18.cpp)
elseif(PCB STREQUAL X7)
  if(PCBREV STREQUAL TPRO)
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_tpro.cpp)
  elseif((PCBREV STREQUAL T20) OR (PCBREV STREQUAL T20V2))
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_t20.cpp)
  elseif(PCBREV STREQUAL GX12)
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_gx12.cpp)
  elseif((PCBREV STREQUAL COMMANDO8) OR (PCBREV STREQUAL LR3PRO) OR (PCBREV STREQUAL T8) OR (PCBREV STREQUAL T12) OR (PCBREV STREQUAL TLITE))
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_xlite.cpp)
  else()
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_128x64.cpp)
  endif()
elseif(PCB STREQUAL X9LITE)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_128x64.cpp)
elseif(PCB STREQUAL X9LITES)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_128x64.cpp)
elseif(PCB STREQUAL XLITE)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_xlite.cpp)
elseif(PCB STREQUAL XLITES)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_xlites.cpp)
elseif(PCB STREQUAL X9E)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x9e.cpp)
elseif((PCB STREQUAL X9D) OR (PCB STREQUAL X9D+))
  if(PCBREV STREQUAL 2019)
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x9dp2019.cpp)
  else()
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x9d.cpp)
  endif()
else()
  message(FATAL_ERROR "PCB '${PCB}' is not supported by YAML storage")
endif()

set(YAML_GEN          ${RADIO_DIRECTORY}/util/generate_yaml.py)
set(YAML_GEN_TEMPLATE ${RADIO_DIRECTORY}/util/yaml_parser.tmpl)

SET(YAML_NODES        "\"RadioData,ModelData,PartialModel\"")
set(YAML_GEN_ARGS     myeeprom.h ${YAML_GEN_TEMPLATE} ${YAML_NODES} -DYAML_GENERATOR)

AddCompilerFlags(YAML_GEN_ARGS)

# Force to build 32 bit binaries (Fix for Apple Silicon M1 hosts)
# TODO: find a way to detect APPLE & M1
# set(YAML_GEN_ARGS ${YAML_GEN_ARGS} -m32)

set(YAML_GEN_ARGS ${YAML_GEN_ARGS}
  -Wno-inconsistent-missing-override
  -Wno-asm-operand-widths
  ${SYSROOT_ARG})

add_custom_target(yaml_data
    WORKING_DIRECTORY ${RADIO_DIRECTORY}/src
    COMMAND ${PYTHON_EXECUTABLE} ${YAML_GEN} ${YAML_GEN_ARGS} > ${YAML_GEN_OUTPUT}
    DEPENDS ${RADIO_DIRECTORY}/src/datastructs.h
            ${RADIO_DIRECTORY}/src/dataconstants.h
            ${RADIO_DIRECTORY}/src/myeeprom.h
            ${RADIO_DIRECTORY}/util/generate_yaml.py
)

AddHWGenTarget(${HW_DESC_JSON} yaml_inputs yaml_inputs.inc)
set(SRC ${SRC} ${CMAKE_CURRENT_BINARY_DIR}/yaml_inputs.inc)

# helper target for testing purposes
add_custom_target(yaml_inputs DEPENDS ${HW_DESC_JSON} yaml_inputs.inc)

