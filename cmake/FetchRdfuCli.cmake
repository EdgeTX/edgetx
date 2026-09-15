# Resolves RDFU_EXECUTABLE, the CLI tool used by the flash/flash-bl/flash-uf2
# targets (separate from FetchRsDfuLib.cmake, Companion's rs_dfu C++ library).
cmake_minimum_required(VERSION 3.14)

find_program(RDFU_EXECUTABLE NAMES rdfu)

if(NOT RDFU_EXECUTABLE)
  # Host, not target: must run on the machine invoking `make flash`.
  if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(RDFU_OS "Linux")
  elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
    set(RDFU_OS "macOS")
  elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(RDFU_OS "Windows")
  else()
    message(FATAL_ERROR "rdfu: unsupported host platform '${CMAKE_HOST_SYSTEM_NAME}'")
  endif()

  if(RDFU_OS STREQUAL "macOS")
    set(RDFU_ARCH "universal")
  else()
    # CMAKE_HOST_SYSTEM_PROCESSOR is empty in `cmake -P` script mode.
    cmake_host_system_information(RESULT RDFU_HOST_ARCH QUERY OS_PLATFORM)
    if(RDFU_HOST_ARCH MATCHES "x86_64|amd64|AMD64")
      set(RDFU_ARCH "x86_64")
    elseif(RDFU_HOST_ARCH MATCHES "aarch64|arm64|ARM64")
      set(RDFU_ARCH "arm64")
    else()
      message(FATAL_ERROR "rdfu: unsupported host architecture '${RDFU_HOST_ARCH}'")
    endif()
  endif()

  set(RDFU_EXT "")
  if(RDFU_OS STREQUAL "Windows")
    set(RDFU_EXT ".exe")
  endif()

  set(RDFU_FILENAME "rdfu-${RDFU_OS}-${RDFU_ARCH}${RDFU_EXT}")
  set(RDFU_URL "https://github.com/EdgeTX/rs-dfu/releases/latest/download/${RDFU_FILENAME}")

  # Manual cache (FetchContent needs a project/generator, unavailable in
  # -P mode), mirroring FetchContent's _deps/<name>-src/ layout.
  set(RDFU_CACHE_DIR "_deps/rdfucli-src")
  set(RDFU_CACHED_PATH "${RDFU_CACHE_DIR}/${RDFU_FILENAME}")

  if(NOT EXISTS "${RDFU_CACHED_PATH}")
    file(MAKE_DIRECTORY "${RDFU_CACHE_DIR}")
    message(STATUS "Fetching rdfu CLI tool from ${RDFU_URL}")
    file(DOWNLOAD "${RDFU_URL}" "${RDFU_CACHED_PATH}" STATUS RDFU_DOWNLOAD_STATUS)

    list(GET RDFU_DOWNLOAD_STATUS 0 RDFU_DOWNLOAD_CODE)
    if(NOT RDFU_DOWNLOAD_CODE EQUAL 0)
      list(GET RDFU_DOWNLOAD_STATUS 1 RDFU_DOWNLOAD_MSG)
      file(REMOVE "${RDFU_CACHED_PATH}")
      message(FATAL_ERROR "Failed to download rdfu CLI tool from ${RDFU_URL}: ${RDFU_DOWNLOAD_MSG}")
    endif()

    if(NOT RDFU_OS STREQUAL "Windows")
      file(CHMOD "${RDFU_CACHED_PATH}" PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
        )
    endif()
  endif()

  file(REAL_PATH "${RDFU_CACHED_PATH}" RDFU_EXECUTABLE)
endif()

if(NOT RDFU_EXECUTABLE)
  message(FATAL_ERROR "Could not find or fetch the rdfu CLI tool (see https://github.com/EdgeTX/rs-dfu)")
endif()
