option(DISK_CACHE "Enable SD card disk cache" ON)
option(UNEXPECTED_SHUTDOWN "Enable the Unexpected Shutdown screen" ON)
option(IMU_LSM6DS33 "Enable I2C2 and LSM6DS33 IMU" OFF)
option(PXX1 "PXX1 protocol support" ON)
option(PXX2 "PXX2 protocol support" OFF)
option(AFHDS3 "AFHDS3 TX Module" ON)
option(MULTIMODULE "DIY Multiprotocol TX Module (https://github.com/pascallanger/DIY-Multiprotocol-TX-Module)" ON)
option(GHOST "Ghost TX Module" ON)
option(SPACEMOUSE "Support for SpaceMouse" OFF)

set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
set(CPU_TYPE STM32F4)
set(HSE_VALUE 12000000)
set(SDCARD YES)
set(STORAGE_MODELSLIST YES)
set(HAPTIC YES)
set(GUI_DIR colorlcd)
set(BITMAPS_DIR 480x272)
set(NAVIGATION_TYPE horus)
set(TARGET_DIR horus)
set(LINKER_SCRIPT targets/horus/stm32f4_flash.ld)
set(RTC_BACKUP_RAM YES)
set(PPM_LIMITS_SYMETRICAL YES)
set(USB_SERIAL ON CACHE BOOL "Enable USB serial (CDC)")
set(ROTARY_ENCODER YES)
set(HARDWARE_EXTERNAL_MODULE YES)

if(BOOTLOADER)
  set(LINKER_SCRIPT targets/horus/stm32f4_flash_bootloader.ld)
else()
  set(LINKER_SCRIPT targets/horus/stm32f4_flash.ld)
endif()

# for size report script
set(CPU_TYPE_FULL STM32F429xI)
set(SIZE_TARGET_MEM_DEFINE "MEM_SIZE_SDRAM2=8192")

if(SPLASH STREQUAL OFF)
  message(FATAL_ERROR "Option SPLASH=OFF not supported on Horus")
endif()

add_definitions(-DPCBFRSKY)

# defines existing internal modules
set(INTERNAL_MODULES PXX1;PXX2;MULTI;CRSF CACHE STRING "Internal modules")

# VCP CLI
set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
set(CLI ON CACHE BOOL "Enable CLI")
set(AUX_SERIAL ON)

# IMU support
set(IMU ON)

