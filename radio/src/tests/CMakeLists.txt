
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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG} -O0 -fsanitize=address")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -O0 ${WARNING_FLAGS} -fsanitize=address")

file(GLOB TEST_SRC_FILES ${RADIO_SRC_DIR}/tests/*.cpp
  CONFIGURE_DEPENDS "${RADIO_SRC_DIR}/tests/*.cpp")

set(TEST_SRC_FILES ${TEST_SRC_FILES}
  ${CMAKE_CURRENT_SOURCE_DIR}/location.h
  ${SIMU_SRC}
)

add_executable(gtests-radio EXCLUDE_FROM_ALL
  ${TEST_SRC_FILES}
)
target_compile_options(gtests-radio PRIVATE ${SIMU_SRC_OPTIONS})

target_link_libraries(gtests-radio gtests-radio-lib)
message(STATUS "Added optional gtests target")
