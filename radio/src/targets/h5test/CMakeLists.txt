option(SHUTDOWN_CONFIRMATION "Shutdown confirmation" OFF)
option(PXX1 "PXX1 protocol support" ON)
option(PXX2 "PXX2 protocol support" OFF)
option(AFHDS3 "AFHDS3 TX Module" OFF)
option(GHOST "Ghost TX Module" ON)
option(INTERNAL_MODULE_PPM "Support for PPM internal module" OFF)
option(AUTOUPDATE "Auto update internal chips from SD" OFF)
option(BIND_KEY "Enable bind button" OFF)
option(BLUETOOTH "FrSky BT module support" OFF)
option(RGBLEDS "WS2812 addressable LED support" OFF)

set(USE_RTC_CLOCK YES)
set(HARDWARE_EXTERNAL_MODULE YES)
set(TARGET_DIR h5test)

set(HSE_VALUE 8000000)
set(PPM_LIMITS_SYMETRICAL YES)

add_definitions(-DPCBTARANIS -DAUDIO -DVOICE)

set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
set(HAPTIC YES)
set(GUI_DIR 128x64)
set(BITMAPS_DIR 128x64)
set(BITMAPS_TARGET 9x_bitmaps)
set(FONTS_TARGET 9x_fonts_1bit)
set(STATUS_LEDS YES)
add_definitions(-DPCBX7 -DSOFTWARE_VOLUME)
add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
set(INTERNAL_MODULE_SERIAL YES)
set(LCD_DRIVER lcd_driver_newspi.cpp)
set(ROTARY_ENCODER YES)
set(MODULE_SIZE_SML YES)
set(USB_CHARGER YES)
set(FLAVOUR h5test)
set(USE_RTC_CLOCK NO)
set(CPU_TYPE_FULL STM32H562xx)
set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
add_definitions(-DRADIO_H5TEST)


set(CPU_TYPE STM32H5)
set(RTC_BACKUP_RAM YES)
add_definitions(-DSTM32H562xx -DSTM32H5)
set(TARGET_LINKER_DIR stm32h562)

if(ENABLE_SERIAL_PASSTHROUGH)
  set(CLI ON "Enable CLI")
endif()

set(INTERNAL_MODULES MULTI;CRSF CACHE STRING "Internal modules")
add_definitions(-DINTERNAL_MODULE_SERIAL)

set(BOARD generic_h5)

include_directories(boards/${BOARD})

set(TARGET_SRC_DIR targets/${TARGET_DIR})
set(BOARD_SRC_DIR boards/${BOARD})

set(BOARD_COMMON_SRC
  ${BOARD_SRC_DIR}/board.cpp
  ${BOARD_SRC_DIR}/system_clock.cpp
  ${BOARD_SRC_DIR}/led_driver.cpp
  ${BOARD_SRC_DIR}/backlight_driver.cpp
  ${BOARD_SRC_DIR}/haptic_driver.cpp
  ${BOARD_SRC_DIR}/usb_charger_driver.cpp
  ${BOARD_SRC_DIR}/${LCD_DRIVER}
  targets/common/arm/stm32/abnormal_reboot.cpp
  targets/common/arm/stm32/delays_driver.cpp
  targets/common/arm/stm32/flash_driver.cpp
  targets/common/arm/stm32/pwr_driver.cpp
  targets/common/arm/stm32/rtc_driver.cpp
  targets/common/arm/stm32/watchdog_driver.cpp
)

if(ROTARY_ENCODER)
  list(APPEND BOARD_COMMON_SRC
    targets/common/arm/stm32/rotary_encoder_driver.cpp
  )
endif()

if(BLUETOOTH)
  list(APPEND BOARD_COMMON_SRC
    targets/common/arm/stm32/bluetooth_driver.cppBOARD_SRC_DIR
  )
endif()

# Bootloader board library
add_library(board_bl OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  ${RADIO_SRC_DIR}/gui/common/stdlcd/boot_menu.cpp
)
# add_dependencies(board_bl ${BITMAPS_TARGET})
set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:board_bl>)

# Firmware board library
add_library(board OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  targets/common/arm/stm32/audio_dac_driver.cpp
  targets/common/arm/stm32/delays_driver.cpp
  targets/common/arm/stm32/heartbeat_driver.cpp
  targets/common/arm/stm32/mixer_scheduler_driver.cpp
  targets/common/arm/stm32/module_timer_driver.cpp
  targets/common/arm/stm32/stm32_pulse_driver.cpp
  targets/common/arm/stm32/stm32_softserial_driver.cpp
  targets/common/arm/stm32/stm32_switch_driver.cpp
  targets/common/arm/stm32/trainer_driver.cpp
#  targets/common/arm/stm32/stm32_ws2812.cpp
)
set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:board>)

if(USB_CHARGE_LED)
  add_definitions(-DUSB_CHARGE_LED)
endif()

if(USE_RTC_CLOCK)
  add_definitions(-DRTCLOCK)
endif()

set(SRC ${SRC}
  io/frsky_firmware_update.cpp
  io/multi_firmware_update.cpp
)

set(GUI_SRC ${GUI_SRC}
  model_input_edit.cpp
  model_mix_edit.cpp
  model_display.cpp
  radio_diagkeys.cpp
  radio_diaganas.cpp
  view_channels.cpp
  view_telemetry.cpp
  view_about.cpp
  bmp.cpp
)

set(FIRMWARE_SRC
  ${FIRMWARE_SRC}
  bootloader/loadboot.cpp
  )

set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} ${BITMAPS_TARGET})
