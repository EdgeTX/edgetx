
project(updates)

set(${PROJECT_NAME}_NAMES
  chooserdialog
  repomodels
  updatecompanion
  updatefirmware
  updateinterface
  updatemultiprotocol
  updateoptionsdialog
  updates
  updatesdcard
  updatesdialog
  updatesounds
  updatethemes
)

AddHeadersSources()

# AUTOMOC does not detect so manually process
set(${PROJECT_NAME}_MOC
  chooserdialog.h
  repomodels.h
  updateinterface.h
  updateoptionsdialog.h
  updates.h
  updatesdialog.h
)

qt5_wrap_cpp(${PROJECT_NAME}_SRCS
  ${${PROJECT_NAME}_MOC}
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
