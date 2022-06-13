set(GTEST_ROOT /usr CACHE STRING "Base path to Google Test headers and source.")

find_path(GTEST_INCDIR gtest/gtest.h HINTS "${GTEST_ROOT}/include" DOC "Path to Google Test header files folder ('gtest/gtest.h').")
find_path(GTEST_SRCDIR src/gtest-all.cc HINTS "${GTEST_ROOT}" "${GTEST_ROOT}/src/gtest" DOC "Path of Google Test 'src' folder.")

if(NOT GTEST_INCDIR OR NOT GTEST_SRCDIR)
  message(STATUS "Googletest will be downloaded")

  set(GTEST_VERSION 1.8.1)

  set(GTEST_SRCDIR "${RADIO_SRC_DIR}/tests/googletest-release-${GTEST_VERSION}/googletest")
  set(GTEST_INCDIR "${GTEST_SRCDIR}/include")

  set(GTEST_URL     "https://github.com/google/googletest/archive/release-${GTEST_VERSION}.tar.gz")
  set(GTEST_ARCHIVE "${RADIO_SRC_DIR}/tests/gtest-${GTEST_VERSION}.tar.gz")

  if (NOT EXISTS "${GTEST_SRCDIR}")
    file(DOWNLOAD "${GTEST_URL}" ${GTEST_ARCHIVE} SHOW_PROGRESS)
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar -xf ${GTEST_ARCHIVE}
      WORKING_DIRECTORY ${RADIO_SRC_DIR}/tests)
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E remove -f ${GTEST_ARCHIVE})
  endif()
endif()
