# Standalone CMake project for building WASM simulator modules.
# Invoked on-demand by the superbuild's wasi-module target so that
# the WASI SDK is only fetched when WASM targets are actually built.
#
# Usage from superbuild:
#   cmake -S . -B build -DPCB=X10 -DPCBREV=TX16S
#   cmake --build build --target wasi-module

cmake_minimum_required(VERSION 3.21)
project(EdgeTX-WASM NONE)  # NONE = no compiler check (we set a toolchain below)

# Inherit the module path from the main project
list(APPEND CMAKE_MODULE_PATH "${EDGETX_SOURCE_DIR}/cmake")

# Collect command-line args to forward, excluding EDGETX_* (consumed here)
include(Macros)
CollectCommandLineArgs(_forwarded_args EXCLUDE "^EDGETX_")

# Fetch / find WASI SDK
include(FetchWasiSDK)
find_package(WasiSDK REQUIRED)

include(ExternalProject)

ExternalProject_Add(wasm
  SOURCE_DIR ${EDGETX_SOURCE_DIR}
  BINARY_DIR ${CMAKE_BINARY_DIR}/wasm-build
  CMAKE_ARGS
    ${_forwarded_args}
    -Wno-dev
    -DCMAKE_BUILD_TYPE=${EDGETX_BUILD_TYPE}
    -DCMAKE_TOOLCHAIN_FILE=${WASI_SDK_PATH}/share/cmake/wasi-sdk-pthread.cmake
  CMAKE_CACHE_ARGS
    -DEdgeTX_SUPERBUILD:BOOL=0
    -DNATIVE_BUILD:BOOL=1
    -DDISABLE_COMPANION:BOOL=1
  BUILD_COMMAND ${CMAKE_COMMAND} --build <BINARY_DIR> --target wasi-module --parallel
  INSTALL_COMMAND ""
)

# Copy .wasm files up after build
ExternalProject_Add_Step(wasm copy-wasm
  DEPENDEES build
  COMMAND ${CMAKE_COMMAND}
    -DSRC_DIR=${CMAKE_BINARY_DIR}/wasm-build -DDST_DIR=${CMAKE_BINARY_DIR}
    -P ${EDGETX_SOURCE_DIR}/cmake/CopyWasmModules.cmake
)