if (PCB STREQUAL X10)
  set(PCBREV "STD" CACHE STRING "PCB Revision")
  add_definitions(-DSOFTWARE_VOLUME)
  set(FIRMWARE_SRC
    ${FIRMWARE_SRC}
    targets/common/arm/stm32/stm32_hal_adc.cpp
    targets/common/arm/stm32/audio_dac_driver.cpp
    )
  if (NOT PCBREV STREQUAL TX16S)
      set(FIRMWARE_SRC
        ${FIRMWARE_SRC}
        targets/common/arm/stm32/sticks_pwm_driver.cpp
        )
  endif()
  set(BITMAPS_TARGET x10_bitmaps)
  set(FONTS_TARGET x10_fonts)
  set(LCD_DRIVER lcd_driver.cpp)
  set(LUA_EXPORT lua_export_x10)
  if (PCBREV STREQUAL EXPRESS)
    set(FLAVOUR x10express)
    set(DEFAULT_INTERNAL_MODULE ISRM_PXX2 CACHE STRING "Default internal module")
    set(BLUETOOTH ON)
    set(AUX_SERIAL OFF)
    add_definitions(-DHARDWARE_POWER_MANAGEMENT_UNIT)
    add_definitions(-DRADIO_X10E)
    add_definitions(-DMANUFACTURER_FRSKY)
  elseif (PCBREV STREQUAL T16)
    set(FLAVOUR t16)
    set(LUA_EXPORT lua_export_t16)
    add_definitions(-DRADIO_T16)
    add_definitions(-DRADIO_FAMILY_T16)
    set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
    option(BLUETOOTH "Support for bluetooth module" OFF)
    option(INTERNAL_GPS "Support for internal NMEA GPS" ON)    
    set(HARDWARE_TOUCH YES)
    add_definitions(-DMANUFACTURER_JUMPER)
    if (NOT BLUETOOTH)
      set(AUX2_SERIAL ON)
    endif()
    set(INTERNAL_GPS_BAUDRATE "9600" CACHE STRING "Baud rate for internal GPS")    
    set(FLYSKY_GIMBAL ON)
  elseif (PCBREV STREQUAL TX16S)
    set(FLAVOUR tx16s)
    set(LUA_EXPORT lua_export_t16)
    add_definitions(-DRADIO_TX16S)
    add_definitions(-DRADIO_FAMILY_T16)
    set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
    option(BLUETOOTH "Support for bluetooth module" OFF)
    option(INTERNAL_GPS "Support for internal NMEA GPS" ON)
    set(HARDWARE_TOUCH YES)
    set(SWSERIALPOWER YES)
    set(USB_CHARGER YES)
    set(DEFAULT_THEME "DARKBLUE")
    add_definitions(-DMANUFACTURER_RADIOMASTER)
    if (NOT BLUETOOTH)
      set(AUX2_SERIAL ON)
    endif()
    set(INTERNAL_GPS_BAUDRATE "9600" CACHE STRING "Baud rate for internal GPS")
    set(FLYSKY_GIMBAL ON)
  elseif (PCBREV STREQUAL T18)
    set(FLAVOUR t18)
    set(LUA_EXPORT lua_export_t16)
    add_definitions(-DRADIO_T18)
    add_definitions(-DRADIO_FAMILY_T16)
    set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
    option(BLUETOOTH "Support for bluetooth module" OFF)
    option(INTERNAL_GPS "Support for internal NMEA GPS" ON)    
    set(HARDWARE_TOUCH YES)
    add_definitions(-DMANUFACTURER_JUMPER)
    if (NOT BLUETOOTH)
      set(AUX2_SERIAL ON)
    endif()
    set(INTERNAL_GPS_BAUDRATE "9600" CACHE STRING "Baud rate for internal GPS")    
    set(FLYSKY_GIMBAL ON)
  else()
    set(FLAVOUR x10)
    set(DEFAULT_INTERNAL_MODULE XJT_PXX1 CACHE STRING "Default internal module")
    set(BLUETOOTH ON)
    add_definitions(-DMANUFACTURER_FRSKY)
  endif()
elseif (PCB STREQUAL X12S)
  set(HARDWARE_TOUCH YES)
  set(DEFAULT_INTERNAL_MODULE XJT_PXX1 CACHE STRING "Default internal module")
  set(FLAVOUR x12s)
  set(BLUETOOTH ON)
  set(PCBREV "13" CACHE STRING "PCB Revision")
  if(${PCBREV} GREATER 10)
    option(INTERNAL_GPS "Internal GPS installed" YES)
  else()
    option(INTERNAL_GPS "Internal GPS installed" NO)
    if(NOT INTERNAL_GPS)
      message("Horus: Internal GPS is optional, use INTERNAL_GPS=YES option to enable it")
    endif()
  endif()
  set(INTERNAL_GPS_BAUDRATE "9600" CACHE STRING "Baud rate for internal GPS")
  set(AUX2_SERIAL ON) # wire to GPS
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    audio_spi_driver.cpp
    x12s_adc_driver.cpp
    )
  set(BITMAPS_TARGET x12s_bitmaps)
  set(FONTS_TARGET x12s_fonts)
  set(LCD_DRIVER lcd_driver.cpp)
  set(LUA_EXPORT lua_export_x12s)
  add_definitions(-DMANUFACTURER_FRSKY)
endif()

add_definitions(-DPCB${PCB})
add_definitions(-DPCBREV=${PCBREV})
add_definitions(-DPCBREV_${PCBREV})

set(RADIO_DEPENDENCIES
  ${RADIO_DEPENDENCIES}
  ${BITMAPS_TARGET}
  )

add_definitions(-DPCBHORUS -DSTM32F429_439xx -DSTM32F429xx -DSDRAM -DCCMRAM -DCOLORLCD -DHARDWARE_KEYS)
add_definitions(-DEEPROM_VARIANT=0 -DAUDIO -DVOICE -DRTCLOCK)
add_definitions(-DGPS_USART_BAUDRATE=${INTERNAL_GPS_BAUDRATE})
add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
add_definitions(-DHARDWARE_TRAINER_JACK)
add_definitions(-DSTM32_SUPPORT_32BIT_TIMERS)

