
if(NOT DISABLE_COMPANION)
  include(QtDefs)
  if(Qt5Core_FOUND)
    message(STATUS "build companion: ON")
    set(BUILD_COMPANION 1)
  else()
    message(STATUS "Qt5 not found")
  endif()
endif()

if(NOT DISABLE_SIMULATOR)
  message(STATUS "build simulator: ON")
  set(BUILD_SIMULATOR 1)
endif()

if(NOT BUILD_COMPANION AND NOT BUILD_SIMULATOR)
  return()
endif()

find_package(SDL2 QUIET COMPONENTS SDL2 CONFIG)
if(TARGET SDL2::SDL2)
  message(STATUS "SDL2 found")
else()
  message(STATUS "SDL not found! Simulator audio, and joystick inputs, will not work.")
endif()

if(BUILD_COMPANION)
  find_package(Libusb1)
  if(LIBUSB1_FOUND)
    find_package(Dfuutil)
  endif()

  if(LINUX)
    find_package(Libssl1)
  endif()

  # OpenSSL
  # environment variable set in github workflows and build-edgetx Dockerfile
  if (DEFINED ENV{OPENSSL_ROOT_DIR})
    set(OPENSSL_ROOT_DIR "$ENV{OPENSSL_ROOT_DIR}")
  endif()

  find_package(OpenSSL)
endif()

# Windows-specific includes and libs shared by sub-projects
if(WIN32)
  list(APPEND WIN_INCLUDE_DIRS "${RADIO_SRC_DIR}/thirdparty/windows/dirent")
  # TODO: is that still necessary?
  set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES OFF)
  set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES OFF)
  set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES OFF)
  set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES OFF)
endif()

# google tests
include(FetchGtest)

if(BUILD_SIMULATOR)
  add_custom_target(tests-radio
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/gtests-radio
    DEPENDS gtests-radio
  )
  list(APPEND run_test_list tests-radio)
  list(APPEND build_test_list gtests-radio)
endif()

if(BUILD_COMPANION)
  add_subdirectory(${COMPANION_SRC_DIRECTORY})

  add_custom_target(tests-companion
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/gtests-companion
    DEPENDS gtests-companion
  )
  list(APPEND run_test_list tests-companion)
  list(APPEND build_test_list gtests-companion)
endif()

add_custom_target(gtests
  DEPENDS ${build_test_list}
)

add_custom_target(tests
  DEPENDS ${run_test_list}
)

set(IGNORE "${ARM_TOOLCHAIN_DIR}")
