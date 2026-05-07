
option(DISABLE_COMPANION "Disable building companion and simulators" OFF)
option(EDGE_TX_BUILD_TESTS "Build native test targets" ON)

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
if(EDGE_TX_BUILD_TESTS)
  include(FetchGtest)
endif()

if(EDGE_TX_BUILD_TESTS)
  add_custom_target(tests-radio
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/gtests-radio
    DEPENDS gtests-radio
  )
endif()

if(Qt6Core_FOUND AND NOT DISABLE_COMPANION AND EDGE_TX_BUILD_TESTS)
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
  if(EDGE_TX_BUILD_TESTS)
    add_custom_target(gtests
      DEPENDS gtests-radio
    )
    add_custom_target(tests
      DEPENDS tests-radio
    )
  endif()
endif()

set(IGNORE "${ARM_TOOLCHAIN_DIR}")
