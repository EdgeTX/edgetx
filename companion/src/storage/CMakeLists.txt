include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(storage_NAMES
  hexinterface
  mountlist
  rlefile
  appdata
  firmwareinterface
  storage
  bineeprom
  eepe
  hexeeprom
  categorized
  sdcard
  etx
  otx
  yaml
)

set(storage_SRCS
)

set(storage_HDRS
)

set(storage_MOC_HDRS
  appdata.h
)

foreach(name ${storage_NAMES})
  set(storage_SRCS ${storage_SRCS} ${name}.cpp)
  set(storage_HDRS ${storage_HDRS} ${name}.h)
endforeach()

qt5_wrap_cpp(storage_SRCS ${storage_MOC_HDRS})

add_library(storage ${storage_SRCS} ${storage_HDRS})
target_link_libraries(storage PRIVATE ${CPN_COMMON_LIB})
