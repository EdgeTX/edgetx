
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

AddHardwareDefTarget(${HW_DESC_JSON})
AddHWGenTarget(${HW_DESC_JSON} hal_keys hal_keys.inc)

set(BOOTLOADER_SRC
  ${BOOTLOADER_SRC}
  ../../../../../gui/${GUI_DIR}/lcd.cpp
  ../../../../../keys.cpp
  ../../../../../strhelpers.cpp
  ../../../../../stamp.cpp
  ../../../../../syscalls.c
  ../../../../../targets/${TARGET_DIR}/${LCD_DRIVER}
  ../../../../../targets/${TARGET_DIR}/backlight_driver.cpp
  ../../../../../targets/${TARGET_DIR}/board.cpp
  ../../../../../targets/${TARGET_DIR}/bootloader/boot_menu.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/hal_keys.inc
  ../pwr_driver.cpp
  ../delays_driver.cpp
  ../flash_driver.cpp
  ../watchdog_driver.cpp
  ../abnormal_reboot.cpp
  init.c
  boot.cpp
  bin_files.cpp
  )

if(ROTARY_ENCODER)
  set(BOOTLOADER_SRC
    ${BOOTLOADER_SRC}
    ../../../../../targets/common/arm/stm32/rotary_encoder_driver.cpp
    )
endif()

if(BLUETOOTH)
  set(BOOTLOADER_SRC
    ${BOOTLOADER_SRC}
    ../../../../../targets/common/arm/stm32/bluetooth_driver.cpp
    )
endif()

if(DEBUG_SEGGER_RTT)
  set(BOOTLOADER_SRC
  ${BOOTLOADER_SRC}
  ../../../../../thirdparty/Segger_RTT/RTT/SEGGER_RTT.c
  )
endif()

if(PCB STREQUAL X10 OR PCB STREQUAL X12S OR PCB STREQUAL NV14 OR PCB STREQUAL PL18)
  set(BOOTLOADER_SRC
    ${BOOTLOADER_SRC}
    ../../../../../gui/colorlcd/fonts.cpp
    ../../../../../gui/colorlcd/colors.cpp
    ../../../../../fonts/lvgl/lv_font_roboto_bl_16.c
    ../../../../../thirdparty/libopenui/src/bitmapbuffer.cpp
    ../../../../../thirdparty/libopenui/thirdparty/lz4/lz4.c
    ../../../../../targets/common/arm/stm32/dma2d.cpp
    ../../../../../targets/common/arm/stm32/rtc_driver.cpp
    ../../../../../targets/common/arm/stm32/diskio_spi_flash.cpp
    ../../../../../targets/common/arm/stm32/spi_flash.cpp
    ../../../../../drivers/frftl.cpp
    ../../../../../crc.cpp
    ../../../../../targets/${TARGET_DIR}/sdram_driver.c
    ../../../../../targets/${TARGET_DIR}/haptic_driver.cpp
    )

  if(PCB STREQUAL PL18)
    set(BOOTLOADER_SRC ${BOOTLOADER_SRC}
      ../../../../../targets/pl18/key_driver.cpp
    )    
  endif()
  
  # Add LVGL sources
  foreach(LVGL_FILE ${LVGL_SRC_FILES_MINIMAL})
    set(BOOTLOADER_SRC ${BOOTLOADER_SRC} ../../../../../${LVGL_FILE})
  endforeach()

  if(NOT (PCB STREQUAL NV14 OR PCB STREQUAL PL18))
    set(BOOTLOADER_SRC
      ${BOOTLOADER_SRC}
      ../../../../../targets/${TARGET_DIR}/led_driver.cpp
    )
  endif()

  
  if(USB_CHARGER)
    set(BOOTLOADER_SRC
      ${BOOTLOADER_SRC}
      ../../../../../targets/${TARGET_DIR}/usb_charger_driver.cpp
      )
  endif()

  if(DEBUG)
    set(BOOTLOADER_SRC
      ${BOOTLOADER_SRC}
      ../../../../../serial.cpp
      ../../../../../boards/generic_stm32/aux_ports.cpp
      )
  endif()
else()
  set(BOOTLOADER_SRC
    ${BOOTLOADER_SRC}
    ../../../../../gui/common/stdlcd/fonts.cpp
    ../../../../../gui/common/stdlcd/utf8.cpp
    ../../../../../crc.cpp
    )
  remove_definitions(-DDEBUG)
endif()

remove_definitions(-DDISK_CACHE)
remove_definitions(-DLUA)
remove_definitions(-DCLI)
remove_definitions(-DSEMIHOSTING)
remove_definitions(-DUSB_SERIAL)
remove_definitions(-DUSE_WATCHDOG)

add_definitions(-DBOOT)

if(NOT NO_LTO)
  message("-- Use LTO to compile bootloader")
  target_compile_options(stm32cube_ll PRIVATE -flto)
  target_compile_options(stm32usb PRIVATE -flto)
  target_compile_options(stm32usb_device_bl PRIVATE -flto)
  target_compile_options(stm32usb_device_fw PRIVATE -flto)
  target_compile_options(stm32_drivers PRIVATE -flto)
endif()
add_executable(bootloader EXCLUDE_FROM_ALL ${BOOTLOADER_SRC})
add_dependencies(bootloader ${BITMAPS_TARGET})

target_link_options(bootloader PRIVATE
  -lm -nostartfiles -T${LINKER_DIR}/bootloader.ld
  -Wl,-Map=bootloader.map,--cref,--no-warn-mismatch,--gc-sections
  --specs=nano.specs
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
