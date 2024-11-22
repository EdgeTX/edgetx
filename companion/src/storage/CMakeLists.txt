
project(storage)

set(${PROJECT_NAME}_NAMES
  hexinterface
  mountlist
  appdata
  firmwareinterface
  storage
  eepe
  hexeeprom
  labeled
  sdcard
  etx
  yaml
  crc
  minizinterface
)

AddHeadersSources()

# AUTOMOC does not detect so manually process
set(${PROJECT_NAME}_MOC
  appdata.h
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
    miniz
)

target_include_directories(${PROJECT_NAME}
  PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
)
