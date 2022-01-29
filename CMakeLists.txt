project(EdgeTX)

set(VERSION_MAJOR "2")
set(VERSION_MINOR "7")
set(VERSION_REVISION "0")
set(CODENAME "dev")
set(VERSION_SUFFIX $ENV{OPENTX_VERSION_SUFFIX})
set(VERSION_FAMILY ${VERSION_MAJOR}.${VERSION_MINOR})
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}${VERSION_SUFFIX})

cmake_minimum_required(VERSION 3.13)

# Automatically link Qt executables to qtmain target on Windows
# https://cmake.org/cmake/help/latest/policy/CMP0020.html
cmake_policy(SET CMP0020 NEW)

# Allow keyword and plain target_link_libraries() signatures to be mixed
# https://cmake.org/cmake/help/latest/policy/CMP0023.html
cmake_policy(SET CMP0023 OLD)

# Use @rpath in a target's install name on MacOS X for locating shared libraries
# https://cmake.org/cmake/help/latest/policy/CMP0042.html
if(POLICY CMP0042)
  cmake_policy(SET CMP0042 NEW)
endif()

# Ignore COMPILE_DEFINITIONS_<Config> properties (prefer generator expressions)
# https://cmake.org/cmake/help/latest/policy/CMP0043.html
if(POLICY CMP0043)
  cmake_policy(SET CMP0043 NEW)
endif()

