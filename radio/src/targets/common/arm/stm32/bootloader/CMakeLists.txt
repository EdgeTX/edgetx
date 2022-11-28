
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

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
  ../../../../../${FATFS_DIR}/ff.c
  ../../../../../${FATFS_DIR}/ffunicode.c
  ../../../../../targets/${TARGET_DIR}/${LCD_DRIVER}
  ../../../../../targets/${TARGET_DIR}/backlight_driver.cpp
  ../../../../../targets/${TARGET_DIR}/keys_driver.cpp
  ../../../../../targets/${TARGET_DIR}/diskio.cpp
  ../../../../../targets/${TARGET_DIR}/board.cpp
  ../pwr_driver.cpp
  ../../../../../targets/${TARGET_DIR}/bootloader/boot_menu.cpp
  ../usbd_usr.cpp
  ../usbd_storage_msd.cpp
  ../delays_driver.cpp
  ../usbd_desc.c
  ../usb_bsp.c
  ../usb_driver.cpp
  ../flash_driver.cpp
  ../stm32_exti_driver.cpp
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

if(PCB STREQUAL X10 OR PCB STREQUAL X12S OR PCB STREQUAL NV14)
  set(BOOTLOADER_SRC
    ${BOOTLOADER_SRC}
    ../../../../../gui/colorlcd/fonts.cpp
    ../../../../../gui/colorlcd/colors.cpp
    ../../../../../fonts/lvgl/lv_font_roboto_bl_16.c
    ../../../../../thirdparty/libopenui/src/bitmapbuffer.cpp
    ../../../../../thirdparty/libopenui/thirdparty/lz4/lz4.c
    ../../../../../targets/${TARGET_DIR}/sdram_driver.c
    ../../../../../targets/common/arm/stm32/sdio_sd.c
    ../../../../../targets/common/arm/stm32/rtc_driver.cpp
    ../../../../../targets/${TARGET_DIR}/haptic_driver.cpp
    )

  # Add LVGL sources
  foreach(LVGL_FILE ${LVGL_SRC_FILES_MINIMAL})
    set(BOOTLOADER_SRC ${BOOTLOADER_SRC} ../../../../../${LVGL_FILE})
  endforeach()

  if(USB_CHARGER)
    set(BOOTLOADER_SRC
      ${BOOTLOADER_SRC}
      ../../../../../targets/${TARGET_DIR}/usb_charger_driver.cpp
      ../../../../../targets/${TARGET_DIR}/led_driver.cpp
      )
  endif()

  if(PCB STREQUAL NV14)
    set(BOOTLOADER_SRC
      ${BOOTLOADER_SRC}
      ../../../../../targets/${TARGET_DIR}/extmodule_helper.cpp
      )
  endif()

  if(DEBUG)
    set(BOOTLOADER_SRC
      ${BOOTLOADER_SRC}
      ../../../../../${STM32LIB_DIR}/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c
      ../../../../../serial.cpp
      ../../../../../targets/common/arm/stm32/aux_serial_driver.cpp
      ../../../../../targets/common/arm/stm32/stm32_usart_driver.cpp
      )
  endif()
else()
  set(BOOTLOADER_SRC
    ${BOOTLOADER_SRC}
    ../../../../../gui/common/stdlcd/fonts.cpp
    ../../../../../gui/common/stdlcd/utf8.cpp
    ../../../../../targets/${TARGET_DIR}/eeprom_driver.cpp
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
