# Fetch and configure WAMR (WebAssembly Micro Runtime)
cmake_minimum_required(VERSION 3.14)

include(FetchContent)

# WAMR build configuration
set(WAMR_BUILD_INTERP 1)
set(WAMR_BUILD_FAST_INTERP 0)
set(WAMR_BUILD_AOT 0)
set(WAMR_BUILD_JIT 0)
set(WAMR_BUILD_LIBC_BUILTIN 1)
set(WAMR_BUILD_LIBC_WASI 1)
set(WAMR_BUILD_LIB_WASI_THREADS 1)
set(WAMR_BUILD_SHARED_MEMORY 1)
set(WAMR_BUILD_BULK_MEMORY 1)
set(WAMR_BUILD_REF_TYPES 1)
set(WAMR_BUILD_SIMD 0)
set(WAMR_BUILD_EXCE_HANDLING 1)

FetchContent_Declare(
  wamr
  GIT_REPOSITORY https://github.com/bytecodealliance/wasm-micro-runtime
  GIT_TAG        WAMR-2.4.4
  GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(wamr)

# WAMR 2.4.4 win_file.c has trailing backslashes in comments that
# clang treats as line continuations, causing compile errors.
if(WIN32)
  target_compile_options(vmlib PRIVATE -Wno-backslash-newline-escape)
endif()
