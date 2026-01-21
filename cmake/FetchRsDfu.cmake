# Fetch rs_dfu
cmake_minimum_required(VERSION 3.14)

include(FetchContent)

# Determine platform and architecture
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(RS_DFU_PLATFORM "unknown-linux-gnu")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(RS_DFU_PLATFORM "apple-darwin")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(RS_DFU_PLATFORM "pc-windows-msvc")
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_SYSTEM_NAME}")
endif()

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
  set(RS_DFU_ARCH "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
  set(RS_DFU_ARCH "aarch64")
else()
  message(FATAL_ERROR "Unsupported architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(RS_DFU_BASE_URL "https://github.com/EdgeTX/rs-dfu/releases/latest/download")
set(RS_DFU_FILENAME "rs_dfu-${RS_DFU_ARCH}-${RS_DFU_PLATFORM}")
set(RS_DFU_URL "${RS_DFU_BASE_URL}/${RS_DFU_FILENAME}.tar.gz")

FetchContent_Declare(
  RsDfu
  URL ${RS_DFU_URL}
  DOWNLOAD_EXTRACT_TIMESTAMP true
  FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable(RsDfu)

set(rs_dfu_DIR "${rsdfu_SOURCE_DIR}/cmake")

message("Fetched rsdfu source code from Github: ${rsdfu_SOURCE_DIR}")
