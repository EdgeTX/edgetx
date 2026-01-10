
cmake_minimum_required(VERSION 3.13)
project(EdgeTX)

set(VERSION_MAJOR "3")
set(VERSION_MINOR "0")
set(VERSION_REVISION "0")
set(CODENAME "dev")
string(TIMESTAMP BUILD_YEAR "%Y")

if(MINGW OR WIN32)
  set(CMAKE_OBJECT_PATH_MAX 200)
endif()

if(DEFINED ENV{EDGETX_VERSION_TAG})
  set(VERSION_TAG "$ENV{EDGETX_VERSION_TAG}")
else()
  if(DEFINED ENV{EDGETX_VERSION_PREFIX})
    set(VERSION_PREFIX "$ENV{EDGETX_VERSION_PREFIX}-")
  else()
    set(VERSION_PREFIX "pre-") # Default prefix for when not tagged
  endif()
  if(DEFINED ENV{EDGETX_VERSION_SUFFIX})
    set(VERSION_SUFFIX "-$ENV{EDGETX_VERSION_SUFFIX}")
  else()
    set(VERSION_SUFFIX "-selfbuild") # Default suffix for when not in automated build environments
  endif()
endif()

set(VERSION_FAMILY ${VERSION_MAJOR}.${VERSION_MINOR})
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION})

option(EdgeTX_SUPERBUILD "Cross-compile EdgeTX for each toolchain" ON)
include(ExternalProject)

if(EdgeTX_SUPERBUILD)
  # Fetch all command line args into CMAKE_ARGS
  get_cmake_property(CACHE_VARS CACHE_VARIABLES)
  foreach(CACHE_VAR ${CACHE_VARS})
    get_property(CACHE_VAR_HELPSTRING CACHE ${CACHE_VAR} PROPERTY HELPSTRING)
    if(CACHE_VAR_HELPSTRING STREQUAL "No help, variable specified on the command line.")
      get_property(CACHE_VAR_TYPE CACHE ${CACHE_VAR} PROPERTY TYPE)
      if(CACHE_VAR_TYPE STREQUAL "UNINITIALIZED")
        set(CACHE_VAR_TYPE)
      else()
        set(CACHE_VAR_TYPE :${CACHE_VAR_TYPE})
      endif()
      list(APPEND CMAKE_ARGS "-D${CACHE_VAR}${CACHE_VAR_TYPE}=${${CACHE_VAR}}")
    endif()
  endforeach()
  message("-- CMAKE_ARGS: ${CMAKE_ARGS}")
  message("-- CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

  # Add explicit targets for triggering cmake in the external projects
  set_property(DIRECTORY PROPERTY EP_STEP_TARGETS configure clean)

  # Native targets
  ExternalProject_Add(native
    SOURCE_DIR ${CMAKE_SOURCE_DIR}
    BINARY_DIR ${CMAKE_BINARY_DIR}/native
    CMAKE_ARGS ${CMAKE_ARGS} -Wno-dev -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    CMAKE_CACHE_ARGS
      -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_SOURCE_DIR}/cmake/toolchain/native.cmake
      -DEdgeTX_SUPERBUILD:BOOL=0
      -DNATIVE_BUILD:BOOL=1
    INSTALL_COMMAND  ""
    EXCLUDE_FROM_ALL TRUE
    )

  # ARM targets
  ExternalProject_Add(arm-none-eabi
    SOURCE_DIR ${CMAKE_SOURCE_DIR}
    BINARY_DIR ${CMAKE_BINARY_DIR}/arm-none-eabi
    CMAKE_ARGS ${CMAKE_ARGS} -Wno-dev -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    CMAKE_CACHE_ARGS
      -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_SOURCE_DIR}/cmake/toolchain/arm-none-eabi.cmake
      -DEdgeTX_SUPERBUILD:BOOL=0
      -DNATIVE_BUILD:BOOL=0
    INSTALL_COMMAND  ""
    EXCLUDE_FROM_ALL TRUE
  )

  if(DEFINED CMAKE_BUILD_PARALLEL_LEVEL)
    set(PARALLEL_FLAG --parallel ${CMAKE_BUILD_PARALLEL_LEVEL})
  else()
    set(PARALLEL_FLAG --parallel)
  endif()

  add_custom_target(configure
    DEPENDS native-configure arm-none-eabi-configure)

  add_custom_target(libsimulator
    COMMAND ${CMAKE_COMMAND} --build native --target libsimulator ${PARALLEL_FLAG}
    DEPENDS native-configure
  )

  add_custom_target(simulator
    COMMAND ${CMAKE_COMMAND} --build native --target simulator ${PARALLEL_FLAG}
    DEPENDS native-configure
  )

  add_custom_target(simu
    COMMAND ${CMAKE_COMMAND} --build native --target simu ${PARALLEL_FLAG}
  )

  add_custom_target(companion
    COMMAND ${CMAKE_COMMAND} --build native --target companion ${PARALLEL_FLAG}
    DEPENDS native-configure
  )

  add_custom_target(gtests-radio
    COMMAND ${CMAKE_COMMAND} --build native --target gtests-radio ${PARALLEL_FLAG}
    DEPENDS native-configure
  )

  add_custom_target(tests-radio
    COMMAND ${CMAKE_COMMAND} --build native --target tests-radio ${PARALLEL_FLAG}
    DEPENDS native-configure
  )

  add_custom_target(bootloader
    COMMAND ${CMAKE_COMMAND} --build arm-none-eabi --target bootloader ${PARALLEL_FLAG}
    DEPENDS arm-none-eabi-configure
  )

  add_custom_target(firmware
    COMMAND ${CMAKE_COMMAND} --build arm-none-eabi --target firmware ${PARALLEL_FLAG}
    DEPENDS arm-none-eabi-configure
  )

  add_custom_target(firmware-size
    COMMAND ${CMAKE_COMMAND} --build arm-none-eabi --target firmware-size ${PARALLEL_FLAG}
    DEPENDS arm-none-eabi-configure
  )

  # Quit cmake at this point
  return()
endif()

enable_language(ASM)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
include(GenericDefinitions)

if (VERSION_TAG)
  message(STATUS "EdgeTX ${VERSION_TAG} @ ${GIT_STR}")
else()
  message(STATUS "EdgeTX ${VERSION_PREFIX}${VERSION}${VERSION_SUFFIX} @ ${GIT_STR}")
endif()

if(NATIVE_BUILD)
  include(NativeTargets)
else()
  # Prevent CMake warnings
  set(IGNORE "${LIBUSB1_ROOT_DIR}" "${OPENSSL_ROOT_DIR}")
endif()

add_subdirectory(${RADIO_SRC_DIR})
