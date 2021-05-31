include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(SRC ${SRC} storage/yaml/yaml_tree_walker.cpp
               storage/yaml/yaml_parser.cpp
               storage/yaml/yaml_bits.cpp
               storage/yaml/yaml_modelslist.cpp
               storage/yaml/yaml_datastructs.cpp)

if(PCB STREQUAL X12S)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x12s.cpp)
elseif(PCB STREQUAL X10)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x10.cpp)
elseif(PCB STREQUAL X7)
  set(YAML_GEN_OUTPUT storage/yaml/yaml_datastructs_x7.cpp)
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

set(YAML_GEN_ARGS ${YAML_GEN_ARGS} -DRTOS_COOS -Wno-inconsistent-missing-override)

#message(STATUS "YAML_GEN_ARGS=${YAML_GEN_ARGS}")

add_custom_target(yaml_data
    WORKING_DIRECTORY ${RADIO_DIRECTORY}/src
    COMMAND ${PYTHON_EXECUTABLE} ${YAML_GEN} ${YAML_GEN_ARGS} > ${YAML_GEN_OUTPUT}
    DEPENDS ${RADIO_DIRECTORY}/src/datastructs.h
            ${RADIO_DIRECTORY}/src/dataconstants.h
            ${RADIO_DIRECTORY}/src/myeeprom.h
            ${RADIO_DIRECTORY}/util/generate_yaml.py
)
