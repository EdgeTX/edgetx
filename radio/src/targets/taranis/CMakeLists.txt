option(SHUTDOWN_CONFIRMATION "Shutdown confirmation" OFF)
option(PXX1 "PXX1 protocol support" ON)
option(PXX2 "PXX2 protocol support" OFF)
option(AFHDS3 "AFHDS3 TX Module" OFF)
option(GHOST "Ghost TX Module" ON)
option(INTERNAL_MODULE_PPM "Support for PPM internal module" OFF)
option(AUTOUPDATE "Auto update internal chips from SD" OFF)
option(BIND_KEY "Enable bind button" OFF)
option(BLUETOOTH "FrSky BT module support" OFF)
option(RGBLEDS "RGBLEDS addressable LED support" OFF)

set(USE_RTC_CLOCK YES)
set(HARDWARE_EXTERNAL_MODULE YES)
set(TARGET_DIR taranis)

set(HSE_VALUE 12000000)
set(TARGET_DIR taranis)
set(PPM_LIMITS_SYMETRICAL YES)

add_definitions(-DPCBTARANIS)

if(PCB STREQUAL X9E)
  set(DEFAULT_INTERNAL_MODULE XJT_PXX1 CACHE STRING "Default internal module")
  set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
  set(STICKS "STANDARD" CACHE STRING "Sticks type (STANDARD/HORUS)")
  set(CPU_TYPE_FULL STM32F407xE)
  set(HAPTIC YES)
  set(FLAVOUR x9e)
  add_definitions(-DPCBX9E -DPCBX9)
  add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    top_lcd_driver.cpp
    )
  set(AUX_SERIAL ON)
  if(STICKS STREQUAL HORUS)
    add_definitions(-DHORUS_STICKS)
  endif()
  set(GUI_DIR 212x64)
  set(BITMAPS_DIR 212x64)
  set(BITMAPS_TARGET taranis_bitmaps)
  set(FONTS_TARGET taranis_fonts)
  set(LCD_DRIVER lcd_driver_spi.cpp)
  set(BLUETOOTH YES)
  set(ROTARY_ENCODER YES)
  set(MODULE_SIZE_STD YES)
elseif(PCB STREQUAL X9D+)
  set(HAPTIC YES)
  add_definitions(-DPCBX9DP -DPCBX9)
  set(GUI_DIR 212x64)
  set(BITMAPS_DIR 212x64)
  set(BITMAPS_TARGET taranis_bitmaps)
  set(FONTS_TARGET taranis_fonts)
  set(LCD_DRIVER lcd_driver_spi.cpp)
  set(PCBREV 2019 CACHE STRING "PCB Revision")
  if(NOT PCBREV STREQUAL 2019)
    message(FATAL_ERROR "PCB=X9D+ requires PCBREV=2019, got '${PCBREV}'. "
      "The STM32F2 based X9D+ was last supported in EdgeTX 2.11. "
      "Pass -DPCBREV=2019, or remove a stale PCBREV entry from CMakeCache.txt.")
  endif()
  #set(AFHDS3 NO)
  add_definitions(-DPCBREV=${PCBREV})
  set(DEFAULT_INTERNAL_MODULE ISRM_PXX2 CACHE STRING "Default internal module")
  set(FLAVOUR x9d+2019)
  set(CPU_TYPE_FULL STM32F407xE)
  set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
  add_definitions(-DSOFTWARE_VOLUME -DPWR_BUTTON_${PWR_BUTTON} -DHARDWARE_POWER_MANAGEMENT_UNIT)
  set(PXX2 ON)
  set(PXX_FREQUENCY "HIGH")
  set(BLUETOOTH YES)
  set(ROTARY_ENCODER YES)
  set(INTERNAL_MODULE_SERIAL YES)
  set(MODULE_SIZE_STD YES)
  set(FLYSKY_GIMBAL ON)
