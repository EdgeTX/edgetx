

if(Qt5Widgets_FOUND)
  add_library(gtests-companion-lib STATIC EXCLUDE_FROM_ALL
    ${googletest_SOURCE_DIR}/googletest/src/gtest-all.cc
  )
  target_include_directories(gtests-companion-lib PRIVATE
    ${googletest_SOURCE_DIR}/googletest
  )
  add_definitions(-DGTESTS)

  set(TESTS_PATH ${COMPANION_SRC_DIRECTORY}/tests)
  set(RADIO_TESTS_PATH ${RADIO_SRC_DIR}/tests)
  configure_file(${TESTS_PATH}/location.h.in ${CMAKE_CURRENT_BINARY_DIR}/location.h @ONLY)

  include_directories(${CMAKE_CURRENT_BINARY_DIR})

  if(WIN32)
    target_include_directories(gtests-companion-lib PUBLIC ${WIN_INCLUDE_DIRS})
    target_link_libraries(gtests-companion-lib PRIVATE ${WIN_LINK_LIBRARIES})
  endif(WIN32)

  file(GLOB TEST_SRC_FILES ${TESTS_PATH}/*.cpp)

  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 ${WARNING_FLAGS}")

  add_executable(gtests-companion EXCLUDE_FROM_ALL ${TEST_SRC_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/location.h.in)
  add_dependencies(gtests-companion gtests-companion-lib)
  target_link_libraries(gtests-companion gtests-companion-lib simulation firmwares storage common)
  message(STATUS "Added optional gtests-companion target")
else()
  message(WARNING "WARNING: gtests target will not be available (check that Qt5Widgets are configured).")
endif()
