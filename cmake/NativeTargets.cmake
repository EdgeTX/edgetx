
option(DISABLE_COMPANION "Disable building companion and simulators" OFF)

if(NOT DISABLE_COMPANION)
  include(QtDefs)
endif(NOT DISABLE_COMPANION)

find_package(SDL2 QUIET COMPONENTS SDL2 CONFIG)
if(TARGET SDL2::SDL2)
  message(STATUS "SDL2 found")
else()
  message(STATUS "SDL not found! Simulator audio, and joystick inputs, will not work.")
endif()

if(Qt6Core_FOUND AND NOT DISABLE_COMPANION)
  # environment variable set in github workflows and build-edgetx Dockerfile
  if(DEFINED ENV{OPENSSL_ROOT_DIR})
    set(OPENSSL_ROOT_DIR "$ENV{OPENSSL_ROOT_DIR}")
  endif()

  find_package(OpenSSL)

  include(FetchRsDfu)
  find_package(rs_dfu REQUIRED)
endif()

# Windows-specific includes and libs shared by sub-projects
if(WIN32)
  # TODO: is that still necessary?
  set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES OFF)
  set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES OFF)
  set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES OFF)
  set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES OFF)
endif()

# google tests
include(FetchGtest)

add_custom_target(tests-radio
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/gtests-radio
  DEPENDS gtests-radio
)

# Coverage target — configure with -DENABLE_COVERAGE=ON, then build this target.
# Requires lcov and genhtml to be installed (e.g. apt install lcov).
find_program(LCOV_EXECUTABLE lcov)
find_program(GENHTML_EXECUTABLE genhtml)
if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
  add_custom_target(coverage-radio
    COMMENT "Running tests and generating lcov HTML coverage report"
    # Reset counters
    COMMAND ${LCOV_EXECUTABLE} --zerocounters --directory ${CMAKE_CURRENT_BINARY_DIR}
    # Run tests
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/gtests-radio
    # Capture coverage data
    COMMAND ${LCOV_EXECUTABLE}
            --capture
            --directory ${CMAKE_CURRENT_BINARY_DIR}
            --output-file ${CMAKE_CURRENT_BINARY_DIR}/coverage.info
            --ignore-errors mismatch
    # Strip third-party and system headers
    COMMAND ${LCOV_EXECUTABLE}
            --remove ${CMAKE_CURRENT_BINARY_DIR}/coverage.info
            "${CMAKE_CURRENT_BINARY_DIR}/_deps/*"
            "/usr/*"
            "*/googletest/*"
            "*/tests/*"
            --output-file ${CMAKE_CURRENT_BINARY_DIR}/coverage_filtered.info
            --ignore-errors unused
    # Generate HTML report
    COMMAND ${GENHTML_EXECUTABLE}
            ${CMAKE_CURRENT_BINARY_DIR}/coverage_filtered.info
            --output-directory ${CMAKE_CURRENT_BINARY_DIR}/coverage_html
            --title "EdgeTX Radio Coverage"
            --legend
    COMMAND ${CMAKE_COMMAND} -E echo
            "Coverage report: ${CMAKE_CURRENT_BINARY_DIR}/coverage_html/index.html"
    DEPENDS gtests-radio
  )
  message(STATUS "Added optional coverage-radio target (lcov/genhtml found)")
else()
  message(STATUS "coverage-radio target unavailable: lcov=${LCOV_EXECUTABLE} genhtml=${GENHTML_EXECUTABLE}")
endif()

if(Qt6Core_FOUND AND NOT DISABLE_COMPANION)
  add_subdirectory(${COMPANION_SRC_DIRECTORY})
  add_custom_target(tests-companion
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/gtests-companion
    DEPENDS gtests-companion
  )
  add_custom_target(gtests
    DEPENDS gtests-radio gtests-companion
  )
  add_custom_target(tests
    DEPENDS tests-radio tests-companion
  )
else()
  add_custom_target(gtests
    DEPENDS gtests-radio
  )
  add_custom_target(tests
    DEPENDS tests-radio
  )
endif()

set(IGNORE "${ARM_TOOLCHAIN_DIR}")