elseif(PCB STREQUAL X7)
  set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
  set(HAPTIC YES)
  set(GUI_DIR 128x64)
  set(BITMAPS_DIR 128x64)
  set(BITMAPS_TARGET 9x_bitmaps)
  set(FONTS_TARGET 9x_fonts_1bit)
  set(LCD_DRIVER lcd_driver_spi.cpp)
  add_definitions(-DPCBX7 -DSOFTWARE_VOLUME)
  add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
  if(PCBREV STREQUAL TPROV2)
    set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_SML YES)
    set(FLAVOUR tprov2)
    set(CPU_TYPE_FULL STM32F407xE)
    set(ROTARY_ENCODER YES)
    set(FUNCTION_SWITCHES ON)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
  elseif(PCBREV STREQUAL TPROS)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_SML YES)
    set(FLAVOUR tpros)
    set(CPU_TYPE_FULL STM32F407xG)
    set(ROTARY_ENCODER YES)
    set(USB_CHARGE_LED YES)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
  elseif(PCBREV STREQUAL BUMBLEBEE)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_SML YES)
    set(FLAVOUR bumblebee)
    set(NAVIGATION_TYPE x7)
    set(CPU_TYPE_FULL STM32F407xG)
    set(ROTARY_ENCODER YES)
    set(FUNCTION_SWITCHES YES)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
    set(AUX_SERIAL ON)
  elseif (PCBREV STREQUAL T14)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_STD YES)
    set(FLAVOUR t14)
    set(ROTARY_ENCODER YES)
    set(CPU_TYPE_FULL STM32F407xG)
    set(PXX2 ON)
    set(USB_CHARGE_LED YES)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
  elseif (PCBREV STREQUAL T12MAX)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_STD YES)
    set(FLAVOUR t12max)
    set(ROTARY_ENCODER YES)
    set(USB_CHARGE_LED YES)
    set(CPU_TYPE_FULL STM32F407xG)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
  elseif(PCBREV STREQUAL T20)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_SML YES)
    set(FLAVOUR t20)
    set(ROTARY_ENCODER YES)
    set(CPU_TYPE_FULL STM32F407xE)
    set(FLYSKY_GIMBAL ON)
    set(FUNCTION_SWITCHES ON)
    add_definitions(-DRADIO_FAMILY_T20)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
  elseif(PCBREV STREQUAL T20V2)
    set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_SML YES)
    set(FLAVOUR t20v2)
    set(ROTARY_ENCODER YES)
    set(CPU_TYPE_FULL STM32F407xG)
    set(FLYSKY_GIMBAL ON)
    set(PXX2 ON)
    set(FUNCTION_SWITCHES ON)
    add_definitions(-DRADIO_FAMILY_T20)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
  elseif(PCBREV STREQUAL TX12MK2)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_STD YES)
    set(FLAVOUR tx12mk2)
    set(CPU_TYPE_FULL STM32F407xE)
    set(ROTARY_ENCODER YES)
    set(USB_CHARGER YES)
    if (NOT BLUETOOTH)
      set(AUX_SERIAL ON)
    endif()
    set(PXX2 ON)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
  elseif(PCBREV STREQUAL MT12)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_SML YES)
    set(FLAVOUR mt12)
    set(CPU_TYPE_FULL STM32F407xG)
    set(ROTARY_ENCODER YES)
    set(BLUETOOTH NO)
    set(USB_CHARGER YES)
    set(HELI NO)
    set(PXX2 ON)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
    set(FLEXSW "2" CACHE STRING "Max flex inputs usable as switches")
    if (NOT BLUETOOTH)
      set(AUX_SERIAL ON)
    endif()
    set(RGBLEDS YES)
  elseif(PCBREV STREQUAL BOXER)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_STD YES)
    set(FLAVOUR boxer)
    set(CPU_TYPE_FULL STM32F407xG)
    set(ROTARY_ENCODER YES)
    set(USB_CHARGER YES)
    set(PXX2 ON)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
    set(FLYSKY_GIMBAL ON)
    if (NOT BLUETOOTH)
      set(AUX_SERIAL ON)
    endif()
  elseif(PCBREV STREQUAL GX12)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_SML YES)
    set(FLAVOUR gx12)
    set(NAVIGATION_TYPE x7)
    set(CPU_TYPE_FULL STM32F407xG)
    set(ROTARY_ENCODER YES)
    set(USB_CHARGER YES)
    add_definitions(-DRGB_LEDS_900NS)
    set(PXX2 ON)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
    set(FLYSKY_GIMBAL YES)
    if (NOT BLUETOOTH)
      set(AUX_SERIAL ON)
    endif()
    set(AUX2_SERIAL OFF)
    set(RGBLEDS YES)
    set(FUNCTION_SWITCHES YES)
    set(FUNCTION_SWITCHES_RGB_LEDS YES)
  elseif(PCBREV STREQUAL ZORRO)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_SML YES)
    set(FLAVOUR zorro)
    set(CPU_TYPE_FULL STM32F407xE)
    set(ROTARY_ENCODER YES)
    set(USB_CHARGER YES)
    set(PXX2 ON)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
    if (NOT BLUETOOTH)
      set(AUX_SERIAL ON)
    endif()
  elseif(PCBREV STREQUAL POCKET)
    set(HAPTIC YES)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_SML YES)
    set(FLAVOUR pocket)
    set(CPU_TYPE_FULL STM32F407xE)
    set(ROTARY_ENCODER YES)
    set(USB_CHARGER YES)
    set(PXX2 ON)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
    if (NOT BLUETOOTH)
      set(AUX_SERIAL ON)
    endif()
  elseif(PCBREV STREQUAL COMMANDO8)
    set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(HARDWARE_EXTERNAL_MODULE YES)
    set(MODULE_SIZE_SML YES)
    set(FLAVOUR commando8)
    set(CPU_TYPE_FULL STM32F407xE)
    set(ROTARY_ENCODER NO)
    set(BLUETOOTH NO)
    set(USE_RTC_CLOCK NO)
    add_definitions(-DMUTUALLY_EXCLUSIVE_MODULES)
  elseif(PCBREV STREQUAL ACCESS)
    set(DEFAULT_INTERNAL_MODULE ISRM_PXX2 CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_STD YES)
    set(FLAVOUR x7access)
    set(CPU_TYPE_FULL STM32F407xE)
    add_definitions(-DPCBREV=${PCBREV})
    add_definitions(-DPCBX7ACCESS)
    add_definitions(-DHARDWARE_POWER_MANAGEMENT_UNIT)
    set(PXX2 ON)
    set(PXX_FREQUENCY "HIGH")
    set(BLUETOOTH YES)
    set(ROTARY_ENCODER YES)
  elseif(PCBREV STREQUAL V14 OR PCBREV STREQUAL V14LCD)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(MODULE_SIZE_STD YES)
    if(PCBREV STREQUAL V14LCD)
      set(FLAVOUR v14lcd)
    else()
      set(FLAVOUR v14)
    endif()
    set(CPU_TYPE_FULL STM32F407xG)
    set(ROTARY_ENCODER YES)
    set(USB_CHARGER YES)
    add_definitions(-DSTM32_SUPPORT_32BIT_TIMERS)
    set(PXX2 ON)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
    set(CSD203_SENSOR ON)
    set(FLYSKY_GIMBAL ON)
    if (NOT BLUETOOTH)
      set(AUX_SERIAL ON)
    endif()
  else()
    message(FATAL_ERROR "PCB=X7 requires a supported PCBREV, got '${PCBREV}'. "
      "The STM32F2 based QX7 (ACCST) was last supported in EdgeTX 2.11.")
  endif()
