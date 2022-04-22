include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SRC ${SRC} storage/yaml/yaml_tree_walker.cpp
               storage/yaml/yaml_parser.cpp
               storage/yaml/yaml_bits.cpp
               storage/yaml/yaml_datastructs.cpp)

if(STORAGE_MODELSLIST)
  set(SRC ${SRC} storage/yaml/yaml_modelslist.cpp)
endif()
          
# Warning: this file must be kept in sync with 'yaml_datastructs.cpp'
#          in the same directory.

if(PCB STREQUAL X12S)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x12s.cpp)
elseif(PCB STREQUAL X10)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x10.cpp)
elseif(PCB STREQUAL NV14)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_nv14.cpp)
elseif(PCB STREQUAL X7)
  if(PCBREV STREQUAL T12)
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_t12.cpp)
  elseif(PCBREV STREQUAL TLITE)
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_tlite.cpp)
  elseif(PCBREV STREQUAL TX12)
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_tx12.cpp)
  elseif(PCBREV STREQUAL TX12MK2)
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_tx12mk2.cpp)
  elseif(PCBREV STREQUAL ZORRO)
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_zorro.cpp)
  elseif(PCBREV STREQUAL T8)
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_t8.cpp)
  elseif(PCBREV STREQUAL TPRO)
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_tpro.cpp)
  else()
    set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x7.cpp)
  endif()
elseif(PCB STREQUAL X9LITE)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x9lite.cpp)
elseif(PCB STREQUAL X9LITES)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x9lites.cpp)
elseif(PCB STREQUAL XLITE)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_xlite.cpp)
elseif(PCB STREQUAL XLITES)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_xlites.cpp)
elseif(PCB STREQUAL X9E)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x9e.cpp)
elseif((PCB STREQUAL X9D) OR (PCB STREQUAL X9D+))
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x9d.cpp)
else()
  message(FATAL_ERROR "PCB '${PCB}' is not supported by YAML storage")
endif()

set(YAML_GEN          ${RADIO_DIRECTORY}/util/generate_yaml.py)
set(YAML_GEN_TEMPLATE ${RADIO_DIRECTORY}/util/yaml_parser.tmpl)

SET(YAML_NODES        "\"RadioData,ModelData,PartialModel\"")
set(YAML_GEN_ARGS     myeeprom.h ${YAML_GEN_TEMPLATE} ${YAML_NODES} -DYAML_GENERATOR)

get_property(flags DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
foreach(flag ${flags})
  #message(STATUS "dir='${dir}'")
  set(YAML_GEN_ARGS ${YAML_GEN_ARGS} -D${flag})
endforeach()

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  #message(STATUS "dir='${dir}'")
  set(YAML_GEN_ARGS ${YAML_GEN_ARGS} -I${dir})
endforeach()

# Force to build 32 bit binaries (Fix for Apple Silicon M1 hosts)
# TODO: find a way to detect APPLE & M1
# set(YAML_GEN_ARGS ${YAML_GEN_ARGS} -m32)

set(YAML_GEN_ARGS ${YAML_GEN_ARGS} -Wno-inconsistent-missing-override)

#message(STATUS "YAML_GEN_ARGS=${YAML_GEN_ARGS}")

add_custom_target(yaml_data
    WORKING_DIRECTORY ${RADIO_DIRECTORY}/src
    COMMAND ${PYTHON_EXECUTABLE} ${YAML_GEN} ${YAML_GEN_ARGS} ${SYSROOT_ARG} > ${YAML_GEN_OUTPUT}
    DEPENDS ${RADIO_DIRECTORY}/src/datastructs.h
            ${RADIO_DIRECTORY}/src/dataconstants.h
            ${RADIO_DIRECTORY}/src/myeeprom.h
            ${RADIO_DIRECTORY}/util/generate_yaml.py
)
