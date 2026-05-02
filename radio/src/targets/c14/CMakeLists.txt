option(UNEXPECTED_SHUTDOWN "Enable the Unexpected Shutdown screen" ON)
option(PXX1 "PXX1 protocol support" ON)
option(PXX2 "PXX2 protocol support" OFF)
option(AFHDS3 "AFHDS3 TX Module" ON)
option(MULTIMODULE "DIY Multiprotocol TX Module (https://github.com/pascallanger/DIY-Multiprotocol-TX-Module)" ON)
option(GHOST "Ghost TX Module" ON)
option(LUA_MIXER "Enable LUA mixer/model scripts support" ON)
option(DISK_CACHE "Enable SD card disk cache" ON)
option(STICK_DEAD_ZONE "Enable sticks dead zone" YES)

# defines existing internal modules
set(INTERNAL_MODULES CRSF CACHE STRING "Internal modules")
set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")

set(FIRMWARE_QSPI YES)
set(FIRMWARE_FORMAT_UF2 YES)
set(PWR_BUTTON "MANAGED" CACHE STRING "Pwr button type (PRESS/SWITCH/MANAGED)")
set(CPU_TYPE STM32H7)
set(HSE_VALUE 8000000)
set(SDCARD YES)
set(STORAGE_MODELSLIST YES)
set(HAPTIC YES)
set(GUI_DIR colorlcd)
set(BITMAPS_DIR 480x272)
set(TARGET_DIR c14)
set(RTC_BACKUP_RAM YES)
set(PPM_LIMITS_SYMETRICAL YES)
set(USB_SERIAL ON CACHE BOOL "Enable USB serial (CDC)")
set(HARDWARE_EXTERNAL_MODULE YES)
set(FUNCTION_SWITCHES NO)
set(RGB_LEDS YES)
set(ROTARY_ENCODER YES)
set(MODULE_SIZE_SML YES)

# IMU support
set(IMU ON)
add_definitions(-DIMU_ICM4207C)
add_definitions(-DIMU_INVERT_TILT_X)
add_definitions(-DIMU_INVERT_TILT_Y)
add_definitions(-DIMU_SWAP_TILT_XY)

# for size report script
set(CPU_TYPE_FULL STM32H750xB)
set(TARGET_LINKER_DIR stm32h750_sdram)
set(TARGET_EXTRAM_START 0xC0000000)

set(SIZE_TARGET_MEM_DEFINE "MEM_SIZE_SDRAM1=8192")
set(FIRMWARE_STRIP_BOOTLOADER YES)

add_definitions(-DPCBC14 -DPCBHORUS -DPCBFLYSKY)
add_definitions(-DBATTERY_CHARGE)
add_definitions(-DUSB_CHARGER)
add_definitions(-DSTM32_SUPPORT_32BIT_TIMERS)
add_definitions(-DFIRMWARE_QSPI)
add_definitions(-DFIRMWARE_FORMAT_UF2)
add_definitions(-DUSE_CUSTOM_EXTI_IRQ)

set(FLAVOUR c14)
add_definitions(-DRADIO_C14)

# Defines internal modules for PL18 via UART7
# set(INTERNAL_MODULES MULTI CACHE STRING "Internal modules")
# set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")

set(BITMAPS_TARGET bm480_bitmaps)
set(FONTS_TARGET x12_fonts)
set(LCD_DRIVER lcd_driver.cpp)
set(HARDWARE_TOUCH YES)
set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} ${BITMAPS_TARGET})
set(FIRMWARE_DEPENDENCIES datacopy)
set(SOFTWARE_KEYBOARD ON)
set(FLYSKY_GIMBAL NO)


add_definitions(
  -DSTM32H750IBTx -DSTM32H750xx
  -DSDRAM -DCCMRAM
  -DHARDWARE_TOUCH -DHARDWARE_KEYS -DSOFTWARE_KEYBOARD
  -DWS2812_MAX_LEDS=24
)

set(SDRAM ON)

