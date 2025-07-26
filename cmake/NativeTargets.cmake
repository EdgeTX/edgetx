
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
  if(DEFINED ENV{LIBUSB1_ROOT_DIR})
    set(LIBUSB1_ROOT_DIR "$ENV{LIBUSB1_ROOT_DIR}")
  endif()

  find_package(Libusb1)

  if(LIBUSB1_FOUND)
    find_package(Dfuutil)
  endif()

  # environment variable set in github workflows and build-edgetx Dockerfile
  if(DEFINED ENV{OPENSSL_ROOT_DIR})
    set(OPENSSL_ROOT_DIR "$ENV{OPENSSL_ROOT_DIR}")
  endif()

  find_package(OpenSSL)
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
