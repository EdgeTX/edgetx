option(DISK_CACHE "Enable SD card disk cache" ON)
option(UNEXPECTED_SHUTDOWN "Enable the Unexpected Shutdown screen" ON)
option(STICKS_DEAD_ZONE "Enable sticks dead zone" YES)
option(MULTIMODULE "DIY Multiprotocol TX Module (https://github.com/pascallanger/DIY-Multiprotocol-TX-Module)" ON)
option(GHOST "Ghost TX Module" ON)
option(PXX1 "PXX1 protocol support" ON)
option(PXX2 "PXX2 protocol support" OFF)
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
set(HARDWARE_EXTERNAL_MODULE YES)

set(TARGET_DIR nv14)

set(RTC_BACKUP_RAM YES)
set(PPM_LIMITS_SYMETRICAL YES)

# for size report script
set(CPU_TYPE_FULL STM32F429xI)
set(TARGET_LINKER_DIR stm32f429_sdram)
set(TARGET_SDRAM_START 0xC0000000)

set(SIZE_TARGET_MEM_DEFINE "MEM_SIZE_SDRAM1=8192")

option(USB_SERIAL "Enable USB serial (CDC)" ON)

set(RF_BAUD_RATE 921600 230400 115200 57600 38400 19200 9600 4800 2400 1200)
set(PCB_RF_BAUD 921600 CACHE STRING "INTERNAL_MODULE_BAUDRATE: ${RF_BAUD_RATE}")
set_property(CACHE PCB_RF_BAUD PROPERTY STRINGS ${RF_BAUD_RATE})

add_definitions(-DPCBNV14 -DPCBFLYSKY -DAFHDS2_BAUDRATE=${PCB_RF_BAUD})
add_definitions(-DBATTERY_CHARGE -DUSE_HATS_AS_KEYS)

if (PCBREV STREQUAL EL18)
    set(FLAVOUR el18)
    # defines existing internal modules
    set(INTERNAL_MODULES AFHDS3 CACHE STRING "Internal modules")
    set(DEFAULT_INTERNAL_MODULE FLYSKY_AFHDS3 CACHE STRING "Default internal module")
    set(AFHDS3 ON)
else()
    set(FLAVOUR nv14)
    # defines existing internal modules
    set(INTERNAL_MODULES AFHDS2A CACHE STRING "Internal modules")
    set(DEFAULT_INTERNAL_MODULE FLYSKY_AFHDS2A CACHE STRING "Default internal module")
    set(AFHDS2 ON)
endif()

set(BITMAPS_TARGET nv14_bitmaps)
set(FONTS_TARGET x12_fonts)
set(LCD_DRIVER lcd_driver.cpp)
set(TOUCH_DRIVER touch_driver.cpp)
set(HARDWARE_TOUCH YES)

set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} ${BITMAPS_TARGET})

set(HARDWARE_TOUCH ON)
set(FLYSKY_GIMBAL ON)

add_definitions(
  -DSTM32F429_439xx -DSTM32F429xx
  -DSDRAM -DCCMRAM -DCOLORLCD -DLIBOPENUI
  -DHARDWARE_TOUCH -DHARDWARE_KEYS)

set(SDRAM ON)

add_definitions(-DEEPROM_VARIANT=0 -DAUDIO -DVOICE -DRTCLOCK)
add_definitions(-DGPS_USART_BAUDRATE=${INTERNAL_GPS_BAUDRATE})
add_definitions(-DPWR_BUTTON_${PWR_BUTTON})

if(STICKS_DEAD_ZONE)
  add_definitions(-DSTICK_DEAD_ZONE)
endif()

if(NOT UNEXPECTED_SHUTDOWN)
  add_definitions(-DNO_UNEXPECTED_SHUTDOWN)
endif()

if(STICKS_DEAD_ZONE)
  add_definitions(-DSTICK_DEAD_ZONE)
endif()

# VCP CLI
set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
set(CLI ON CACHE BOOL "Enable CLI")

include_directories(${RADIO_SRC_DIR}/fonts/colorlcd gui/${GUI_DIR} gui/${GUI_DIR}/layouts)

file(GLOB THEMES_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd ${RADIO_SRC_DIR}/gui/colorlcd/themes/*.cpp)
file(GLOB LAYOUTS_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd ${RADIO_SRC_DIR}/gui/colorlcd/layouts/*.cpp)
file(GLOB WIDGETS_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd ${RADIO_SRC_DIR}/gui/colorlcd/widgets/*.cpp)

if(DISK_CACHE)
  set(SRC ${SRC} disk_cache.cpp)
  add_definitions(-DDISK_CACHE)
endif()

#set(AUX_SERIAL_DRIVER ../common/arm/stm32/aux_serial_driver.cpp)

set(GVAR_SCREEN model_gvars.cpp)

set(FIRMWARE_TARGET_SRC
  ${FIRMWARE_TARGET_SRC}
  board.cpp
  ${LCD_DRIVER}
  ${TOUCH_DRIVER}
  sdram_driver.c
  battery_driver.cpp
  backlight_driver.cpp
  )

set(FIRMWARE_SRC
  ${FIRMWARE_SRC}
  targets/common/arm/stm32/dma2d.cpp
  targets/common/arm/stm32/spi_flash.cpp
  targets/common/arm/stm32/diskio_spi_flash.cpp
  drivers/frftl.cpp
  targets/common/arm/stm32/vs1053b.cpp
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

