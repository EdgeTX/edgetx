
project(flash)

set(${PROJECT_NAME}_NAMES
  customizesplashdialog
  flashfirmwaredialog
  process_copy
  process_sync
  radiointerface
  splashlibrarydialog
)

AddHeadersSources()

# these have no cpp
set(${PROJECT_NAME}_HDRS
  ${${PROJECT_NAME}_HDRS}
  splashlabel.h
)

qt_wrap_cpp(${PROJECT_NAME}_SRCS
)

add_library(${PROJECT_NAME}
  ${${PROJECT_NAME}_HDRS}
  ${${PROJECT_NAME}_SRCS}
)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    ${CPN_COMMON_LIB}
    ${RS_DFU_LIBRARIES}
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
  PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
    "${RS_DFU_INCLUDE_DIRS}"
)
