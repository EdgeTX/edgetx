
project(progress)

set(${PROJECT_NAME}_NAMES
  progressdialog
  progresswidget
)

AddHeadersSources()

qt_wrap_cpp(${PROJECT_NAME}_SRCS
)

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_HDRS}
  ${${PROJECT_NAME}_SRCS}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${CPN_COMMON_LIB}
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
)
