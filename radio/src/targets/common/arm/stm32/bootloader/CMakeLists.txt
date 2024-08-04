
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

AddHardwareDefTarget(${HW_DESC_JSON})
AddHWGenTarget(${HW_DESC_JSON} hal_keys hal_keys.inc)

set(BOOTLOADER_SRC ${BOOTLOADER_SRC}
  ${CMAKE_CURRENT_BINARY_DIR}/hal_keys.inc
  ${RADIO_SRC_DIR}/keys.cpp
  ${RADIO_SRC_DIR}/strhelpers.cpp
  ${RADIO_SRC_DIR}/stamp.cpp
  ${RADIO_SRC_DIR}/syscalls.c
  ${RADIO_SRC_DIR}/crc.cpp
  boot.cpp
  bin_files.cpp
)

if(DEBUG_SEGGER_RTT)
  set(BOOTLOADER_SRC ${BOOTLOADER_SRC}
    ${RADIO_SRC_DIR}/thirdparty/Segger_RTT/RTT/SEGGER_RTT.c
  )
endif()

# TODO: this shouldn't be needed at all
remove_definitions(-DDISK_CACHE)
remove_definitions(-DLUA)
remove_definitions(-DCLI)
remove_definitions(-DSEMIHOSTING)
remove_definitions(-DUSB_SERIAL)

if(NOT NO_LTO)
  message("-- Use LTO to compile bootloader")
  target_compile_options(stm32cube_ll PRIVATE -flto)
  target_compile_options(stm32usb PRIVATE -flto)
  target_compile_options(stm32usb_device_bl PRIVATE -flto)
  target_compile_options(stm32usb_device_fw PRIVATE -flto)
  target_compile_options(stm32_drivers PRIVATE -flto)
  target_compile_options(stm32_drivers_w_dbg_fw PRIVATE -flto)
  target_compile_options(stm32_drivers_w_dbg_bl PRIVATE -flto)
endif()

add_executable(bootloader EXCLUDE_FROM_ALL ${BOOTLOADER_SRC})
add_dependencies(bootloader ${BITMAPS_TARGET})

target_compile_definitions(bootloader PRIVATE BOOT)
target_compile_definitions(board_bl PRIVATE BOOT)

target_link_options(bootloader BEFORE PRIVATE
  -L${LINKER_DIR}/${TARGET_LINKER_DIR}/bootloader
  -T${LINKER_DIR}/bootloader.ld
  -Wl,-Map=bootloader.map,--cref,--no-warn-mismatch,--gc-sections
  -lm --specs=nano.specs
)

if (DEFINED TARGET_SDRAM_START)
  # message("TARGET_SDRAM_START = ${TARGET_SDRAM_START}")
  target_link_options(bootloader PRIVATE
    -Wl,--defsym=__sdram_start=${TARGET_SDRAM_START}
  )
endif()

if (DEFINED TARGET_SDRAM_SIZE)
  # message("TARGET_SDRAM_SIZE = ${TARGET_SDRAM_SIZE}")
  target_link_options(bootloader PRIVATE
    -Wl,--defsym=__sdram_size=${TARGET_SDRAM_SIZE}
  )
endif()

if (ENABLE_BOOTLOADER_DEBUG)
  target_compile_definitions(board_bl PRIVATE -DDEBUG)
  target_compile_definitions(bootloader PRIVATE -DDEBUG)
  target_sources(bootloader PRIVATE
    ${RADIO_SRC_DIR}/serial.cpp
  )
  # uncomment to enable TRACE in drivers
  # target_compile_definitions(stm32_drivers_w_dbg_bl PRIVATE -DDEBUG)
endif()

target_include_directories(bootloader
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(
  TARGET bootloader POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary bootloader.elf ${CMAKE_BINARY_DIR}/bootloader.bin
)

if(PYTHONINTERP_FOUND)
  add_custom_command(
    TARGET bootloader POST_BUILD
    COMMAND ${PYTHON_EXECUTABLE} ${RADIO_DIRECTORY}/util/bin2lbm.py ${CMAKE_BINARY_DIR}/bootloader.bin bootloader.lbm
  )
endif()

PrintTargetReport("bootloader")
