option(UNEXPECTED_SHUTDOWN "Enable the Unexpected Shutdown screen" ON)
option(PXX1 "PXX1 protocol support" ON)
option(PXX2 "PXX2 protocol support" OFF)
option(AFHDS3 "AFHDS3 TX Module" ON)
option(MULTIMODULE "DIY Multiprotocol TX Module (https://github.com/pascallanger/DIY-Multiprotocol-TX-Module)" ON)
option(GHOST "Ghost TX Module" ON)
option(MODULE_SIZE_STD "Standard size TX Module" ON)
option(LUA_MIXER "Enable LUA mixer/model scripts support" ON)
option(DISK_CACHE "Enable SD card disk cache" ON)
option(INTERNAL_GPS "Support for internal GPS" ON)

set(FIRMWARE_QSPI YES)
set(FIRMWARE_FORMAT_UF2 YES)
set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
set(CPU_TYPE STM32H7)
set(HSE_VALUE 48000000)
set(SDCARD YES)
set(STORAGE_MODELSLIST YES)
set(HAPTIC YES)
set(GUI_DIR colorlcd)
set(BITMAPS_DIR 800x480)
set(TARGET_DIR tx16smk3)
set(RTC_BACKUP_RAM YES)
set(PPM_LIMITS_SYMETRICAL YES)
set(USB_SERIAL ON CACHE BOOL "Enable USB serial (CDC)")
set(HARDWARE_EXTERNAL_MODULE YES)
set(INTERNAL_MODULE_AFHDS3 NO)
set(ROTARY_ENCODER YES)
set(FUNCTION_SWITCHES_WITH_RGB YES)
set(FLEXSW "2" CACHE STRING "Max flex inputs usable as switches")
set(INTERNAL_GPS_BAUDRATE "9600" CACHE STRING "Baud rate for internal GPS")

#option(STICKS_DEAD_ZONE "Enable sticks dead zone" YES)

# IMU support
set(IMU ON)
add_definitions(-DIMU_ICM4207C)

# for size report script
set(CPU_TYPE_FULL STM32H750xB)
set(TARGET_LINKER_DIR stm32h750_sdram)
set(TARGET_EXTRAM_START 0xD0000000)
set(TARGET_EXTRAM_SIZE 32M)
set(SIZE_TARGET_MEM_DEFINE "MEM_SIZE_SDRAM1=32768")
set(FIRMWARE_STRIP_BOOTLOADER YES)

add_definitions(-DPCBTX16SMK3 -DPCBHORUS)
add_definitions(-DBATTERY_CHARGE)
add_definitions(-DSTM32_SUPPORT_32BIT_TIMERS)
add_definitions(-DFIRMWARE_QSPI)
add_definitions(-DFIRMWARE_FORMAT_UF2)
add_definitions(-DUSE_CUSTOM_EXTI_IRQ)
add_definitions(-DUSB_CHARGER)
add_definitions(-DLUMINOSITY_SENSOR)

set(FLAVOUR tx16smk3)
add_definitions(-DRADIO_TX16SMK3)
add_definitions(-DMANUFACTURER_RADIOMASTER)

# defines existing internal modules
set(INTERNAL_MODULES CRSF CACHE STRING "Internal modules")
set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")

set(BITMAPS_TARGET bm800_bitmaps)
set(FONTS_TARGET x12_fonts)
set(LCD_DRIVER lcd_driver.cpp)
set(HARDWARE_TOUCH YES)
set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} ${BITMAPS_TARGET})
set(FIRMWARE_DEPENDENCIES datacopy)
set(SOFTWARE_KEYBOARD ON)
set(FUNCTION_SWITCHES ON)
set(USB_CHARGER YES)
set(AUX_SERIAL YES)
set(AUX2_SERIAL YES)

add_definitions(
  -DSTM32H750XBTx -DSTM32H750xx
  -DSDRAM -DCCMRAM
  -DCOLORLCD -DLIBOPENUI
  -DHARDWARE_TOUCH -DHARDWARE_KEYS -DSOFTWARE_KEYBOARD
  -DWS2812_MAX_LEDS=26
)

set(SDRAM ON)

add_definitions(-DAUDIO -DVOICE -DRTCLOCK)
add_definitions(-DGPS_USART_BAUDRATE=${INTERNAL_GPS_BAUDRATE})
add_definitions(-DPWR_BUTTON_${PWR_BUTTON})

if(STICKS_DEAD_ZONE)
  add_definitions(-DSTICK_DEAD_ZONE)
endif()

if(NOT UNEXPECTED_SHUTDOWN)
  add_definitions(-DNO_UNEXPECTED_SHUTDOWN)
