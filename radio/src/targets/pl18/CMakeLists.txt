option(UNEXPECTED_SHUTDOWN "Enable the Unexpected Shutdown screen" ON)
option(PXX1 "PXX1 protocol support" ON)
option(PXX2 "PXX2 protocol support" OFF)
option(AFHDS3 "AFHDS3 TX Module" ON)
option(MULTIMODULE "DIY Multiprotocol TX Module (https://github.com/pascallanger/DIY-Multiprotocol-TX-Module)" ON)
option(GHOST "Ghost TX Module" ON)
option(MODULE_SIZE_STD "Standard size TX Module" ON)

set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
set(CPU_TYPE STM32F4)
set(HSE_VALUE 12000000)
set(STORAGE_MODELSLIST YES)
set(HAPTIC YES)
set(GUI_DIR colorlcd)
set(BITMAPS_DIR 480x272)
set(TARGET_DIR pl18)
set(RTC_BACKUP_RAM YES)
set(PPM_LIMITS_SYMETRICAL YES)
set(USB_SERIAL ON CACHE BOOL "Enable USB serial (CDC)")
set(HARDWARE_EXTERNAL_MODULE YES)

#option(STICKS_DEAD_ZONE "Enable sticks dead zone" YES)
#option(AFHDS2 "Support for AFHDS2" OFF)

# for size report script
set(CPU_TYPE_FULL STM32F429xI)
set(TARGET_LINKER_DIR stm32f429_sdram)
set(TARGET_SDRAM_START 0xC0000000)

set(SIZE_TARGET_MEM_DEFINE "MEM_SIZE_SDRAM1=8192")

#set(RF_BAUD_RATE 921600 230400 115200 57600 38400 19200 9600 4800 2400 1200)
#set(PCB_RF_BAUD 921600 CACHE STRING "INTERNAL_MODULE_BAUDRATE: ${RF_BAUD_RATE}")
#set_property(CACHE PCB_RF_BAUD PROPERTY STRINGS ${RF_BAUD_RATE})

add_definitions(-DPCBPL18 -DPCBFLYSKY)
add_definitions(-DBATTERY_CHARGE)
add_definitions(-DSOFTWARE_VOLUME)
add_definitions(-DSPI_FLASH)
add_definitions(-DSTM32_SUPPORT_32BIT_TIMERS)

if(PCBREV STREQUAL NB4P)
  set(FLAVOUR nb4p)
  add_definitions(-DRADIO_NB4P)
  set(ROTARY_ENCODER YES)
  set(KEY_DRIVER nb4p_key_driver.cpp)
elseif(PCBREV STREQUAL PL18EV)
  set(FLAVOUR pl18ev)
  add_definitions(-DRADIO_PL18EV -DUSE_HATS_AS_KEYS)
  set(WIRELESS_CHARGER YES)
  set(FLYSKY_GIMBAL ON)
  set(KEY_DRIVER key_driver.cpp)
else()
  set(FLAVOUR pl18)
  add_definitions(-DRADIO_PL18 -DUSE_HATS_AS_KEYS)
  set(WIRELESS_CHARGER YES)
  set(FLYSKY_GIMBAL ON)
  set(KEY_DRIVER key_driver.cpp)

  # Defines internal modules for PL18 via UART7
  set(INTERNAL_MODULES MULTI CACHE STRING "Internal modules")
  set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
endif()

set(BITMAPS_TARGET pl18_bitmaps)
set(FONTS_TARGET x12_fonts)
set(LCD_DRIVER lcd_driver.cpp)
set(TOUCH_DRIVER touch_driver.cpp)
set(HARDWARE_TOUCH YES)
set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} ${BITMAPS_TARGET})
set(FIRMWARE_DEPENDENCIES datacopy)

set(HARDWARE_TOUCH ON)
set(SOFTWARE_KEYBOARD ON)

add_definitions(
  -DSTM32F429_439xx -DSTM32F429xx
  -DSDRAM -DCCMRAM -DCOLORLCD -DLIBOPENUI
  -DHARDWARE_TOUCH -DHARDWARE_KEYS
  -DSOFTWARE_KEYBOARD)

