option(UNEXPECTED_SHUTDOWN "Enable the Unexpected Shutdown screen" ON)
option(PXX1 "PXX1 protocol support" OFF)
option(PXX2 "PXX2 protocol support" OFF)
option(AFHDS3 "AFHDS3 TX Module" OFF)
option(MULTIMODULE "DIY Multiprotocol TX Module (https://github.com/pascallanger/DIY-Multiprotocol-TX-Module)" OFF)
option(GHOST "Ghost TX Module" OFF)
option(MODULE_SIZE_STD "Standard size TX Module" ON)
option(LUA_MIXER "Enable LUA mixer/model scripts support" ON)

set(CPU_TYPE STM32H7)
set(HSE_VALUE 25000000)

set(FLAVOUR h747)
set(TARGET_DIR stm32h747i-eval)

# set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
set(SDCARD YES)
set(STORAGE_MODELSLIST YES)
# set(HAPTIC YES)
set(GUI_DIR colorlcd)
set(BITMAPS_DIR 480x272)
# set(RTC_BACKUP_RAM YES)
set(PPM_LIMITS_SYMETRICAL YES)
# set(USB_SERIAL ON CACHE BOOL "Enable USB serial (CDC)")
# set(HARDWARE_EXTERNAL_MODULE YES)

# for size report script
set(CPU_TYPE_FULL STM32H747Ix)
set(TARGET_LINKER_DIR stm32h7xx_sdram)
set(TARGET_LINKER_PARAMS "-Wl,--defsym=__SDRAM_START__=0xD0000000")
set(SIZE_TARGET_MEM_DEFINE "MEM_SIZE_SDRAM1=8192")
set(BITMAPS_TARGET ${FLAVOUR}_bitmaps)
set(FONTS_TARGET x12_fonts)

set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} ${BITMAPS_TARGET})
set(HARDWARE_TOUCH ON)
# set(FLYSKY_GIMBAL ON)
set(IMU ON)

add_definitions(
  -DSTM32H747xx -DCORE_CM7
  -DSDRAM -DCCMRAM -DCOLORLCD -DLIBOPENUI -DPCBHORUS -DPCBX10 -DPCBFRSKY
  -DHARDWARE_TOUCH -DSOFTWARE_KEYBOARD
  -DSD_SDIO_HAS_TRANSCEIVER
)

# Make malloc() thread-safe
add_definitions(-DTHREADSAFE_MALLOC)

set(SDRAM ON)

# add_definitions(-DAUDIO -DVOICE -DRTCLOCK)

# VCP CLI
set(CLI ON CACHE BOOL "Enable CLI")

if(NOT NATIVE_BUILD)
  set(TARGET_SRC_DIR targets/${TARGET_DIR})

  set(BOARD_COMMON_SRC
    ${TARGET_SRC_DIR}/board.cpp
    ${TARGET_SRC_DIR}/system_init.cpp
    ${TARGET_SRC_DIR}/lcd_driver.cpp
    ${TARGET_SRC_DIR}/otm8009a.c
    ${TARGET_SRC_DIR}/otm8009a_reg.c
    ${TARGET_SRC_DIR}/backlight_driver.cpp
    targets/common/arm/stm32/dma2d.cpp
  )

  # Bootloader board library
  add_library(board_bl OBJECT EXCLUDE_FROM_ALL
    ${BOARD_COMMON_SRC}
    ${TARGET_SRC_DIR}/system_clock.cpp
    ${TARGET_SRC_DIR}/sdram_driver.cpp
  )
  set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:board_bl>)

  # Firmware board library
  add_library(board OBJECT EXCLUDE_FROM_ALL
    ${BOARD_COMMON_SRC}
    ${TARGET_SRC_DIR}/bsp_io.cpp
    ${TARGET_SRC_DIR}/mfxstm32l152.cpp
    ${TARGET_SRC_DIR}/tp_ft6236.cpp
  )
  set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:board>)
endif()

include_directories(${RADIO_SRC_DIR}/fonts/colorlcd gui/${GUI_DIR} gui/${GUI_DIR}/layouts)

file(GLOB THEMES_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd ${RADIO_SRC_DIR}/gui/colorlcd/themes/*.cpp)
file(GLOB LAYOUTS_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd ${RADIO_SRC_DIR}/gui/colorlcd/layouts/*.cpp)
file(GLOB WIDGETS_SRC RELATIVE ${RADIO_SRC_DIR}/gui/colorlcd ${RADIO_SRC_DIR}/gui/colorlcd/widgets/*.cpp)

set(LCD_DRIVER lcd_driver.cpp)
set(GVAR_SCREEN model_gvars.cpp)

if(BOOTLOADER)
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    ../common/arm/loadboot.cpp
  )
endif()

# set(FIRMWARE_SRC
#   ${FIRMWARE_SRC}
  # targets/common/arm/stm32/audio_dac_driver.cpp
  # targets/common/arm/stm32/dma2d.cpp
  # targets/common/arm/stm32/spi_flash.cpp
  # targets/common/arm/stm32/diskio_spi_flash.cpp
  # targets/common/arm/stm32/stm32_ws2812.cpp
  # boards/generic_stm32/rgb_leds.cpp
  # drivers/frftl.cpp
# )