endif()

if(DISK_CACHE)
  set(SRC ${SRC} disk_cache.cpp)
  add_definitions(-DDISK_CACHE)
endif()

if(FUNCTION_SWITCHES_WITH_RGB)
  set(FUNCTION_SWITCHES YES)
  add_definitions(-DFUNCTION_SWITCHES)
  add_definitions(-DFUNCTION_SWITCHES_RGB_LEDS)
endif()

if(FUNCTION_SWITCHES)
add_definitions(-DFUNCTION_SWITCHES)
endif()

if(INTERNAL_GPS)
  message("-- Internal GPS enabled")
  add_definitions(-DINTERNAL_GPS)
  add_definitions(-DGPS_USART_BAUDRATE=${INTERNAL_GPS_BAUDRATE})
  set(SRC ${SRC} gps.cpp gps_nmea.cpp gps_ubx.cpp)
endif()

set(AFHDS3 ON)

# VCP CLI
set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
set(CLI ON CACHE BOOL "Enable CLI")

set(BOARD rm-h750)

include_directories(boards/${BOARD})

set(TARGET_SRC_DIR targets/${TARGET_DIR})
set(BOARD_SRC_DIR boards/${BOARD})

set(BOARD_COMMON_SRC
  ${BOARD_SRC_DIR}/board.cpp
  ${BOARD_SRC_DIR}/bsp_io.cpp
  ${TARGET_SRC_DIR}/key_driver.cpp
  ${BOARD_SRC_DIR}/lcd_driver_800.cpp
  ${BOARD_SRC_DIR}/backlight_driver.cpp
  ${BOARD_SRC_DIR}/haptic_driver.cpp
  ${BOARD_SRC_DIR}/extflash_driver.cpp
  ${BOARD_SRC_DIR}/usb_charger_driver.cpp
  targets/common/arm/stm32/abnormal_reboot.cpp
  targets/common/arm/stm32/delays_driver.cpp
  targets/common/arm/stm32/dma2d.cpp
  targets/common/arm/stm32/flash_driver.cpp
  targets/common/arm/stm32/pwr_driver.cpp
  targets/common/arm/stm32/rotary_encoder_driver.cpp
  targets/common/arm/stm32/rtc_driver.cpp
  targets/common/arm/stm32/watchdog_driver.cpp
  drivers/pca95xx.cpp
)

# Bootloader board library
add_library(board_bl OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  ${BOARD_SRC_DIR}/sdram_driver.cpp
  ${RADIO_SRC_DIR}/gui/colorlcd/boot_menu.cpp
  targets/common/arm/stm32/stm32_qspi.cpp
  targets/common/arm/stm32/stm32_pulse_driver.cpp
)
add_dependencies(board_bl minimal_board_lib)
set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:board_bl>)

set(CMSIS_SRC ${BOARD_SRC_DIR}/system_clock.c)

# Firmware board library
add_library(board OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  ${BOARD_SRC_DIR}/tp_gt911.cpp
  ${BOARD_SRC_DIR}/audio_driver.cpp
  ${BOARD_SRC_DIR}/luminosity_sensor.cpp
  drivers/icm42607C.cpp
  drivers/imu_filter.cpp
  drivers/tas2505.cpp
  targets/common/arm/stm32/heartbeat_driver.cpp
  targets/common/arm/stm32/mixer_scheduler_driver.cpp
  targets/common/arm/stm32/module_timer_driver.cpp
  targets/common/arm/stm32/stm32_pulse_driver.cpp
  targets/common/arm/stm32/stm32_softserial_driver.cpp
  targets/common/arm/stm32/stm32_qspi.cpp
  targets/common/arm/stm32/stm32_i2s.cpp
  targets/common/arm/stm32/stm32_switch_driver.cpp
  targets/common/arm/stm32/stm32_ws2812.cpp
  targets/common/arm/stm32/trainer_driver.cpp
)
add_dependencies(board board_lib)
set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:board>)

include_directories(${RADIO_SRC_DIR}/fonts/colorlcd gui/${GUI_DIR} gui/${GUI_DIR}/layouts)

if(BOOTLOADER)
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    ../../bootloader/loadboot.cpp
    )
endif()

set(SRC
  ${SRC}
  io/frsky_firmware_update.cpp
  io/multi_firmware_update.cpp
  )

if (MULTIMODULE)
  add_definitions(-DMULTI_PROTOLIST)
  set(SRC ${SRC}
    io/multi_protolist.cpp
    )
endif()

# Make malloc() thread-safe
add_definitions(-DTHREADSAFE_MALLOC)
