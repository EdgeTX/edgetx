option(UNEXPECTED_SHUTDOWN "Enable the Unexpected Shutdown screen" ON)
option(PXX1 "PXX1 protocol support" OFF)
option(PXX2 "PXX2 protocol support" OFF)
option(AFHDS3 "AFHDS3 TX Module" OFF)
option(MULTIMODULE "DIY Multiprotocol TX Module (https://github.com/pascallanger/DIY-Multiprotocol-TX-Module)" OFF)
option(GHOST "Ghost TX Module" OFF)
option(MODULE_SIZE_STD "Standard size TX Module" ON)
option(LUA_MIXER "Enable LUA mixer/model scripts support" ON)
option(USB_HS "Enable USB HS" ON)

set(CPU_TYPE STM32H7RS)
set(HSE_VALUE 24000000)

set(FLAVOUR h7rs)
set(TARGET_DIR stm32h7s78-dk)

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
set(CPU_TYPE_FULL STM32H7S7xx)
set(TARGET_LINKER_DIR stm32h7rs_sdram)
set(TARGET_LINKER_PARAMS "-Wl,--defsym=__PSRAM_START__=0x90000000")
set(SIZE_TARGET_MEM_DEFINE "MEM_SIZE_SDRAM1=32768")
set(BITMAPS_TARGET ${FLAVOUR}_bitmaps)
set(FONTS_TARGET x12_fonts)

set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} ${BITMAPS_TARGET})
set(HARDWARE_TOUCH ON)
# set(FLYSKY_GIMBAL ON)
set(IMU ON)

set(FIRMWARE_QSPI ON)

add_definitions(
  -DSTM32H7S7xx -DCORE_CM7 -DFIRMWARE_QSPI
  -DSDRAM -DCCMRAM -DCOLORLCD -DLIBOPENUI -DPCBHORUS -DPCBX10 -DPCBFRSKY
  -DHARDWARE_TOUCH -DSOFTWARE_KEYBOARD
)

# Make malloc() thread-safe
add_definitions(-DTHREADSAFE_MALLOC)

set(SDRAM ON)
set(FIRMWARE_STRIP_BOOTLOADER ON)

# add_definitions(-DAUDIO -DVOICE -DRTCLOCK)

# VCP CLI
set(CLI ON CACHE BOOL "Enable CLI")

if(USE_USB_HS)
  add_definitions(-DUSE_USB_HS)
endif()

set(TARGET_SRC_DIR targets/${TARGET_DIR})

set(BOARD_COMMON_SRC
  ${TARGET_SRC_DIR}/board.cpp
  ${TARGET_SRC_DIR}/system_init.cpp
  ${TARGET_SRC_DIR}/lcd_driver.cpp
  ${TARGET_SRC_DIR}/backlight_driver.cpp
  targets/common/arm/stm32/abnormal_reboot.cpp
  targets/common/arm/stm32/dma2d.cpp
  targets/common/arm/stm32/pwr_driver.cpp
  targets/common/arm/stm32/flash_driver.cpp
  targets/common/arm/stm32/delays_driver.cpp
  targets/common/arm/stm32/rtc_driver.cpp
  targets/common/arm/stm32/watchdog_driver.cpp
  targets/common/arm/stm32/stm32_xspi_nor.cpp
)

# Bootloader board library
add_library(board_bl OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  ${TARGET_SRC_DIR}/system_clock.cpp
  ${TARGET_SRC_DIR}/extram_driver.cpp
  ${TARGET_SRC_DIR}/extflash_driver.cpp
  ${TARGET_SRC_DIR}/bootloader/boot_menu.cpp
)
set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:board_bl>)

# Firmware board library
add_library(board OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  ${TARGET_SRC_DIR}/tp_gt911.cpp
  targets/common/arm/stm32/lsm6ds_driver.cpp
  targets/common/arm/stm32/mixer_scheduler_driver.cpp
  targets/common/arm/stm32/stm32_switch_driver.cpp
)
set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:board>)

include_directories(${RADIO_SRC_DIR}/fonts/colorlcd gui/${GUI_DIR} gui/${GUI_DIR}/layouts)

if(BOOTLOADER)
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    ../common/arm/loadboot.cpp
  )
endif()