endif()

message("CPU_TYPE_FULL = ${CPU_TYPE_FULL}")
if((CPU_TYPE_FULL STREQUAL STM32F407xE) OR (CPU_TYPE_FULL STREQUAL STM32F407xG))
  set(CPU_TYPE STM32F4)
  add_definitions(-DSTM32F40_41xxx -DSTM32F407xx)
  set(TARGET_LINKER_DIR stm32f40x)
  if (CPU_TYPE_FULL STREQUAL STM32F407xG)
    set(TARGET_FLASH_SIZE 1M)
    add_definitions(-DSTM32F407xG)
  else()
    set(USBJ_EX OFF)
  endif()
elseif (CPU_TYPE_FULL STREQUAL STM32F413xG)
  set(CPU_TYPE STM32F4)
  set(RTC_BACKUP_RAM YES)
  add_definitions(-DSTM32F40_41xxx -DSTM32F413xx)
  set(TARGET_LINKER_DIR stm32f413)
else()
  message(FATAL_ERROR "Unknown CPU_TYPE_FULL" )
endif()

if ((CPU_TYPE_FULL STREQUAL STM32F407xG) AND (NOT PCBREV STREQUAL MT12))
  set(RTC_BACKUP_RAM YES)
endif()

if(FUNCTION_SWITCHES)
  add_definitions(-DFUNCTION_SWITCHES)
endif()

if(FUNCTION_SWITCHES_RGB_LEDS)
  set(RGB_LEDS YES)
  add_definitions(-DFUNCTION_SWITCHES_RGB_LEDS)
endif()

if(RGB_LEDS)
  add_definitions(-DRGB_LEDS)
endif()

if(ENABLE_SERIAL_PASSTHROUGH)
  set(CLI ON "Enable CLI")
endif()

if(INTERNAL_MODULE_SERIAL)

  # defines default internal modules
  if(DEFAULT_INTERNAL_MODULE STREQUAL XJT_PXX1)
    set(INTERNAL_MODULES PXX1 CACHE STRING "Internal modules")
  elseif(DEFAULT_INTERNAL_MODULE STREQUAL ISRM_PXX2)
    set(INTERNAL_MODULES PXX2 CACHE STRING "Internal modules")
  else()
    set(INTERNAL_MODULES MULTI;CRSF CACHE STRING "Internal modules")
  endif()

  add_definitions(-DINTERNAL_MODULE_SERIAL)
  if(PXX_FREQUENCY STREQUAL HIGH)
    add_definitions(-DPXX_FREQUENCY_HIGH)
  endif()

else()

  # defines existing internal modules
  set(INTERNAL_MODULES PXX1)

endif()

set(TARGET_SRC_DIR targets/${TARGET_DIR})

