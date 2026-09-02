# Wrapper around wasi-sdk's pthread toolchain that uses the non-deprecated
# wasm32-wasip1-threads triple (compatible with SDK 25+).

# Propagate WASI_SDK_PREFIX into try_compile sub-projects (used during ABI detection)
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES WASI_SDK_PREFIX)

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