add_definitions(-DAUDIO -DVOICE -DRTCLOCK -DSOFTWARE_VOLUME)
add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
if(PWR_BUTTON STREQUAL "MANAGED")
  add_definitions(-DPWR_BUTTON_PRESS)
endif()

if(INTERNAL_GPS)
  add_definitions(-DGPS_USART_BAUDRATE=${INTERNAL_GPS_BAUDRATE})
endif()

if(STICK_DEAD_ZONE)
  add_definitions(-DSTICK_DEAD_ZONE)
endif()

if(NOT UNEXPECTED_SHUTDOWN)
  add_definitions(-DNO_UNEXPECTED_SHUTDOWN)
endif()

if(DISK_CACHE)
  set(SRC ${SRC} disk_cache.cpp)
  add_definitions(-DDISK_CACHE)
endif()

if(FUNCTION_SWITCHES_RGB_LEDS)
  set(RGB_LEDS YES)
  add_definitions(-DFUNCTION_SWITCHES_RGB_LEDS)
endif()

if(RGB_LEDS)
  add_definitions(-DRGB_LEDS)
endif()

if(FUNCTION_SWITCHES)
add_definitions(-DFUNCTION_SWITCHES)
endif()

# VCP CLI
set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
set(CLI ON CACHE BOOL "Enable CLI")

set(TARGET_SRC_DIR targets/${TARGET_DIR})

set(BOARD_COMMON_SRC
  ${TARGET_SRC_DIR}/board.cpp
  ${TARGET_SRC_DIR}/bsp_io.cpp
  ${TARGET_SRC_DIR}/key_driver.cpp
  ${TARGET_SRC_DIR}/lcd_driver.cpp
  ${TARGET_SRC_DIR}/backlight_driver.cpp
  ${TARGET_SRC_DIR}/haptic_driver.cpp
  ${TARGET_SRC_DIR}/extflash_driver.cpp
  ${TARGET_SRC_DIR}/rotary_enc_simulator.cpp
  targets/common/arm/stm32/abnormal_reboot.cpp
  targets/common/arm/stm32/delays_driver.cpp
  targets/common/arm/stm32/dma2d.cpp
  targets/common/arm/stm32/flash_driver.cpp
  targets/common/arm/stm32/pwr_driver.cpp
  targets/common/arm/stm32/rtc_driver.cpp
  targets/common/arm/stm32/watchdog_driver.cpp
  drivers/pca95xx.cpp
)

# Bootloader board library
add_library(board_bl OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  ${TARGET_SRC_DIR}/sdram_driver.cpp
  ${RADIO_SRC_DIR}/gui/colorlcd/boot_menu.cpp
  targets/common/arm/stm32/stm32_qspi.cpp
  targets/common/arm/stm32/stm32_pulse_driver.cpp
)
add_dependencies(board_bl minimal_board_lib)
set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:board_bl>)

set(CMSIS_SRC ${TARGET_SRC_DIR}/system_clock.c)

# Firmware board library
add_library(board OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  ${TARGET_SRC_DIR}/led_driver.cpp
  ${TARGET_SRC_DIR}/tp_gt911.cpp
  ${TARGET_SRC_DIR}/battery_driver.cpp
  targets/common/arm/stm32/heartbeat_driver.cpp
  drivers/icm42607C.cpp
  targets/common/arm/stm32/mixer_scheduler_driver.cpp
  targets/common/arm/stm32/module_timer_driver.cpp
  targets/common/arm/stm32/stm32_pulse_driver.cpp
  targets/common/arm/stm32/stm32_softserial_driver.cpp
  targets/common/arm/stm32/stm32_qspi.cpp
  targets/common/arm/stm32/stm32_switch_driver.cpp
  targets/common/arm/stm32/audio_dac_driver.cpp
  targets/common/arm/stm32/stm32_rgbleds.cpp
  targets/common/arm/stm32/trainer_driver.cpp
  targets/common/arm/stm32/sticks_pwm_driver.cpp
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
