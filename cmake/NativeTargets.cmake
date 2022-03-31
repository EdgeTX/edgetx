
option(DISABLE_COMPANION "Disable building companion and simulators" OFF)

# if(WIN32)
#   set(WIN_EXTRA_LIBS_PATH "C:/Programs" CACHE PATH
#     "Base path to extra libs/headers on Windows (SDL & pthreads folders should be in here).")
#   list(APPEND CMAKE_PREFIX_PATH "${WIN_EXTRA_LIBS_PATH}" "${WIN_EXTRA_LIBS_PATH}/SDL")  # hints for FindSDL
# endif()

# libfox
if(NOT WIN32)
  find_package(Fox QUIET)  # QUIET not working on WIN32?
  if (FOX_FOUND)
    message(STATUS "Foxlib found at ${FOX_LIBRARY}")
  else() # if(NOT WIN32)
    message("Libfox not found, simu target will not be available")
  endif()
endif()

if(NOT DISABLE_COMPANION)
  include(QtDefs)
endif(NOT DISABLE_COMPANION)

if(Qt5Core_FOUND OR FOX_FOUND)
  set(SDL_BUILDING_LIBRARY YES)  # this prevents FindSDL from appending SDLmain lib to the results, which we don't want
  find_package("SDL")
  if(SDL_FOUND)
    message(STATUS "SDL Lib: ${SDL_LIBRARY}; Headers: ${SDL_INCLUDE_DIR}; Version: ${SDL_VERSION_STRING}")
    if (NOT ${SDL_VERSION_STRING} VERSION_LESS "2.0")
      message(FATAL_ERROR "OpenTX requires SDL 1.x, not 2.x")
    endif()
  else()
    message(STATUS "SDL not found! Simulator audio, and joystick inputs, will not work.")
  endif()
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
include(DownloadGTest)

add_custom_target(tests-radio
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/gtests-radio
  DEPENDS gtests-radio
  )

if(Qt5Core_FOUND AND NOT DISABLE_COMPANION)
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
