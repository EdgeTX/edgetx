cmake_minimum_required(VERSION 3.1.0)

project(ImageViewer)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Widgets)
find_package(Qt5Svg)

set(PWD "${CMAKE_CURRENT_LIST_DIR}")
set(SRC_PATH "../../src")

include_directories(${SRC_PATH}/widgets)
#add_subdirectory(${MLQT_PATH}/widgets)

set(HDRS
  "${PWD}/GraphicsImageView.h"
  "${PWD}/ImageGrid.h"
  "${PWD}/ImageViewer.h"
  "${SRC_PATH}/widgets/OverlayStackLayout.h"
)

set(SRCS
  "${PWD}/main.cpp"
  "${PWD}/GraphicsImageView.cpp"
  "${PWD}/ImageGrid.cpp"
  "${PWD}/ImageViewer.cpp"
  "${SRC_PATH}/widgets/OverlayStackLayout.cpp"
)

set(RSRCS "${PWD}/imageviewer.qrc")

add_executable(${PROJECT_NAME} ${HDRS} ${SRCS} ${RSRCS})

target_compile_definitions(${PROJECT_NAME} PRIVATE QT_USE_QSTRINGBUILDER)
target_compile_definitions(${PROJECT_NAME} PRIVATE SOURCE_DIR="${PWD}")
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg)
