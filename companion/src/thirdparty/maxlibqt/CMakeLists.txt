cmake_minimum_required(VERSION 2.8.12)
project(maxLibQt)

set(CMAKE_COLOR_MAKEFILE ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(DOC_DIR "${PROJECT_SOURCE_DIR}/doc")

find_package(Qt5Core)
if(Qt5Core_FOUND)
	message(STATUS "Qt Version: ${Qt5Core_VERSION}")
else()
	message(FATAL_ERROR "Qt not found!")
endif()

## Subprojects
add_subdirectory("${SRC_DIR}")

## Doxymentation
add_subdirectory("${DOC_DIR}")
