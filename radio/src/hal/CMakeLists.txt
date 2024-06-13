
AddHWGenTarget(${HW_DESC_JSON} hal_keys hal_keys.inc)
# add_library(hal_keys OBJECT EXCLUDE_FROM_ALL
#   ${CMAKE_CURRENT_BINARY_DIR}/hal_keys.inc
#   hal/key_driver.cpp
# )

set(SRC ${SRC}
  ${CMAKE_CURRENT_BINARY_DIR}/hal_keys.inc
  hal/key_driver.cpp
  hal/module_port.cpp
  hal/adc_driver.cpp
  hal/switch_driver.cpp
)

if(FUNCTION_SWITCHES)
  set(SRC ${SRC}
    hal/rgbleds.cpp
  )
endif()

if (FLEXSW)
  # define the number of allowed flexible adc entries used as switch
  add_definitions(-DMAX_FLEX_SWITCHES=${FLEXSW})
endif()

include_directories(${FATFS_DIR} ${FATFS_DIR}/option)

set(BOOTLOADER_SRC ${BOOTLOADER_SRC}
  ${CMAKE_CURRENT_SOURCE_DIR}/hal/key_driver.cpp
)

if (NOT NATIVE_BUILD)
  set(SRC ${SRC}
    hal/storage.cpp
    hal/fatfs_diskio.cpp
    ${FATFS_DIR}/ff.c
    ${FATFS_DIR}/ffunicode.c
  )
  set(BOOTLOADER_SRC ${BOOTLOADER_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/hal/storage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hal/fatfs_diskio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/${FATFS_DIR}/ff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/${FATFS_DIR}/ffunicode.c
  )
endif()
