#file(GLOB translations-new *.py)
file(GLOB trans-py-files ${RADIO_SRC_DIR}/translations-new/*.py)

#message(STATUS "${trans-py-files}")

foreach(trans-py-file ${trans-py-files})
  get_filename_component(language ${trans-py-file} NAME_WE)
  get_filename_component(path ${trans-py-file} DIRECTORY)
  set(target ${RADIO_BINARY_DIR}/translations-new/${language}.cpp)
  if(PYTHONINTERP_FOUND)
    add_custom_command(
      OUTPUT ${target}
      COMMAND ${PYTHON_EXECUTABLE} ${TOOLS_DIR}/encode-translation-new.py ${trans-py-file} ${target} ${language}
      DEPENDS ${translation} ${TOOLS_DIR}/encode-translation-new.py ${TOOLS_DIR}/charset.py
      )
  endif()
  list(APPEND translations_files_new ${target})
endforeach()

set(TRANSLATIONS_SRC ${translations_files_new})

#message(STATUS "SRC=${SRC}")
#target_sources(radio_translations_new PRIVATE ${translations_files_new})
#include_directories(${CMAKE_CURRENT_SOURCE_DIR})
#include_directories(${CMAKE_CURRENT_BINARY_DIR})

#include_directories(../ ${CMAKE_CURRENT_BINARY_DIR}/..)
#add_custom_target(radio_translations_new DEPENDS ${translations_files_new})
#set(TRANSLATIONS_SRC ${translations_files_new} PARENT_SCOPE)
#message(STATUS "TRANSLATIONS_SRC=${TRANSLATIONS_SRC}")
#message(STATUS ${translations_files_new})

#set(CMAKE_EXE_LINKER_FLAGS "-lm -T${RADIO_SRC_DIR}/${LINKER_SCRIPT} -Wl,-Map=firmware.map,--cref,--no-warn-mismatch,--gc-sections")

#add_library(radio_translations_new STATIC IMPORTED ${translations_files_new})
#target_compile_definitions(radio_translations_new PUBLIC ${APP_COMMON_DEFINES})  # set in top-level CMakeLists
#target_include_directories(radio_translations_new PUBLIC ../ ${CMAKE_CURRENT_BINARY_DIR}/..)
