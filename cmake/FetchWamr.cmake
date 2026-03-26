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
set(WAMR_DISABLE_HW_BOUND_CHECK 1)

FetchContent_Declare(
  wamr
  GIT_REPOSITORY https://github.com/bytecodealliance/wasm-micro-runtime
  GIT_TAG        WAMR-2.4.4
  GIT_SHALLOW    TRUE
  UPDATE_DISCONNECTED TRUE
)

# Populate separately so we can patch before building
FetchContent_GetProperties(wamr)
if(NOT wamr_POPULATED)
  FetchContent_Populate(wamr)

  # WAMR 2.4.4 win_file.c has trailing backslashes in comments that
  # clang treats as line continuations, causing compile errors.
  execute_process(
    COMMAND git apply "${CMAKE_CURRENT_LIST_DIR}/wamr-fix-win-backslash-comments.patch"
    WORKING_DIRECTORY "${wamr_SOURCE_DIR}"
    RESULT_VARIABLE _patch_result
  )
  if(_patch_result)
    message(WARNING "WAMR win_file.c patch failed (may already be applied)")
  endif()

  add_subdirectory("${wamr_SOURCE_DIR}" "${wamr_BINARY_DIR}" EXCLUDE_FROM_ALL)

  if(WIN32)
    # WAMR unconditionally adds -lm and -ldl as PUBLIC link libraries,
    # which don't exist on Windows.
    get_target_property(_vmlib_libs vmlib INTERFACE_LINK_LIBRARIES)
    if(_vmlib_libs)
      list(REMOVE_ITEM _vmlib_libs "-lm" "-ldl")
      set_target_properties(vmlib PROPERTIES INTERFACE_LINK_LIBRARIES "${_vmlib_libs}")
    endif()

    # WAMR headers use __declspec(dllimport/dllexport) on MSVC-compatible
    # compilers, but we link vmlib statically. Override both API macros to
    # empty for vmlib itself (PUBLIC = PRIVATE + INTERFACE).
    target_compile_definitions(vmlib PUBLIC
      WASM_RUNTIME_API_EXTERN=
      WASM_API_EXTERN=
    )
  endif()
endif()
