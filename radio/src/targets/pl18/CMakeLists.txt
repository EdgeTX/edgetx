option(DISK_CACHE "Enable SD card disk cache" ON)
option(UNEXPECTED_SHUTDOWN "Enable the Unexpected Shutdown screen" ON)
option(STICKS_DEAD_ZONE "Enable sticks dead zone" YES)
option(PXX1 "PXX1 protocol support" ON)
option(PXX2 "PXX2 protocol support" OFF)
option(AFHDS3 "AFHDS3 TX Module" ON)
option(MULTIMODULE "DIY Multiprotocol TX Module (https://github.com/pascallanger/DIY-Multiprotocol-TX-Module)" ON)
option(GHOST "Ghost TX Module" ON)
option(MODULE_SIZE_STD "Standard size TX Module" ON)
option(USE_RTC_CLOCK "RTC Clock" ON)

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

# for size report script
set(CPU_TYPE_FULL STM32F429xI)
set(TARGET_LINKER_DIR stm32f429_sdram)
set(TARGET_EXTRAM_START 0xC0000000)

if(PCBREV STREQUAL PL18U)
  set(TARGET_SDRAM_SIZE 32768K)
  set(SIZE_TARGET_MEM_DEFINE "MEM_SIZE_SDRAM1=32768")
else()
  set(SIZE_TARGET_MEM_DEFINE "MEM_SIZE_SDRAM1=8192")
endif()

set(RF_BAUD_RATE 921600 230400 115200 57600 38400 19200 9600 4800 2400 1200)
set(PCB_RF_BAUD 921600 CACHE STRING "INTERNAL_MODULE_BAUDRATE: ${RF_BAUD_RATE}")
set_property(CACHE PCB_RF_BAUD PROPERTY STRINGS ${RF_BAUD_RATE})

add_definitions(-DPCBPL18 -DPCBFLYSKY)
add_definitions(-DBATTERY_CHARGE)
add_definitions(-DSTM32_SUPPORT_32BIT_TIMERS)

if(PCBREV STREQUAL PL18U)
  set(FLAVOUR pl18u)
  add_definitions(-DRADIO_PL18U)
  set(DISK_CACHE ON)
  set(WIRELESS_CHARGER YES)
  set(FLYSKY_GIMBAL ON)
  set(LED_STRIP ON)
  set(USE_VS1053B ON)
  set(KEY_DRIVER pl18u_key_driver.cpp)

  # Defines internal module for PL18U
  set(INTERNAL_MODULES AFHDS3;CRSF CACHE STRING "Internal modules")
  set(DEFAULT_INTERNAL_MODULE FLYSKY_AFHDS3 CACHE STRING "Default internal module")
elseif (PCBREV STREQUAL EL18)
    set(FLAVOUR el18)
    add_definitions(-DRADIO_EL18 -DRADIO_NV14_FAMILY -DPCBNV14 -DUSE_HATS_AS_KEYS)
    set(DISK_CACHE ON)
    set(FLYSKY_GIMBAL ON)
    set(USE_VS1053B ON)
    set(AFHDS3 ON)

    # defines existing internal modules
    set(INTERNAL_MODULES AFHDS3;CRSF CACHE STRING "Internal modules")
    set(DEFAULT_INTERNAL_MODULE FLYSKY_AFHDS3 CACHE STRING "Default internal module")
elseif(PCBREV STREQUAL NV14)
    set(FLAVOUR nv14)
    add_definitions(-DRADIO_NV14 -DRADIO_NV14_FAMILY -DPCBNV14)
    add_definitions(-DUSE_HATS_AS_KEYS -DAFHDS2_BAUDRATE=${PCB_RF_BAUD})
    set(DISK_CACHE ON)
    set(FLYSKY_GIMBAL ON)
    set(USE_VS1053B ON)
    set(AFHDS2 ON)
    set(AFHDS3 ON)

    # defines existing internal modules
    set(INTERNAL_MODULES AFHDS2A CACHE STRING "Internal modules")
    set(DEFAULT_INTERNAL_MODULE FLYSKY_AFHDS2A CACHE STRING "Default internal module")
elseif(PCBREV STREQUAL NB4P)
  set(FLAVOUR nb4p)
  add_definitions(-DRADIO_NB4P -DSPI_FLASH)
  set(ROTARY_ENCODER YES)
  set(LED_STRIP ON)
  set(AFHDS3 ON)
  set(KEY_DRIVER nb4p_key_driver.cpp)

  # Defines internal modules for PL18 via UART7
  set(INTERNAL_MODULES MULTI;AFHDS3;CRSF CACHE STRING "Internal modules")
  set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
