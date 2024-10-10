
project(generaledit)

set(${PROJECT_NAME}_NAMES
  calibration
  generaledit
  generaloptions
  generalsetup
  hardware
  timezoneedit
  trainer
)

AddHeadersSources()

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_HDRS}
  ${${PROJECT_NAME}_SRCS}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${CPN_COMMON_LIB}
    Qt5::Multimedia
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${maxLibQt_SOURCE_DIR}/widgets"
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
)
