# Runs rdfu for the flash/flash-bl/flash-uf2 targets in radio/src/CMakeLists.txt.
# Invoked via `cmake -P` so resolving/fetching the binary happens at build
# time, not on every configure.
#
# -D args: RDFU_SUBCOMMAND, RDFU_FILE, RDFU_VENDOR, RDFU_PRODUCT,
# RDFU_START_ADDRESS (optional, omitted for UF2 writes)
cmake_minimum_required(VERSION 3.14)

include(${CMAKE_CURRENT_LIST_DIR}/FetchRdfuCli.cmake)

set(RDFU_ARGS ${RDFU_SUBCOMMAND} --vendor ${RDFU_VENDOR} --product ${RDFU_PRODUCT})
if(DEFINED RDFU_START_ADDRESS AND NOT RDFU_START_ADDRESS STREQUAL "")
  list(APPEND RDFU_ARGS --start-address ${RDFU_START_ADDRESS})
endif()
list(APPEND RDFU_ARGS ${RDFU_FILE})

execute_process(
  COMMAND ${RDFU_EXECUTABLE} ${RDFU_ARGS}
  RESULT_VARIABLE RDFU_RESULT
  )

if(NOT RDFU_RESULT EQUAL 0)
  message(FATAL_ERROR "rdfu ${RDFU_SUBCOMMAND} failed (exit code ${RDFU_RESULT})")
endif()
