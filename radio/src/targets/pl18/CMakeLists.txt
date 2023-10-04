option(UNEXPECTED_SHUTDOWN "Enable the Unexpected Shutdown screen" ON)
option(STICKS_DEAD_ZONE "Enable sticks dead zone" YES)
option(MULTIMODULE "DIY Multiprotocol TX Module (https://github.com/pascallanger/DIY-Multiprotocol-TX-Module)" ON)
option(AFHDS2 "Support for AFHDS2" OFF)
option(GHOST "Ghost TX Module" ON)
option(PXX1 "PXX1 protocol support" ON)
option(PXX2 "PXX2 protocol support" OFF)
option(MODULE_SIZE_STD "Standard size TX Module" ON)

set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
set(CPU_TYPE STM32F4)
set(HSE_VALUE 12000000)
set(SDCARD YES)
set(STORAGE_MODELSLIST YES)
set(HAPTIC YES)
set(GUI_DIR colorlcd)
set(BITMAPS_DIR 480x272)
set(HARDWARE_EXTERNAL_MODULE YES)
set(WIRELESS_CHARGER YES)
set(TARGET_DIR pl18)

set(RTC_BACKUP_RAM YES)
set(PPM_LIMITS_SYMETRICAL YES)
# for size report script
set(CPU_TYPE_FULL STM32F429xI)
set(SIZE_TARGET_MEM_DEFINE "MEM_SIZE_SDRAM2=8192")
option(USB_SERIAL "Enable USB serial (CDC)" ON)

set(RF_BAUD_RATE 921600 230400 115200 57600 38400 19200 9600 4800 2400 1200)
set(PCB_RF_BAUD 921600 CACHE STRING "INTERNAL_MODULE_BAUDRATE: ${RF_BAUD_RATE}")
set_property(CACHE PCB_RF_BAUD PROPERTY STRINGS ${RF_BAUD_RATE})

set(FLAVOUR pl18)
add_definitions(-DPCBPL18 -DPCBFLYSKY)
add_definitions(-DBATTERY_CHARGE)
add_definitions(-DSOFTWARE_VOLUME)
add_definitions(-DSPI_FLASH)

# defines existing internal modules
# set(INTERNAL_MODULES MULTI CACHE STRING "Internal modules")
# set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")

set(BITMAPS_TARGET pl18_bitmaps)
set(FONTS_TARGET x12_fonts)
set(LCD_DRIVER lcd_driver.cpp)
set(TOUCH_DRIVER tp_cst340.cpp)
set(HARDWARE_TOUCH YES)
set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} ${BITMAPS_TARGET})
set(FIRMWARE_DEPENDENCIES datacopy)

set(HARDWARE_TOUCH ON)
set(SOFTWARE_KEYBOARD ON)
set(FLYSKY_GIMBAL ON)

add_definitions(
  -DSTM32F429_439xx -DSTM32F429xx
  -DSDRAM -DCOLORLCD -DLIBOPENUI
  -DHARDWARE_TOUCH -DHARDWARE_KEYS
  -DSOFTWARE_KEYBOARD -DUSE_TRIMS_AS_BUTTONS)

set(SDRAM ON)

add_definitions(-DEEPROM_VARIANT=0 -DAUDIO -DVOICE -DRTCLOCK)
add_definitions(-DGPS_USART_BAUDRATE=${INTERNAL_GPS_BAUDRATE})
add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
add_definitions(-DCROSSFIRE_NATIVE)
add_definitions(-DHARDWARE_EXTERNAL_MODULE)
add_definitions(-DWIRELESS_CHARGER)

if(STICKS_DEAD_ZONE)
  add_definitions(-DSTICK_DEAD_ZONE)
endif()

if(NOT UNEXPECTED_SHUTDOWN)
  add_definitions(-DNO_UNEXPECTED_SHUTDOWN)
endif()

if(STICKS_DEAD_ZONE)
  add_definitions(-DSTICK_DEAD_ZONE)
endif()
set(AFHDS3 ON)

# VCP CLI
set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
set(CLI ON CACHE BOOL "Enable CLI")

include_directories(${RADIO_SRC_DIR}/fonts/colorlcd gui/${GUI_DIR} gui/${GUI_DIR}/layouts)

file(GLOB THEMES_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd ${RADIO_SRC_DIR}/gui/colorlcd/themes/*.cpp)
file(GLOB LAYOUTS_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd ${RADIO_SRC_DIR}/gui/colorlcd/layouts/*.cpp)
file(GLOB WIDGETS_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd ${RADIO_SRC_DIR}/gui/colorlcd/widgets/*.cpp)

set(SRC
  ${SRC}
  io/frsky_firmware_update.cpp
  )

set(GVAR_SCREEN model_gvars.cpp)

if(BOOTLOADER)
  set(LINKER_SCRIPT targets/pl18/stm32f4_flash_bootloader.ld)
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    ../common/arm/loadboot.cpp
    )
else()
  set(LINKER_SCRIPT targets/pl18/stm32f4_flash.ld)
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

set(FIRMWARE_TARGET_SRC
  ${FIRMWARE_TARGET_SRC}
  ${LCD_DRIVER}
  ${TOUCH_DRIVER}
  board.cpp
  key_driver.cpp
  battery_driver.cpp
  backlight_driver.cpp
  sdram_driver.c
  )

set(FIRMWARE_SRC
  ${FIRMWARE_SRC}
  hal/adc_driver.cpp
  targets/common/arm/stm32/stm32_exti_driver.cpp
  targets/common/arm/stm32/timers_driver.cpp
  targets/common/arm/stm32/stm32_pulse_driver.cpp
  targets/common/arm/stm32/stm32_usart_driver.cpp
  targets/common/arm/stm32/pwr_driver.cpp
  targets/common/arm/stm32/trainer_driver.cpp
  targets/common/arm/stm32/audio_dac_driver.cpp
  targets/common/arm/stm32/spi_flash.cpp
  targets/common/arm/stm32/diskio_spi_flash.cpp
  drivers/frftl.cpp
  )

# Make malloc() thread-safe
add_definitions(-DTHREADSAFE_MALLOC)

set(STM32LIB_SRC
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_fmc.c
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_ltdc.c
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma2d.c
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c
  )

