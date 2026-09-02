# FindWasiSDK.cmake
# Module to find WASI SDK installation
#
# This module defines the following variables:
#   WasiSDK_FOUND         - True if WASI SDK is found
#   WasiSDK_ROOT_DIR      - Root directory of WASI SDK installation
#
# Usage:
#   set(WASI_SDK_PATH "/path/to/wasi-sdk")
#   find_package(WasiSDK REQUIRED)

cmake_minimum_required(VERSION 3.10)

# Use WASI_SDK_PATH if provided, otherwise try to find it
if(WASI_SDK_PATH)
    set(_wasi_sdk_search_paths "${WASI_SDK_PATH}")
else()
    # Default search paths for common WASI SDK installations
    set(_wasi_sdk_search_paths
        "$ENV{WASI_SDK_PATH}"
        "/opt/wasi-sdk"
    )
endif()

# Find the WASI SDK root directory by looking for the toolchain file
find_path(WasiSDK_ROOT_DIR
    NAMES share/cmake/wasi-sdk.cmake
    PATHS ${_wasi_sdk_search_paths}
    DOC "WASI SDK root directory"
    NO_DEFAULT_PATH
)

# Handle find_package arguments
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WasiSDK
    DEFAULT_MSG
    WasiSDK_ROOT_DIR
)

# Mark variables as advanced
mark_as_advanced(
    WasiSDK_ROOT_DIR
)

if(WasiSDK_FOUND)
    set(WASI_SDK_PATH ${WasiSDK_ROOT_DIR})
endif()
