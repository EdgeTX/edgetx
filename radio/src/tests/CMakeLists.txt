

if(Qt5Widgets_FOUND)
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

  if(WIN32)
    target_include_directories(gtests-radio-lib PUBLIC ${WIN_INCLUDE_DIRS})
    target_link_libraries(gtests-radio-lib PRIVATE ${WIN_LINK_LIBRARIES})
  endif(WIN32)

  if(SDL2_FOUND)
    target_include_directories(gtests-radio-lib PUBLIC ${SDL2_INCLUDE_DIR})
    target_link_libraries(gtests-radio-lib PRIVATE ${SDL2_LIBRARIES})
  endif()


  file(GLOB TEST_SRC_FILES ${RADIO_SRC_DIR}/tests/*.cpp
    CONFIGURE_DEPENDS "${RADIO_SRC_DIR}/tests/*.cpp")

  set(TEST_SRC_FILES ${TEST_SRC_FILES}
    ${CMAKE_CURRENT_SOURCE_DIR}/location.h
    ${SIMU_SRC}
    )

  if(MINGW)
    # struct packing breaks on MinGW w/out -mno-ms-bitfields: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=52991 & http://stackoverflow.com/questions/24015852/struct-packing-and-alignment-with-mingw
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
  endif()

  add_executable(gtests-radio EXCLUDE_FROM_ALL
    ${TEST_SRC_FILES}
    )
  target_compile_options(gtests-radio PRIVATE ${SIMU_SRC_OPTIONS})

  add_dependencies(gtests-radio gtests-radio-lib)
  target_link_libraries(gtests-radio gtests-radio-lib pthread Qt5::Core Qt5::Widgets)
  message(STATUS "Added optional gtests target")
endif()
