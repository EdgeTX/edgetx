# Fetch rs_dfu

include(FetchContent)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  set(rs_dfu_target_arch "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
  set(rs_dfu_target_arch "aarch64")
else()
  message(FATAL_ERROR "rs_dfu does not have support for architecture ${CMAKE_SYSTEM_PROCESSOR}!")
endif()

if(WIN32)
  set(rs_dfu_target_os "pc-windows-msvc")
elseif(APPLE)
  set(rs_dfu_target_os "apple-darwin")
else()
  set(rs_dfu_target_os "unknown-linux-gnu")
endif()

set(rs_dfu_asset "rs_dfu-${rs_dfu_target_arch}-${rs_dfu_target_os}.tar.gz")

FetchContent_Declare(
  rs_dfu
  URL     "https://github.com/edgetx/rs-dfu/releases/latest/download/${rs_dfu_asset}"
)

FetchContent_MakeAvailable(rs_dfu)

set(rs_dfu_DIR "${rs_dfu_SOURCE_DIR}/cmake")

message("Fetched rs_dfu source code from Github: ${rs_dfu_SOURCE_DIR}")
