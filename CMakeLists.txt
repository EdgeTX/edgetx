
cmake_minimum_required(VERSION 3.13)
project(EdgeTX)

set(VERSION_MAJOR "3")
set(VERSION_MINOR "0")
set(VERSION_REVISION "0")
set(CODENAME "dev")
string(TIMESTAMP BUILD_YEAR "%Y")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

if(MINGW OR WIN32)
  set(CMAKE_OBJECT_PATH_MAX 200)
endif()

if(DEFINED ENV{EDGETX_VERSION_TAG})
  set(VERSION_TAG "$ENV{EDGETX_VERSION_TAG}")
else()
  if(DEFINED ENV{EDGETX_VERSION_SUFFIX})
    set(VERSION_SUFFIX "$ENV{EDGETX_VERSION_SUFFIX}")
  else()
    set(VERSION_SUFFIX "selfbuild") # If not defined, set suffix to selfbuild
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

  option(DISABLE_RADIO "Disable building radio" OFF)
  option(DISABLE_COMPANION "Disable building companion" OFF)
  option(DISABLE_SIMULATOR "Disable building simulator" OFF)

  if(NOT DISABLE_COMPANION OR NOT DISABLE_SIMULATOR)
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

    if(NOT DISABLE_COMPANION)
      add_custom_target(companion
        COMMAND $(MAKE) -C native companion
        DEPENDS native-configure
      )
    endif()

    if(NOT DISABLE_SIMULATOR)
      add_custom_target(libsimulator
        COMMAND $(MAKE) -C native libsimulator
        DEPENDS native-configure
      )

      add_custom_target(simulator
        COMMAND $(MAKE) -C native simulator
        DEPENDS native-configure
      )

      add_custom_target(simu
        COMMAND $(MAKE) -C native simu
        DEPENDS native-configure
      )

      add_custom_target(gtests-radio
        COMMAND $(MAKE) -C native gtests-radio
        DEPENDS native-configure
      )

      add_custom_target(tests-radio
        COMMAND $(MAKE) -C native tests-radio
        DEPENDS native-configure
      )
    endif()
  endif()

  if(NOT DISABLE_RADIO)
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

    # add_custom_target(configure
    #   DEPENDS native-configure arm-none-eabi-configure)

    add_custom_target(bootloader
      COMMAND $(MAKE) -C arm-none-eabi bootloader
      DEPENDS arm-none-eabi-configure
    )
    add_custom_target(firmware
      COMMAND $(MAKE) -C arm-none-eabi firmware
      DEPENDS arm-none-eabi-configure
    )
    add_custom_target(firmware-size
      COMMAND $(MAKE) -C arm-none-eabi firmware-size
      DEPENDS arm-none-eabi-configure
    )
  endif()

  # Webasm targets
  if(ENABLE_WASM)
    include(FetchWasiSDK)
    find_package(WasiSDK REQUIRED)
 
    ExternalProject_Add(wasm
      SOURCE_DIR ${CMAKE_SOURCE_DIR}
      BINARY_DIR ${CMAKE_BINARY_DIR}/wasm
      CMAKE_ARGS
        ${CMAKE_ARGS}
        -Wno-dev
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_TOOLCHAIN_FILE=${WASI_SDK_PATH}/share/cmake/wasi-sdk-pthread.cmake
      CMAKE_CACHE_ARGS
        -DEdgeTX_SUPERBUILD:BOOL=0
        -DNATIVE_BUILD:BOOL=1
        -DDISABLE_COMPANION:BOOL=1
      INSTALL_COMMAND  ""
      EXCLUDE_FROM_ALL TRUE
    )

    add_custom_target(wasi-module
      COMMAND $(MAKE) -C wasm wasi-module
      DEPENDS wasm-configure
    )
  endif()

  # Quit cmake at this point
  return()
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
include(GenericDefinitions)

if (VERSION_TAG)
  message(STATUS "EdgeTX ${VERSION_TAG} @ ${GIT_STR}")
else()
  message(STATUS "EdgeTX ${VERSION}-${VERSION_SUFFIX} @ ${GIT_STR}")
endif()

if(NATIVE_BUILD)
  if(NOT WASI)
    include(NativeTargets)
  endif()
  if(NOT DISABLE_SIMULATOR)
    add_subdirectory(${RADIO_SRC_DIR})
  endif()
else()
  add_subdirectory(${RADIO_SRC_DIR})

  # Prevent CMake warnings
  set(IGNORE "${SDL2_LIBRARY_PATH}" "${LIBSSL1_ROOT_DIR}" "${OPENSSL_ROOT_DIR}")
endif()

# Prevent CMake warnings
set(IGNORE
  "${ENABLE_WASM}"    
  "${DISABLE_RADIO}"
  "${DISABLE_COMPANION}"
  "${DISABLE_SIMULATOR}"
)
