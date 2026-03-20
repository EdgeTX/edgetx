# Wrapper around wasi-sdk's pthread toolchain that uses the non-deprecated
# wasm32-wasip1-threads triple (compatible with SDK 25+).
if(NOT WASI_SDK_PREFIX)
  if(DEFINED ENV{WASI_SDK_PATH})
    set(WASI_SDK_PREFIX $ENV{WASI_SDK_PATH})
  elseif(EXISTS "/opt/wasi-sdk")
    set(WASI_SDK_PREFIX "/opt/wasi-sdk")
  endif()
endif()
include(${WASI_SDK_PREFIX}/share/cmake/wasi-sdk-pthread.cmake)
set(triple wasm32-wasip1-threads)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER_TARGET ${triple})
set(CMAKE_ASM_COMPILER_TARGET ${triple})
