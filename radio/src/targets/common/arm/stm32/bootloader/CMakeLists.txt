
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
  ../../../../../${STM32USB_DIR}/STM32_USB_OTG_Driver/src/usb_core.c
  ../../../../../${STM32USB_DIR}/STM32_USB_OTG_Driver/src/usb_dcd.c
  ../../../../../${STM32USB_DIR}/STM32_USB_OTG_Driver/src/usb_dcd_int.c
  ../../../../../${STM32USB_DIR}/STM32_USB_Device_Library/Core/src/usbd_core.c
  ../../../../../${STM32USB_DIR}/STM32_USB_Device_Library/Core/src/usbd_ioreq.c
  ../../../../../${STM32USB_DIR}/STM32_USB_Device_Library/Core/src/usbd_req.c
  ../../../../../${STM32USB_DIR}/STM32_USB_Device_Library/Class/msc/src/usbd_msc_data.c
  ../../../../../${STM32USB_DIR}/STM32_USB_Device_Library/Class/msc/src/usbd_msc_scsi.c
  ../../../../../${STM32USB_DIR}/STM32_USB_Device_Library/Class/msc/src/usbd_msc_bot.c
  ../../../../../${STM32USB_DIR}/STM32_USB_Device_Library/Class/msc/src/usbd_msc_core.c
  ../../../../../targets/${TARGET_DIR}/${LCD_DRIVER}
  ../../../../../targets/${TARGET_DIR}/backlight_driver.cpp
  ../../../../../targets/${TARGET_DIR}/board.cpp
  ../../../../../targets/${TARGET_DIR}/bootloader/boot_menu.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/hal_keys.inc
  ../pwr_driver.cpp
  ../usbd_usr.cpp
  ../usbd_storage_msd.cpp
  ../delays_driver.cpp
  ../timers_driver.cpp
  ../usbd_desc.c
  ../usb_bsp.c
  ../usb_driver.cpp
  ../flash_driver.cpp
  ../stm32_exti_driver.cpp
  ../stm32_serial_driver.cpp
  ../stm32_usart_driver.cpp
  ../stm32_gpio_driver.cpp
  ../stm32_timer.cpp
  ../stm32_dma.cpp
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


if(PCB STREQUAL X10 OR PCB STREQUAL X12S OR PCB STREQUAL NV14)
  set(BOOTLOADER_SRC
    ${BOOTLOADER_SRC}
    ../../../../../gui/colorlcd/fonts.cpp
    ../../../../../gui/colorlcd/colors.cpp
    ../../../../../fonts/lvgl/lv_font_roboto_bl_16.c
    ../../../../../thirdparty/libopenui/src/bitmapbuffer.cpp
    ../../../../../thirdparty/libopenui/thirdparty/lz4/lz4.c
    ../../../../../targets/common/arm/stm32/diskio_sdio.cpp
    ../../../../../targets/common/arm/stm32/rtc_driver.cpp
    ../../../../../targets/common/arm/stm32/diskio_spi_flash.cpp
    ../../../../../targets/common/arm/stm32/spi_flash.cpp
    ../../../../../targets/common/arm/stm32/stm32_spi.cpp
    ../../../../../drivers/frftl.cpp
    ../../../../../crc.cpp
    ../../../../../targets/${TARGET_DIR}/sdram_driver.c
    ../../../../../targets/${TARGET_DIR}/haptic_driver.cpp
    )

  # Add LVGL sources
  foreach(LVGL_FILE ${LVGL_SRC_FILES_MINIMAL})
    set(BOOTLOADER_SRC ${BOOTLOADER_SRC} ../../../../../${LVGL_FILE})
  endforeach()

  if(NOT PCB STREQUAL NV14)
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
      ../../../../../targets/common/arm/stm32/stm32_serial_driver.cpp
      ../../../../../targets/common/arm/stm32/stm32_usart_driver.cpp
      ../../../../../targets/common/arm/stm32/stm32_dma.cpp
      )
  endif()
else()
  set(BOOTLOADER_SRC
    ${BOOTLOADER_SRC}
    ../../../../../gui/common/stdlcd/fonts.cpp
    ../../../../../gui/common/stdlcd/utf8.cpp
    ../../../../../targets/common/arm/stm32/stm32_gpio_driver.cpp
    ../../../../../targets/common/arm/stm32/stm32_spi.cpp
    ../../../../../targets/common/arm/stm32/sdcard_spi.cpp
    ../../../../../targets/common/arm/stm32/diskio_spi.cpp
    ../../../../../crc.cpp
    )
  remove_definitions(-DDEBUG)
endif()

remove_definitions(-DDISK_CACHE)
remove_definitions(-DLUA)
remove_definitions(-DCLI)
remove_definitions(-DSEMIHOSTING)
remove_definitions(-DUSB_SERIAL)
remove_definitions(-DWATCHDOG)

add_definitions(-DBOOT)

set(CMAKE_EXE_LINKER_FLAGS "-mcpu=${MCU} -mthumb -nostartfiles -lm -T${RADIO_SRC_DIR}/targets/${TARGET_DIR}/stm32_ramboot.ld -Wl,-Map=bootloader.map,--cref,--no-warn-mismatch,--gc-sections")
# use smaller nano libc
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs")

add_executable(bootloader EXCLUDE_FROM_ALL ${BOOTLOADER_SRC})
add_dependencies(bootloader ${BITMAPS_TARGET})

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
