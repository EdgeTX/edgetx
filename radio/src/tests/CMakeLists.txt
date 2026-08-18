
add_library(gtests-radio-lib STATIC EXCLUDE_FROM_ALL
  ${googletest_SOURCE_DIR}/googletest/src/gtest-all.cc
)
target_include_directories(gtests-radio-lib PRIVATE
  ${googletest_SOURCE_DIR}/googletest
)

set(TESTS_PATH ${RADIO_SRC_DIR}/tests)
set(TESTS_BUILD_PATH ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${RADIO_SRC_DIR}/tests/location.h.in ${CMAKE_CURRENT_BINARY_DIR}/location.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# AppleClang >=17 ASan hangs at startup on recent macOS (e.g. "Tahoe").
set(TESTS_ASAN_DEFAULT ON)
if(APPLE AND CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" AND
   CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 17.0)
  message(WARNING "Disabling ASan for gtests-radio: AppleClang "
    "${CMAKE_CXX_COMPILER_VERSION} is known to hang at startup on this "
    "platform. Pass -DTESTS_ASAN=ON to force it back on, e.g. with a "
    "non-Apple clang.")
  set(TESTS_ASAN_DEFAULT OFF)
endif()
option(TESTS_ASAN "Enable AddressSanitizer for gtests-radio" ${TESTS_ASAN_DEFAULT})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -O0 ${WARNING_FLAGS}")
if(TESTS_ASAN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

file(GLOB TEST_SRC_FILES ${RADIO_SRC_DIR}/tests/*.cpp
  CONFIGURE_DEPENDS "${RADIO_SRC_DIR}/tests/*.cpp")

set(TEST_SRC_FILES ${TEST_SRC_FILES}
  ${SIMU_SRC}
  ${RADIO_SRC_DIR}/targets/simu/arg_parser.cpp
  ${RADIO_SRC_DIR}/targets/simu/automation_capture.cpp
  ${RADIO_SRC_DIR}/targets/simu/automation_protocol.cpp
  ${RADIO_SRC_DIR}/targets/simu/automation_stdio.cpp
)

add_executable(gtests-radio EXCLUDE_FROM_ALL
  ${TEST_SRC_FILES}
)
target_compile_options(gtests-radio PRIVATE ${SIMU_SRC_OPTIONS})

target_link_libraries(gtests-radio gtests-radio-lib)
message(STATUS "Added optional gtests target")