set(SDRAM ON)

add_definitions(-DAUDIO -DVOICE -DRTCLOCK)
add_definitions(-DGPS_USART_BAUDRATE=${INTERNAL_GPS_BAUDRATE})
add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
add_definitions(-DCROSSFIRE_NATIVE)
add_definitions(-DHARDWARE_EXTERNAL_MODULE)

if(WIRELESS_CHARGER)
  add_definitions(-DWIRELESS_CHARGER)
endif()

if(STICKS_DEAD_ZONE)
  add_definitions(-DSTICK_DEAD_ZONE)
endif()

if(NOT UNEXPECTED_SHUTDOWN)
  add_definitions(-DNO_UNEXPECTED_SHUTDOWN)
endif()

set(AFHDS3 ON)

# VCP CLI
set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
set(CLI ON CACHE BOOL "Enable CLI")

set(TARGET_SRC_DIR targets/${TARGET_DIR})

set(BOARD_COMMON_SRC
  ${TARGET_SRC_DIR}/board.cpp
  ${TARGET_SRC_DIR}/led_driver.cpp
  ${TARGET_SRC_DIR}/${KEY_DRIVER}
  ${TARGET_SRC_DIR}/haptic_driver.cpp
  ${TARGET_SRC_DIR}/backlight_driver.cpp
  ${TARGET_SRC_DIR}/${LCD_DRIVER}
  targets/common/arm/stm32/abnormal_reboot.cpp
  targets/common/arm/stm32/delays_driver.cpp
  targets/common/arm/stm32/diskio_spi_flash.cpp
  targets/common/arm/stm32/dma2d.cpp
  targets/common/arm/stm32/flash_driver.cpp
  targets/common/arm/stm32/pwr_driver.cpp
  targets/common/arm/stm32/rtc_driver.cpp
  targets/common/arm/stm32/sdram_driver.cpp
  targets/common/arm/stm32/spi_flash.cpp
  targets/common/arm/stm32/watchdog_driver.cpp
  drivers/frftl.cpp
)

if(ROTARY_ENCODER)
  list(APPEND BOARD_COMMON_SRC
    targets/common/arm/stm32/rotary_encoder_driver.cpp
  )
endif()

# Bootloader board library
add_library(board_bl OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  ${TARGET_SRC_DIR}/bootloader/boot_menu.cpp
)
add_dependencies(board_bl ${BITMAPS_TARGET})
set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:board_bl>)

# Firmware board library
add_library(board OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  ${TARGET_SRC_DIR}/${TOUCH_DRIVER}
  ${TARGET_SRC_DIR}/battery_driver.cpp
  targets/common/arm/stm32/audio_dac_driver.cpp
  targets/common/arm/stm32/delays_driver.cpp
  targets/common/arm/stm32/heartbeat_driver.cpp
  targets/common/arm/stm32/mixer_scheduler_driver.cpp
  targets/common/arm/stm32/module_timer_driver.cpp
  targets/common/arm/stm32/stm32_pulse_driver.cpp
  targets/common/arm/stm32/stm32_softserial_driver.cpp
  targets/common/arm/stm32/stm32_switch_driver.cpp
  targets/common/arm/stm32/stm32_ws2812.cpp
  targets/common/arm/stm32/trainer_driver.cpp
  targets/common/arm/stm32/spi_flash.cpp
  targets/common/arm/stm32/diskio_spi_flash.cpp
  drivers/frftl.cpp
)
set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:board>)

set(FIRMWARE_SRC
  ${FIRMWARE_SRC}
  targets/common/arm/loadboot.cpp
)

set(SRC ${SRC}
  io/frsky_firmware_update.cpp
  io/multi_firmware_update.cpp
  io/multi_protolist.cpp
)

if (MULTIMODULE)
  add_definitions(-DMULTI_PROTOLIST)
endif()

# Make malloc() thread-safe
add_definitions(-DTHREADSAFE_MALLOC)