elseif(PCBREV STREQUAL PL18EV)
  set(FLAVOUR pl18ev)
  add_definitions(-DRADIO_PL18EV -DSPI_FLASH -DUSE_HATS_AS_KEYS)
  set(WIRELESS_CHARGER YES)
  set(FLYSKY_GIMBAL ON)
  set(LED_STRIP ON)
  set(AFHDS3 ON)
  set(KEY_DRIVER key_driver.cpp)
else()
  set(FLAVOUR pl18)
  add_definitions(-DRADIO_PL18 -DSPI_FLASH -DUSE_HATS_AS_KEYS)
  set(WIRELESS_CHARGER YES)
  set(FLYSKY_GIMBAL ON)
  set(LED_STRIP ON)
  set(AFHDS3 ON)
  set(KEY_DRIVER key_driver.cpp)

  # Defines internal modules for PL18 via UART7
  set(INTERNAL_MODULES MULTI CACHE STRING "Internal modules")
  set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
endif()

set(BITMAPS_TARGET bm480_bitmaps)
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
  -DSDRAM -DCOLORLCD -DLIBOPENUI
  -DHARDWARE_TOUCH -DHARDWARE_KEYS
  -DSOFTWARE_KEYBOARD)

set(SDRAM ON)

add_definitions(-DAUDIO -DVOICE)
add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
add_definitions(-DCROSSFIRE_NATIVE)
add_definitions(-DHARDWARE_EXTERNAL_MODULE)

if(INTERNAL_GPS)
  add_definitions(-DGPS_USART_BAUDRATE=${INTERNAL_GPS_BAUDRATE})
endif()

if(USE_RTC_CLOCK)
  message("-- RTC Clock enabled")
  add_definitions(-DRTCLOCK)
endif()

if(WIRELESS_CHARGER)
  add_definitions(-DWIRELESS_CHARGER)
endif()

if(STICKS_DEAD_ZONE)
  add_definitions(-DSTICK_DEAD_ZONE)
endif()

if(NOT UNEXPECTED_SHUTDOWN)
  add_definitions(-DNO_UNEXPECTED_SHUTDOWN)
endif()

# VCP CLI
set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
set(CLI ON CACHE BOOL "Enable CLI")

set(TARGET_SRC_DIR targets/${TARGET_DIR})

set(BOARD_COMMON_SRC
  ${TARGET_SRC_DIR}/board.cpp
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
  targets/common/arm/stm32/spi_flash.cpp
  targets/common/arm/stm32/watchdog_driver.cpp
  drivers/frftl.cpp
)

if(ROTARY_ENCODER)
  set(BOARD_COMMON_SRC ${BOARD_COMMON_SRC}
    targets/common/arm/stm32/rotary_encoder_driver.cpp
  )
endif()

if(DISK_CACHE)
  set(BOARD_COMMON_SRC ${BOARD_COMMON_SRC}
    disk_cache.cpp
  )
  add_definitions(-DDISK_CACHE)
endif()

# Bootloader board library
add_library(board_bl OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  ${RADIO_SRC_DIR}/gui/colorlcd/boot_menu.cpp
  targets/common/arm/stm32/sdram_driver.cpp
)
add_dependencies(board_bl ${BITMAPS_TARGET})
set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:board_bl>)

# Firmware board library
add_library(board OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  ${TARGET_SRC_DIR}/${TOUCH_DRIVER}
  ${TARGET_SRC_DIR}/battery_driver.cpp
  targets/common/arm/stm32/delays_driver.cpp
  targets/common/arm/stm32/heartbeat_driver.cpp
  targets/common/arm/stm32/mixer_scheduler_driver.cpp
  targets/common/arm/stm32/module_timer_driver.cpp
  targets/common/arm/stm32/stm32_pulse_driver.cpp
  targets/common/arm/stm32/stm32_softserial_driver.cpp
  targets/common/arm/stm32/stm32_switch_driver.cpp
  targets/common/arm/stm32/trainer_driver.cpp
  targets/common/arm/stm32/spi_flash.cpp
  targets/common/arm/stm32/diskio_spi_flash.cpp
  drivers/frftl.cpp  
)
set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:board>)

if(PCBREV STREQUAL NB4P)
  target_sources(board PRIVATE ${TARGET_SRC_DIR}/nb4p_switch_driver.cpp)
endif()

if(USE_VS1053B)
  target_sources(board PRIVATE targets/common/arm/stm32/vs1053b.cpp)
else()
  target_sources(board PRIVATE targets/common/arm/stm32/audio_dac_driver.cpp)
  add_definitions(-DSOFTWARE_VOLUME)
endif()

if(LED_STRIP)
  target_sources(board PRIVATE ${TARGET_SRC_DIR}/led_driver.cpp)
  target_sources(board PRIVATE targets/common/arm/stm32/stm32_ws2812.cpp)
endif()


set(FIRMWARE_SRC
  ${FIRMWARE_SRC}
  bootloader/loadboot.cpp
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