set(SDRAM ON)

if(DEFAULT_THEME)
  add_definitions(-DDEFAULT_THEME_${DEFAULT_THEME})
else()
  add_definitions(-DDEFAULT_THEME_DEFAULT)
endif()

if(NOT UNEXPECTED_SHUTDOWN)
  add_definitions(-DNO_UNEXPECTED_SHUTDOWN)
endif()

if(IMU_LSM6DS33)
  add_definitions(-DIMU_LSM6DS33)
  set(AUX_SERIAL OFF)
endif()

if(NOT INTERNAL_MODULE_PXX1)
  option(ALLOW_TRAINER_MULTI "Allow multimodule to be use as trainer input" OFF)
endif()

if("PXX2" IN_LIST INTERNAL_MODULES)
  if (NOT PCBREV STREQUAL EXPRESS)
    set(ACCESS_DENIED ON)
    add_definitions(-DACCESS_DENIED)
  endif()
endif()

if(ALLOW_TRAINER_MULTI)
  add_definitions(-DALLOW_TRAINER_MULTI)
endif()

if(DISK_CACHE)
  set(SRC ${SRC} disk_cache.cpp)
  add_definitions(-DDISK_CACHE)
endif()

if(INTERNAL_GPS)
  set(SRC ${SRC} gps.cpp)
  add_definitions(-DINTERNAL_GPS)
  message("-- Internal GPS enabled")
endif()

if(SPACEMOUSE)
  add_definitions(-DSPACEMOUSE)
  set(SRC ${SRC} spacemouse.cpp)
  message("-- SpaceMouse enabled")
endif()

set(GVAR_SCREEN model_gvars.cpp)

if(HARDWARE_TOUCH)
  set(TOUCH_DRIVER tp_gt911.cpp)
  add_definitions(-DHARDWARE_TOUCH)
endif()

if(SWSERIALPOWER)
  add_definitions(-DSWSERIALPOWER)
endif()

if(USB_CHARGER)
  set(FIRMWARE_TARGET_SRC ${FIRMWARE_TARGET_SRC} usb_charger_driver.cpp)
  add_definitions(-DUSB_CHARGER)
endif()

if(BLUETOOTH)
  add_definitions(-DBLUETOOTH)
  set(FIRMWARE_SRC
    ${FIRMWARE_SRC}
    targets/common/arm/stm32/bluetooth_driver.cpp)
  set(SRC
    ${SRC}
    bluetooth.cpp)
  add_definitions(-DHARDWARE_TRAINER_BLUETOOTH)
endif()

set(FIRMWARE_TARGET_SRC
  ${FIRMWARE_TARGET_SRC}
  ${LCD_DRIVER}
  ${TOUCH_DRIVER}
  board.cpp
  backlight_driver.cpp
  led_driver.cpp
  sdram_driver.c
  )

set(FIRMWARE_SRC
  ${FIRMWARE_SRC}
  hal/adc_driver.cpp
  boards/generic_stm32/module_ports.cpp
  boards/generic_stm32/aux_ports.cpp
  boards/generic_stm32/sport_update.cpp
  boards/generic_stm32/intmodule_heartbeat.cpp
  targets/common/arm/stm32/stm32_exti_driver.cpp
  targets/common/arm/stm32/mixer_scheduler_driver.cpp
  targets/common/arm/stm32/timers_driver.cpp
  targets/common/arm/stm32/extmodule_driver.cpp
  targets/common/arm/stm32/stm32_pulse_driver.cpp
  targets/common/arm/stm32/stm32_usart_driver.cpp
  targets/common/arm/stm32/rotary_encoder_driver.cpp
  targets/common/arm/stm32/pwr_driver.cpp
  targets/common/arm/stm32/heartbeat_driver.cpp
  targets/common/arm/stm32/trainer_driver.cpp
  targets/common/arm/stm32/sdio_sd.c
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
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma2d.c
  )
