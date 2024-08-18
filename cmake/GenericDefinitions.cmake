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
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_definitions(-D_GLIBCXX_USE_C99=1) # proper to_string definition

set(RADIO_DIRECTORY ${PROJECT_SOURCE_DIR}/radio)
set(RADIO_SRC_DIR ${RADIO_DIRECTORY}/src)
set(COMPANION_SRC_DIRECTORY ${PROJECT_SOURCE_DIR}/companion/src)
set(SIMU_SRC_DIRECTORY ${COMPANION_SRC_DIRECTORY}/simulation)
set(TOOLS_DIR ${PROJECT_SOURCE_DIR}/tools)
set(UTILS_DIR ${PROJECT_SOURCE_DIR}/radio/util)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

string(TIMESTAMP DATE "%Y-%m-%d")
string(TIMESTAMP TIME "%H:%M:%S")

include(Macros)
git_id(GIT_STR)

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

if(CMAKE_HOST_APPLE)
  execute_process(COMMAND xcrun --show-sdk-path
    OUTPUT_VARIABLE SDKROOT
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  message("-- SDKROOT: ${SDKROOT}")
  #set(CMAKE_OSX_SYSROOT ${SDKROOT})
  set(SYSROOT_ARG -isysroot ${SDKROOT})
endif()

# options shared by all targets

option(APP_CUSTOM_DBG_HANDLER "Use custom message handler for debug output in all Qt apps." ON)
set(APP_CUSTOM_DBG_LEVEL "0" CACHE STRING
  "Debug output level: 0=debug+; 1=info+; 2=warn+; 3=critical+; 4=fatal only.")
set(APP_CUSTOM_DBG_SHOW_FILE "DEFAULT" CACHE STRING
  "Show source file names in debug output. 1/0/DEFAULT (default is ON with DEBUG builds).")

option(VERBOSE_CMAKELISTS "Show extra information while processing CMakeLists.txt files." OFF)
option(WARNINGS_AS_ERRORS "Treat any compiler warning as an error (adds -Werror flag)." OFF)
