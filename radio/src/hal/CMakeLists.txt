
set(SRC ${SRC}
  hal/key_driver.cpp
  hal/module_port.cpp
  hal/adc_driver.cpp
  hal/switch_driver.cpp
  hal/usb_driver.cpp
)

if(FUNCTION_SWITCHES OR RGB_LEDS)
  set(SRC ${SRC}
    hal/rgbleds.cpp
  )
endif()

if (FLEXSW)
  # define the number of allowed flexible adc entries used as switch
  add_definitions(-DMAX_FLEX_SWITCHES=${FLEXSW})
endif()

include_directories(${FATFS_DIR} ${FATFS_DIR}/option)

if(EXFAT)
  add_definitions(-DFF_FS_EXFAT=1)
endif()

set(BOOTLOADER_SRC ${BOOTLOADER_SRC}
  ${CMAKE_CURRENT_SOURCE_DIR}/hal/key_driver.cpp
)

if ((NOT NATIVE_BUILD) OR SIMU_DISKIO)
  set(SRC ${SRC}
    hal/storage.cpp
    hal/fatfs_diskio.cpp
    hal/flash_driver.cpp
    ${FATFS_DIR}/ff.c
    ${FATFS_DIR}/ffunicode.c
  )
  set(BOOTLOADER_SRC ${BOOTLOADER_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/hal/storage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hal/fatfs_diskio.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/hal/flash_driver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/${FATFS_DIR}/ff.c
    ${CMAKE_CURRENT_SOURCE_DIR}/${FATFS_DIR}/ffunicode.c
  )
endif()