# Only interpret if() arguments as variables or keywords when unquoted
# https://cmake.org/cmake/help/latest/policy/CMP0054.html
if(POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

# Custom command byproducts must be explicit - for ninja
# https://cmake.org/cmake/help/latest/policy/CMP0058.html
if(POLICY CMP0058)
  cmake_policy(SET CMP0058 NEW)
endif()

set(CMAKE_COLOR_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-D_GLIBCXX_USE_C99=1) # proper to_string definition

set(RADIO_DIRECTORY ${PROJECT_SOURCE_DIR}/radio)
set(RADIO_SRC_DIR ${RADIO_DIRECTORY}/src)
set(COMPANION_SRC_DIRECTORY ${PROJECT_SOURCE_DIR}/companion/src)
set(SIMU_SRC_DIRECTORY ${COMPANION_SRC_DIRECTORY}/simulation)
set(TOOLS_DIR ${PROJECT_SOURCE_DIR}/tools)
set(UTILS_DIR ${PROJECT_SOURCE_DIR}/radio/util)
set(LIBOPENUI_TOOLS_DIR ${RADIO_SRC_DIR}/thirdparty/libopenui/tools)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

include(Macros)
today(DATE)
now(TIME)
git_id(GIT_STR)

message(STATUS "EdgeTX ${VERSION} @ ${GIT_STR}")

# options shared by all targets

option(APP_CUSTOM_DBG_HANDLER "Use custom message handler for debug output in all Qt apps." ON)
set(APP_CUSTOM_DBG_LEVEL "0" CACHE STRING
  "Debug output level: 0=debug+; 1=info+; 2=warn+; 3=critical+; 4=fatal only.")
set(APP_CUSTOM_DBG_SHOW_FILE "DEFAULT" CACHE STRING
  "Show source file names in debug output. 1/0/DEFAULT (default is ON with DEBUG builds).")

option(VERBOSE_CMAKELISTS "Show extra information while processing CMakeLists.txt files." OFF)
option(WARNINGS_AS_ERRORS "Treat any compiler warning as an error (adds -Werror flag)." OFF)

if(WIN32)
  set(WIN_EXTRA_LIBS_PATH "C:/Programs" CACHE PATH
    "Base path to extra libs/headers on Windows (SDL & pthreads folders should be in here).")
  list(APPEND CMAKE_PREFIX_PATH "${WIN_EXTRA_LIBS_PATH}" "${WIN_EXTRA_LIBS_PATH}/SDL")  # hints for FindSDL
endif()

if(CMAKE_HOST_APPLE)
  execute_process(COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE SDKROOT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  message("-- SDKROOT: ${SDKROOT}")
  set(CMAKE_OSX_SYSROOT ${SDKROOT})
  set(SYSROOT_ARG -isysroot ${SDKROOT})
endif()


# Python check
find_package(PythonInterp 3 REQUIRED)
if(PYTHONINTERP_FOUND)
  message(STATUS "Python found, version: ${PYTHON_VERSION_STRING}")
  get_filename_component(PYTHON_DIRECTORY ${PYTHON_EXECUTABLE} DIRECTORY)
else()
  message(WARNING "Python not found! Most firmware and simu flavors not buildable.")
  set(LUA NO)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")  # regular Clang or AppleClang
  set(CLANG TRUE)
endif()

option(DISABLE_COMPANION "Disable building companion and simulators" OFF)

if(NOT DISABLE_COMPANION)  # FIXME cosmetics/style

if(APPLE AND EXISTS /usr/local/opt/qt5)
	# Homebrew installs Qt5 (up to at least 5.9.1) in
	# /usr/local/qt5, ensure it can be found by CMake since
	# it is not in the default /usr/local prefix.
	list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/qt5")
endif()

find_package(Qt5Core)
find_package(Qt5Widgets)
find_package(Qt5Xml)
find_package(Qt5LinguistTools)
find_package(Qt5PrintSupport)
find_package(Qt5Multimedia)
find_package(Qt5Svg)

if(Qt5Core_FOUND)
  message(STATUS "Qt Version: ${Qt5Core_VERSION}")

  ### Get locations of Qt binary executables & libs (libs are for distros, not for linking)
  # first set up some hints
  get_target_property(QtCore_LOCATION Qt5::Core LOCATION)
  get_filename_component(qt_core_path ${QtCore_LOCATION} PATH)
  if(APPLE)
    get_filename_component(qt_core_path "${qt_core_path}/.." ABSOLUTE)
  endif()

  set(QT_LIB_DIR ${qt_core_path} CACHE PATH "Path to Qt libraries (.dll|.framework|.so).")
  find_path(QT_BIN_DIR NAMES "qmake" "qmake.exe" HINTS "${CMAKE_PREFIX_PATH}/bin" "${qt_core_path}/../bin" "${qt_core_path}" DOC "Path to Qt binaries (qmake, lupdate, etc.).")
  find_program(QT_QMAKE_EXECUTABLE qmake HINTS "${QT_BIN_DIR}" DOC "Location of qmake program.")
  find_program(QT_LUPDATE_EXECUTABLE lupdate HINTS "${QT_BIN_DIR}" DOC "Location of Qt's 'lupdate' program for updating translation files.")

  # Try to find Qt translations
  if(QT_QMAKE_EXECUTABLE)
    execute_process(
      COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_TRANSLATIONS
      OUTPUT_VARIABLE qt_translations_hint
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
  endif()
  find_path(QT_TRANSLATIONS_DIR NAMES "qt_en.qm" HINTS "${qt_translations_hint}" DOC "Path to prebuilt Qt translations (qt_*.qm).")

  ### Common definitions for the Qt-based apps
  list(APPEND APP_COMMON_DEFINES -DSIMU)
  list(APPEND APP_COMMON_DEFINES -DQXT_STATIC)
  list(APPEND APP_COMMON_DEFINES -DQT_USE_QSTRINGBUILDER)  # more efficient QString construction using % operator

  if(MSVC)
    list(APPEND APP_COMMON_DEFINES -D__STDC_LIMIT_MACROS)
  endif()
  if(APP_CUSTOM_DBG_HANDLER)
    # provide full qDebug log context to our custom handler. This may also affect libsimulator, which is why it is here
    list(APPEND APP_COMMON_DEFINES -DQT_MESSAGELOGCONTEXT)
    list(APPEND APP_COMMON_DEFINES -DAPP_DBG_HANDLER_ENABLE=1)
    if(APP_CUSTOM_DBG_LEVEL GREATER -1 AND APP_CUSTOM_DBG_LEVEL LESS 5)
      list(APPEND APP_COMMON_DEFINES -DAPP_DBG_HANDLER_DEFAULT_LEVEL=${APP_CUSTOM_DBG_LEVEL})
    endif()
    if(NOT APP_CUSTOM_DBG_SHOW_FILE STREQUAL "DEFAULT" AND (APP_CUSTOM_DBG_SHOW_FILE STREQUAL "1" OR APP_CUSTOM_DBG_SHOW_FILE STREQUAL "0"))
      list(APPEND APP_COMMON_DEFINES -DAPP_DBG_HANDLER_SHOW_SRC_PATH=${APP_CUSTOM_DBG_SHOW_FILE})
    endif()
  else()
    list(APPEND APP_COMMON_DEFINES -DAPP_DBG_HANDLER_ENABLE=0)
  endif()

else()
  message(WARNING "Qt not found! Companion and Simulator builds disabled.")
endif()

if(NOT WIN32)
  find_package(Fox QUIET)  # QUIET not working on WIN32?
  if (FOX_FOUND)
    message(STATUS "Foxlib found at ${FOX_LIBRARY}")
  else() # if(NOT WIN32)
    message("Libfox not found, simu target will not be available")
  endif()
endif()

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
endif(NOT DISABLE_COMPANION)

# Check for a file that is typically left from a OpenTX 2.1 build and abort if found
if (EXISTS ${RADIO_SRC_DIR}/stamp.h OR EXISTS ${RADIO_SRC_DIR}/translations/en.h)
   message(FATAL_ERROR "Source directory contains files leftover from a OpenTX 2.1 build. Please run `git clean -f` in source directory (Careful: Will remove any extra files) or do a new clean git checkout")
endif()

# Windows-specific includes and libs shared by sub-projects
if(WIN32)
  list(APPEND WIN_INCLUDE_DIRS "${RADIO_SRC_DIR}/thirdparty/windows/dirent")
  if(MSVC)
    list(APPEND WIN_INCLUDE_DIRS "${RADIO_SRC_DIR}/thirdparty/windows/msinttypes")
    #set(WIN_PTHREAD_BASE_PATH "${WIN_EXTRA_LIBS_PATH}/pthreads/Pre-built.2" CACHE PATH "Where to find pthreads include and lib folders.")
    #list(APPEND WIN_INCLUDE_DIRS "${WIN_PTHREAD_BASE_PATH}/include")
    #find_file(WIN_PTHREAD_LIB pthreadVC2.lib PATHS "${WIN_PTHREAD_BASE_PATH}" PATH_SUFFIXES lib/x86 lib NO_DEFAULT_PATH)
    #find_file(WIN_PTHREAD_DLL pthreadVC2.dll PATHS "${WIN_PTHREAD_BASE_PATH}" PATH_SUFFIXES lib/x86 lib dll/x86 dll NO_DEFAULT_PATH)
    #if(WIN_PTHREAD_LIB)
    #  list(APPEND WIN_LINK_LIBRARIES "${WIN_PTHREAD_LIB}")
    #else()
    #  message(SEND_ERROR "pthreadVC2.lib not found!")
    #endif()
    find_package(pthreads)
    #target_link_libraries(mytarget pthreads)
  else()
    set(CMAKE_C_USE_RESPONSE_FILE_FOR_INCLUDES OFF)
    set(CMAKE_C_USE_RESPONSE_FILE_FOR_LIBRARIES OFF)
    set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES OFF)
    set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_LIBRARIES OFF)
  endif()
endif()

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

add_subdirectory(${RADIO_SRC_DIR})

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
