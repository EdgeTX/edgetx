option(DISK_CACHE "Enable SD card disk cache" ON)
option(UNEXPECTED_SHUTDOWN "Enable the Unexpected Shutdown screen" ON)
option(STICKS_DEAD_ZONE "Enable sticks dead zone" YES)
option(MULTIMODULE "DIY Multiprotocol TX Module (https://github.com/pascallanger/DIY-Multiprotocol-TX-Module)" ON)
option(AFHDS2 "Support for AFHDS2" ON)
option(GHOST "Ghost TX Module" ON)
option(PXX1 "PXX1 protocol support" ON)
option(PXX2 "PXX2 protocol support" OFF)

set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
set(CPU_TYPE STM32F4)
set(HSE_VALUE 12000000)
set(SDCARD YES)
set(STORAGE SDCARD)
set(STORAGE_FORMAT RAW)
set(STORAGE_MODELSLIST YES)
set(HAPTIC YES)
set(GUI_DIR colorlcd)
set(BITMAPS_DIR 480x272)
set(HARDWARE_EXTERNAL_MODULE YES)

set(TARGET_DIR nv14)
add_definitions(-DHARDWARE_TRAINER_JACK)

if(BOOTLOADER)
  set(LINKER_SCRIPT targets/nv14/stm32f4_flash_bootloader.ld)
else()
  set(LINKER_SCRIPT targets/nv14/stm32f4_flash.ld)
endif()

if(YAML_STORAGE)
  set(STORAGE_CONVERT RAW)
  set(STORAGE_FORMAT  YAML)
endif()

set(RTC_BACKUP_RAM YES)
set(PPM_LIMITS_SYMETRICAL YES)
# for size report script
set(CPU_TYPE_FULL STM32F429xI)
set(SIZE_TARGET_MEM_DEFINE "MEM_SIZE_SDRAM2=8192")
option(USB_SERIAL "Enable USB serial (CDC)" ON)

set(RF_BAUD_RATE 921600 230400 115200 57600 38400 19200 9600 4800 2400 1200)
set(PCB_RF_BAUD 921600 CACHE STRING "INTERNAL_MODULE_BAUDRATE: ${RF_BAUD_RATE}")
set_property(CACHE PCB_RF_BAUD PROPERTY STRINGS ${RF_BAUD_RATE})

set(FLAVOUR nv14)
add_definitions(-DPCBNV14 -DPCBFLYSKY -DAFHDS2_BAUDRATE=${PCB_RF_BAUD})
add_definitions(-DBATTERY_CHARGE)

# defines existing internal modules
set(INTERNAL_MODULES AFHDS2A;AFHDS3 CACHE STRING "Internal modules")
set(DEFAULT_INTERNAL_MODULE FLYSKY CACHE STRING "Default internal module")

set(BITMAPS_TARGET nv14_bitmaps)
set(FONTS_TARGET x12_fonts)
set(LCD_DRIVER lcd_driver.cpp)
set(LUA_EXPORT lua_export_nv14)
set(TOUCH_DRIVER touch_driver.cpp)
set(HARDWARE_TOUCH YES)

set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} ${BITMAPS_TARGET})
set(FIRMWARE_DEPENDENCIES datacopy)

set(HARDWARE_TOUCH ON)

add_definitions(
  -DSTM32F429_439xx -DSTM32F429xx
  -DSDRAM -DCOLORLCD -DLIBOPENUI
  -DHARDWARE_TOUCH -DHARDWARE_KEYS)

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

set(AFHDS2 ON)
set(AFHDS3 ON)

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

set(SRC
  ${SRC}
  io/frsky_firmware_update.cpp
  )

set(GVAR_SCREEN model_gvars.cpp)

if(BOOTLOADER)
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    ../common/arm/loadboot.cpp
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

set(FIRMWARE_TARGET_SRC
  ${FIRMWARE_TARGET_SRC}
  board.cpp
  startup_stm32f42_43xxx.s
  ${LCD_DRIVER}
  ${TOUCH_DRIVER}
  sdram_driver.c
  trainer_driver.cpp
  battery_driver.cpp
  audio_spi_driver.cpp
  backlight_driver.cpp
  extmodule_helper.cpp
  ../horus/flyskyHallStick_driver.cpp
  )

set(FIRMWARE_SRC
  ${FIRMWARE_SRC}
  hal/adc_driver.cpp
  targets/common/arm/stm32/stm32_hal_adc.cpp
  targets/common/arm/stm32/timers_driver.cpp
  targets/common/arm/stm32/mixer_scheduler_driver.cpp
  targets/common/arm/stm32/extmodule_driver.cpp
  targets/common/arm/stm32/stm32_pulse_driver.cpp
  targets/common/arm/stm32/extmodule_serial_driver.cpp
  targets/common/arm/stm32/intmodule_serial_driver.cpp
  targets/common/arm/stm32/stm32_usart_driver.cpp
  targets/common/arm/stm32/pwr_driver.cpp
  targets/common/arm/stm32/sdio_sd.c
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

if(PYTHONINTERP_FOUND)
  add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/radio/src/datacopy.cpp
    WORKING_DIRECTORY ${RADIO_DIRECTORY}/src
    COMMAND ${PYTHON_EXECUTABLE} ${RADIO_DIRECTORY}/util/generate_datacopy.py datastructs_private.h
      -DPCBFLYSKY -DPCBNV14 -DCPUARM -DSTM32F4 -DSTM32F429_439xx -DCOLORLCD -DBACKUP -DSIMU
      -I. -Igui/colorlcd -Itargets/nv14 -Itargets/common/arm/stm32
      -I${THIRDPARTY_DIR} -I${THIRDPARTY_DIR}/libopenui/src
      -I${THIRDPARTY_DIR}/STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/STM32F4xx_StdPeriph_Driver/inc
      -I${THIRDPARTY_DIR}/STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/Device/ST/STM32F4xx/Include
      -I${THIRDPARTY_DIR}/STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/Include
      -Wno-asm-operand-widths -Wno-pragma-once-outside-header ${SYSROOT_ARG}
    > ${PROJECT_BINARY_DIR}/radio/src/datacopy.cpp
    DEPENDS ${RADIO_DIRECTORY}/src/datastructs.h ${RADIO_DIRECTORY}/util/generate_datacopy.py
    )
  add_custom_target(datacopy DEPENDS ${PROJECT_BINARY_DIR}/radio/src/datacopy.cpp)
endif()