set(BOARD_COMMON_SRC
  ${TARGET_SRC_DIR}/board.cpp
  ${TARGET_SRC_DIR}/haptic_driver.cpp
  ${TARGET_SRC_DIR}/backlight_driver.cpp
  ${TARGET_SRC_DIR}/${LCD_DRIVER}
  targets/common/arm/stm32/abnormal_reboot.cpp
  targets/common/arm/stm32/delays_driver.cpp
  targets/common/arm/stm32/flash_driver.cpp
  targets/common/arm/stm32/pwr_driver.cpp
  targets/common/arm/stm32/rtc_driver.cpp
  targets/common/arm/stm32/watchdog_driver.cpp
)

if(ROTARY_ENCODER)
  list(APPEND BOARD_COMMON_SRC
    targets/common/arm/stm32/rotary_encoder_driver.cpp
  )
endif()

if(PCBREV STREQUAL MT12)
  list(APPEND BOARD_COMMON_SRC
    ${TARGET_SRC_DIR}/pwm_ch_driver.cpp
  )
endif()

# Bootloader board library
add_library(board_bl OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  ${RADIO_SRC_DIR}/gui/common/stdlcd/boot_menu.cpp
)
# add_dependencies(board_bl ${BITMAPS_TARGET})
set(BOOTLOADER_SRC ${BOOTLOADER_SRC} $<TARGET_OBJECTS:board_bl>)

# Firmware board library
add_library(board OBJECT EXCLUDE_FROM_ALL
  ${BOARD_COMMON_SRC}
  ${TARGET_SRC_DIR}/volume_i2c.cpp
  targets/common/arm/stm32/audio_dac_driver.cpp
  targets/common/arm/stm32/delays_driver.cpp
  targets/common/arm/stm32/heartbeat_driver.cpp
  targets/common/arm/stm32/mixer_scheduler_driver.cpp
  targets/common/arm/stm32/module_timer_driver.cpp
  targets/common/arm/stm32/stm32_pulse_driver.cpp
  targets/common/arm/stm32/stm32_softserial_driver.cpp
  targets/common/arm/stm32/stm32_switch_driver.cpp
  targets/common/arm/stm32/trainer_driver.cpp
  targets/common/arm/stm32/stm32_rgbleds.cpp
)
set(FIRMWARE_SRC ${FIRMWARE_SRC} $<TARGET_OBJECTS:board>)

if(BLUETOOTH)
  target_sources(board PRIVATE targets/common/arm/stm32/bluetooth_driver.cpp)
endif()

if(BIND_KEY)
  add_definitions(-DBIND_KEY)
  target_sources(board PRIVATE ${TARGET_SRC_DIR}/bind_button_driver.cpp)
endif()

if(USB_CHARGE_LED)
  add_definitions(-DUSB_CHARGE_LED)
endif()

if(USB_CHARGER)
  add_definitions(-DUSB_CHARGER)
  target_sources(board PRIVATE ${TARGET_SRC_DIR}/usb_charger_driver.cpp)
endif()

if(IMU)
  target_sources(board PRIVATE drivers/lsm6ds.cpp)
endif()

if(AUTOUPDATE)
  add_definitions(-DAUTOUPDATE)
endif()

if(USE_RTC_CLOCK)
  add_definitions(-DRTCLOCK)
endif()

if(RGBLEDS)
  add_definitions(-DRGBLEDS)
endif()

if(FLAVOUR STREQUAL gx12)
  target_sources(board PRIVATE
    drivers/pca95xx.cpp
    ${TARGET_SRC_DIR}/gx12/bsp_io.cpp
  )
  target_include_directories(board PRIVATE ${TARGET_SRC_DIR}/gx12)
endif()

if(FLAVOUR STREQUAL mt12)
  target_sources(board PRIVATE
    ${TARGET_SRC_DIR}/pwm_ch_driver.cpp
  )
  set(SRC ${SRC}
    gui/common/stdlcd/radio_servo_pwm.cpp
  )
endif()

set(SRC ${SRC}
  io/frsky_firmware_update.cpp
  io/multi_firmware_update.cpp
)

set(GUI_SRC ${GUI_SRC}
  model_input_edit.cpp
  model_mix_edit.cpp
  model_display.cpp
  radio_diagkeys.cpp
  radio_diaganas.cpp
  view_channels.cpp
  view_telemetry.cpp
  view_about.cpp
  bmp.cpp
)

if(PCBREV STREQUAL COMMANDO8)
  set(FIRMWARE_SRC
    ${FIRMWARE_SRC}
    targets/common/arm/stm32/sticks_pwm_driver.cpp)
endif()

set(FIRMWARE_SRC
  ${FIRMWARE_SRC}
  bootloader/loadboot.cpp
  )

set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} ${BITMAPS_TARGET})
