cmake_minimum_required(VERSION 3.14)

set(WASI_SDK_VERSION "25")
set(WASI_SDK_FULL_VERSION "${WASI_SDK_VERSION}.0")

# Determine platform and architecture
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
  set(WASI_SDK_PLATFORM "linux")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
  set(WASI_SDK_PLATFORM "macos")
elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
  set(WASI_SDK_PLATFORM "windows")
else()
  message(FATAL_ERROR "Unsupported platform: ${CMAKE_HOST_SYSTEM_NAME}")
endif()

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64|amd64|AMD64")
  set(WASI_SDK_ARCH "x86_64")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
  set(WASI_SDK_ARCH "arm64")
else()
  message(FATAL_ERROR "Unsupported Linux architecture: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
endif()

set(WASI_SDK_BASE_URL "https://github.com/WebAssembly/wasi-sdk/releases/download")
set(WASI_SDK_FILENAME "wasi-sdk-${WASI_SDK_VERSION}.0-${WASI_SDK_ARCH}-${WASI_SDK_PLATFORM}")
set(WASI_SDK_URL "${WASI_SDK_BASE_URL}/wasi-sdk-${WASI_SDK_VERSION}/${WASI_SDK_FILENAME}.tar.gz")

set(FETCHCONTENT_QUIET FALSE)

include(FetchContent)
FetchContent_Declare(
  WasiSDK
  URL      ${WASI_SDK_URL}
  DOWNLOAD_EXTRACT_TIMESTAMP true
  FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable(WasiSDK)

set(WASI_SDK_PATH ${wasisdk_SOURCE_DIR})
