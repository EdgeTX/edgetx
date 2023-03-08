
project(maxLibQt)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(DOC_DIR "${PROJECT_SOURCE_DIR}/doc")

## Subprojects
add_subdirectory("${SRC_DIR}")

## Doxymentation
add_subdirectory("${DOC_DIR}")
