option(UNEXPECTED_SHUTDOWN "Enable the Unexpected Shutdown screen" ON)
option(PXX1 "PXX1 protocol support" ON)
option(PXX2 "PXX2 protocol support" OFF)
option(AFHDS3 "AFHDS3 TX Module" ON)
option(MULTIMODULE "DIY Multiprotocol TX Module (https://github.com/pascallanger/DIY-Multiprotocol-TX-Module)" ON)
option(GHOST "Ghost TX Module" ON)
option(MODULE_SIZE_STD "Standard size TX Module" ON)
option(LUA_MIXER "Enable LUA mixer/model scripts support" ON)

set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
set(CPU_TYPE STM32F4)
set(HSE_VALUE 12000000)
set(SDCARD YES)
set(STORAGE_MODELSLIST YES)
set(HAPTIC YES)
set(GUI_DIR colorlcd)
set(BITMAPS_DIR 480x272)
set(TARGET_DIR pl18)
set(RTC_BACKUP_RAM YES)
set(PPM_LIMITS_SYMETRICAL YES)
set(USB_SERIAL ON CACHE BOOL "Enable USB serial (CDC)")
set(HARDWARE_EXTERNAL_MODULE YES)
set(WIRELESS_CHARGER YES)

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

if(PCBREV STREQUAL PL18EV)
  set(FLAVOUR pl18ev)
  add_definitions(-DRADIO_PL18EV)
else()
  set(FLAVOUR pl18)
  add_definitions(-DRADIO_PL18)

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
set(FLYSKY_GIMBAL ON)

add_definitions(
  -DSTM32F429_439xx -DSTM32F429xx
  -DSDRAM -DCCMRAM -DCOLORLCD -DLIBOPENUI
  -DHARDWARE_TOUCH -DHARDWARE_KEYS
  -DSOFTWARE_KEYBOARD -DUSE_HATS_AS_KEYS)

set(SDRAM ON)

add_definitions(-DEEPROM_VARIANT=0 -DAUDIO -DVOICE -DRTCLOCK)
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

include_directories(${RADIO_SRC_DIR}/fonts/colorlcd gui/${GUI_DIR} gui/${GUI_DIR}/layouts)

file(GLOB THEMES_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd ${RADIO_SRC_DIR}/gui/colorlcd/themes/*.cpp)
file(GLOB LAYOUTS_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd ${RADIO_SRC_DIR}/gui/colorlcd/layouts/*.cpp)
file(GLOB WIDGETS_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd ${RADIO_SRC_DIR}/gui/colorlcd/widgets/*.cpp)

set(GVAR_SCREEN model_gvars.cpp)

set(FIRMWARE_TARGET_SRC
  ${FIRMWARE_TARGET_SRC}
  ${LCD_DRIVER}
  ${TOUCH_DRIVER}
  board.cpp
  key_driver.cpp
  battery_driver.cpp
  backlight_driver.cpp
  led_driver.cpp
  sdram_driver.c
  )

set(FIRMWARE_SRC
  ${FIRMWARE_SRC}
  targets/common/arm/stm32/audio_dac_driver.cpp
  targets/common/arm/stm32/dma2d.cpp
  targets/common/arm/stm32/spi_flash.cpp
  targets/common/arm/stm32/diskio_spi_flash.cpp
  targets/common/arm/stm32/stm32_ws2812.cpp
  boards/generic_stm32/rgb_leds.cpp
  drivers/frftl.cpp
  )

if(BOOTLOADER)
  set(FIRMWARE_SRC
    ${FIRMWARE_SRC}
    targets/common/arm/loadboot.cpp
  )
endif()

set(SRC ${SRC}
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

set(STM32LIB_SRC
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.c
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_ltdc.c
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c
  )

