option(SHUTDOWN_CONFIRMATION "Shutdown confirmation" OFF)
option(LCD_DUAL_BUFFER "Dual LCD Buffer" OFF)
option(PXX1 "PXX1 protocol support" ON)
option(PXX2 "PXX2 protocol support" OFF)
option(AFHDS3 "AFHDS3 TX Module" OFF)
option(GHOST "Ghost TX Module" ON)
option(INTERNAL_MODULE_PPM "Support for PPM internal module" OFF)
option(AUTOUPDATE "Auto update internal chips from SD" OFF)
option(BIND_KEY "Enable bind button" OFF)

set(HARDWARE_EXTERNAL_MODULE YES)

add_definitions(-DPCBFRSKY)

if(PCB STREQUAL X9E)
  set(DEFAULT_INTERNAL_MODULE XJT_PXX1 CACHE STRING "Default internal module")
  set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
  set(STICKS "STANDARD" CACHE STRING "Sticks type (STANDARD/HORUS)")
  set(CPU_TYPE STM32F4)
  set(CPU_TYPE_FULL STM32F407xE)  # for size report
  set(LINKER_SCRIPT targets/taranis/stm32f4_flash.ld)
  add_definitions(-DSTM32F40_41xxx -DSTM32F407xx)
  set(HAPTIC YES)
  set(LUA_EXPORT lua_export_x9e)
  set(FLAVOUR x9e)
  add_definitions(-DPCBX9E -DPCBX9)
  add_definitions(-DEEPROM_VARIANT=0x8000)
  add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    top_lcd_driver.cpp
    startup_stm32f40_41xxx.s
    )
  set(AUX_SERIAL ON)
  if(STICKS STREQUAL HORUS)
    add_definitions(-DHORUS_STICKS)
  endif()
  set(GUI_DIR 212x64)
  set(BITMAPS_DIR 212x64)
  set(NAVIGATION_TYPE x9d)
  set(BITMAPS_TARGET taranis_bitmaps)
  set(FONTS_TARGET taranis_fonts)
  set(LCD_DRIVER lcd_driver_spi.cpp)
  set(GVAR_SCREEN model_gvars.cpp)
  set(STATUS_LEDS NO)
  set(BLUETOOTH YES)
  set(ROTARY_ENCODER YES)
  add_definitions(-DMANUFACTURER_FRSKY)
elseif(PCB STREQUAL X9D+)
  set(HAPTIC YES)
  set(LUA_EXPORT lua_export_x9d)
  add_definitions(-DPCBX9DP -DPCBX9)
  add_definitions(-DEEPROM_VARIANT=0)
  set(GUI_DIR 212x64)
  set(BITMAPS_DIR 212x64)
  set(NAVIGATION_TYPE x9d)
  set(BITMAPS_TARGET taranis_bitmaps)
  set(FONTS_TARGET taranis_fonts)
  set(LCD_DRIVER lcd_driver_spi.cpp)
  set(GVAR_SCREEN model_gvars.cpp)
  set(PCBREV 2014 CACHE STRING "PCB Revision")
  #set(AFHDS3 NO)
  add_definitions(-DPCBREV=${PCBREV})
  add_definitions(-DMANUFACTURER_FRSKY)
  if (${PCBREV} STREQUAL 2019)
    set(DEFAULT_INTERNAL_MODULE ISRM_PXX2 CACHE STRING "Default internal module")
    set(FLAVOUR x9d+2019)
    set(LUA_EXPORT lua_export_x9d+2019)
    set(CPU_TYPE STM32F4)
    set(CPU_TYPE_FULL STM32F407xE)  # for size report
    set(LINKER_SCRIPT targets/taranis/stm32f4_flash.ld)
    set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
    set(FIRMWARE_TARGET_SRC
      ${FIRMWARE_TARGET_SRC}
      startup_stm32f40_41xxx.s
      )
    add_definitions(-DRADIO_X9DP2019)
    add_definitions(-DSTM32F40_41xxx -DSTM32F407xx)
    add_definitions(-DSOFTWARE_VOLUME -DPWR_BUTTON_${PWR_BUTTON} -DHARDWARE_POWER_MANAGEMENT_UNIT)
    set(STATUS_LEDS YES)
    set(PXX2 ON)
    set(PXX_FREQUENCY "HIGH")
    set(BLUETOOTH YES)
    set(ROTARY_ENCODER YES)
    set(INTERNAL_MODULE_SERIAL YES)
  else()
    set(DEFAULT_INTERNAL_MODULE XJT_PXX1 CACHE STRING "Default internal module")
    set(FLAVOUR x9d+)
    set(CPU_TYPE STM32F2)
    set(CPU_TYPE_FULL STM32F205xE)  # for size report
    add_definitions(-DSTM32F205xx)
    set(LINKER_SCRIPT targets/taranis/stm32f2_flash.ld)
    set(AUX_SERIAL ON)
    set(STATUS_LEDS NO)
  endif()
elseif(PCB STREQUAL X9D)
  set(DEFAULT_INTERNAL_MODULE XJT_PXX1 CACHE STRING "Default internal module")
  set(FLAVOUR x9d)
  set(CPU_TYPE STM32F2)
  set(CPU_TYPE_FULL STM32F205xE)  # for size report
  add_definitions(-DSTM32F205xx)
  set(LINKER_SCRIPT targets/taranis/stm32f2_flash.ld)
  option(HAPTIC "Haptic support" OFF)
  set(LUA_EXPORT lua_export_x9d)
  add_definitions(-DPCBX9D -DPCBX9)
  add_definitions(-DEEPROM_VARIANT=0)
  set(GUI_DIR 212x64)
  set(BITMAPS_DIR 212x64)
  set(NAVIGATION_TYPE x9d)
  set(BITMAPS_TARGET taranis_bitmaps)
  set(FONTS_TARGET taranis_fonts)
  set(LCD_DRIVER lcd_driver_aspi.cpp)
  set(AUX_SERIAL ON)
  set(GVAR_SCREEN model_gvars.cpp)
  set(STATUS_LEDS NO)
  add_definitions(-DMANUFACTURER_FRSKY)
  #set(AFHDS3 NO)
elseif(PCB STREQUAL X7)
  set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
  set(HAPTIC YES)
  set(GUI_DIR 128x64)
  set(BITMAPS_DIR 128x64)
  set(BITMAPS_TARGET 9x_bitmaps)
  set(FONTS_TARGET 9x_fonts_1bit)
  set(LCD_DRIVER lcd_driver_spi.cpp)
  set(GVAR_SCREEN model_gvars.cpp)
  set(STATUS_LEDS YES)
  add_definitions(-DPCBX7 -DSOFTWARE_VOLUME)
  add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
  if(PCBREV STREQUAL T12)
    set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(FLAVOUR t12)
    set(NAVIGATION_TYPE 9x)
    set(CPU_TYPE STM32F2)
    set(CPU_TYPE_FULL STM32F205xE)  # for size report
    add_definitions(-DSTM32F205xx)
    set(LINKER_SCRIPT targets/taranis/stm32f2_flash.ld)
    set(LUA_EXPORT lua_export_t12)
    add_definitions(-DRADIO_T12)
    add_definitions(-DRADIO_FAMILY_JUMPER_T12)
    add_definitions(-DEEPROM_VARIANT=0x4001)
    add_definitions(-DMANUFACTURER_JUMPER)
  elseif(PCBREV STREQUAL TPRO)
    set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(FLAVOUR tpro)
    set(NAVIGATION_TYPE x7)
    set(CPU_TYPE STM32F2)
    set(ROTARY_ENCODER YES)
    set(CPU_TYPE_FULL STM32F205xE)  # for size report
    add_definitions(-DSTM32F205xx)
    set(LINKER_SCRIPT targets/taranis/stm32f2_flash.ld)
    set(LUA_EXPORT lua_export_tpro)
    add_definitions(-DFUNCTION_SWITCHES)
    add_definitions(-DRADIO_TPRO)
    add_definitions(-DEEPROM_VARIANT=0x4005)
    add_definitions(-DMANUFACTURER_JUMPER)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
    set(CLI ON CACHE BOOL "Enable CLI")
  elseif(PCBREV STREQUAL TLITE)
    set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(FLAVOUR tlite)
    set(NAVIGATION_TYPE 9x)
    set(CPU_TYPE STM32F2)
    set(CPU_TYPE_FULL STM32F205xE)  # for size report
    add_definitions(-DSTM32F205xx)
    set(LINKER_SCRIPT targets/taranis/stm32f2_flash.ld)
    set(LUA_EXPORT lua_export_tlite)
    add_definitions(-DRADIO_TLITE)
    add_definitions(-DRADIO_FAMILY_JUMPER_T12)
    add_definitions(-DEEPROM_VARIANT=0x4003)
    add_definitions(-DMANUFACTURER_JUMPER)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
    set(CLI ON CACHE BOOL "Enable CLI")
  elseif(PCBREV STREQUAL TX12)
    set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(FLAVOUR tx12)
    set(NAVIGATION_TYPE x7)
    set(CPU_TYPE STM32F2)
    set(CPU_TYPE_FULL STM32F205xE)  # for size report
    add_definitions(-DSTM32F205xx)
    set(LINKER_SCRIPT targets/taranis/stm32f2_flash.ld)
    set(ROTARY_ENCODER YES)
    set(LUA_EXPORT lua_export_tx12)
    set(BLUETOOTH NO)
    set(USB_CHARGER YES)
    add_definitions(-DRADIO_TX12)
    add_definitions(-DEEPROM_VARIANT=0x4002)
    add_definitions(-DMANUFACTURER_RADIOMASTER)
  elseif(PCBREV STREQUAL TX12MK2)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(FLAVOUR tx12mk2)
    set(NAVIGATION_TYPE x7)
    set(CPU_TYPE STM32F4)
    set(CPU_TYPE_FULL STM32F407xE)  # for size report
    add_definitions(-DSTM32F407xx)
    set(LINKER_SCRIPT targets/taranis/stm32f4_flash.ld)
    set(FIRMWARE_TARGET_SRC
            ${FIRMWARE_TARGET_SRC}
            startup_stm32f40_41xxx.s
            ../../thirdparty/STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c
            )
    add_definitions(-DSTM32F40_41xxx -DSTM32F407xx)
    set(ROTARY_ENCODER YES)
    set(LUA_EXPORT lua_export_tx12mk2)
    set(BLUETOOTH NO)
    set(USB_CHARGER YES)
    set(AUX_SERIAL ON)
    add_definitions(-DRADIO_TX12MK2)
    add_definitions(-DEEPROM_VARIANT=0x4008)
    add_definitions(-DMANUFACTURER_RADIOMASTER)
    set(PXX2 ON)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
    set(CLI ON CACHE BOOL "Enable CLI")
  elseif(PCBREV STREQUAL ZORRO)
    set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(FLAVOUR zorro)
    set(NAVIGATION_TYPE x7)
    set(CPU_TYPE STM32F4)
    set(CPU_TYPE_FULL STM32F407xE)  # for size report
    add_definitions(-DSTM32F407xx)
    set(LINKER_SCRIPT targets/taranis/stm32f4_flash.ld)
    set(FIRMWARE_TARGET_SRC
      ${FIRMWARE_TARGET_SRC}
      startup_stm32f40_41xxx.s
      ../../thirdparty/STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c
      )
    add_definitions(-DSTM32F40_41xxx -DSTM32F407xx)
    set(ROTARY_ENCODER YES)
    set(LUA_EXPORT lua_export_zorro)
    set(BLUETOOTH NO)
    set(USB_CHARGER YES)
    set(AUX_SERIAL ON)
    add_definitions(-DRADIO_ZORRO)
    add_definitions(-DEEPROM_VARIANT=0x4006)
    add_definitions(-DMANUFACTURER_RADIOMASTER)
    set(PXX2 ON)
    set(ENABLE_SERIAL_PASSTHROUGH ON CACHE BOOL "Enable serial passthrough")
    set(CLI ON CACHE BOOL "Enable CLI")
  elseif(PCBREV STREQUAL T8)
    set(DEFAULT_INTERNAL_MODULE MULTIMODULE CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(HARDWARE_EXTERNAL_MODULE NO)
    set(FLAVOUR t8)
    set(NAVIGATION_TYPE x7)
    set(CPU_TYPE STM32F2)
    set(CPU_TYPE_FULL STM32F205xE)  # for size report
    add_definitions(-DSTM32F205xx)
    set(LINKER_SCRIPT targets/taranis/stm32f2_flash.ld)
    set(ROTARY_ENCODER NO)
    set(LUA_EXPORT lua_export_t8)
    set(BLUETOOTH NO)
    if(RADIOMASTER_RELEASE)
      set(BIND_KEY YES)
    endif()
    add_definitions(-DRADIO_T8)
    add_definitions(-DEEPROM_VARIANT=0x4004)
    add_definitions(-DMANUFACTURER_RADIOMASTER)
  elseif(PCBREV STREQUAL ACCESS)
    set(DEFAULT_INTERNAL_MODULE ISRM_PXX2 CACHE STRING "Default internal module")
    set(INTERNAL_MODULE_SERIAL YES)
    set(FLAVOUR x7access)
    set(NAVIGATION_TYPE x7)
    set(CPU_TYPE STM32F4)
    set(CPU_TYPE_FULL STM32F407xE)  # for size report
    set(LINKER_SCRIPT targets/taranis/stm32f4_flash.ld)
    set(LUA_EXPORT lua_export_x7access)
    add_definitions(-DRADIO_X7ACCESS)
    add_definitions(-DEEPROM_VARIANT=0x4000)
    add_definitions(-DPCBREV=${PCBREV})
    add_definitions(-DPCBX7ACCESS)
    set(FIRMWARE_TARGET_SRC
      ${FIRMWARE_TARGET_SRC}
      startup_stm32f40_41xxx.s
      )
    add_definitions(-DSTM32F40_41xxx -DSTM32F407xx)
    add_definitions(-DHARDWARE_POWER_MANAGEMENT_UNIT)
    set(PXX2 ON)
    set(PXX_FREQUENCY "HIGH")
    set(BLUETOOTH YES)
    set(ROTARY_ENCODER YES)
    add_definitions(-DMANUFACTURER_FRSKY)
  else()
    set(DEFAULT_INTERNAL_MODULE XJT_PXX1 CACHE STRING "Default internal module")
    set(FLAVOUR x7)
    set(NAVIGATION_TYPE x7)
    set(CPU_TYPE STM32F2)
    set(CPU_TYPE_FULL STM32F205xE)  # for size report
    add_definitions(-DSTM32F205xx)
    set(LINKER_SCRIPT targets/taranis/stm32f2_flash.ld)
    set(LUA_EXPORT lua_export_x7)
    add_definitions(-DRADIO_X7)
    add_definitions(-DEEPROM_VARIANT=0x4000)
    set(BLUETOOTH YES)
    add_definitions(-DBLUETOOTH_PROBE)
    set(ROTARY_ENCODER YES)
    add_definitions(-DMANUFACTURER_FRSKY)
    if(DEFINED PCBREV_HARDCODED)
      # This *can* be used to prevent messing with PA14 (SWCLK), so stlink can be used
      # without interruptions.
      if(NOT ${PCBREV_HARDCODED} MATCHES "^[0-1]$")
        message(FATAL_ERROR "PCBREV_HARDCODED must be 0 or 1, not '${PCBREV_HARDCODED}'")
      endif()
      add_definitions(-DPCBREV_HARDCODED=${PCBREV_HARDCODED})
    endif()
  endif()
elseif(PCB STREQUAL X9LITE)
  set(DEFAULT_INTERNAL_MODULE ISRM_PXX2 CACHE STRING "Default internal module")
  set(INTERNAL_MODULE_SERIAL YES)
  set(PXX_FREQUENCY "HIGH")
  set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
  set(CPU_TYPE STM32F2)
  set(CPU_TYPE_FULL STM32F205xE)  # for size report
  add_definitions(-DSTM32F205xx)
  set(LINKER_SCRIPT targets/taranis/stm32f2_flash.ld)
  set(HAPTIC YES)
  set(LUA_EXPORT lua_export_x9lite)
  set(FLAVOUR x9lite)
  add_definitions(-DBOARD_NAME="x9lite")
  add_definitions(-DPCBX9LITE -DPCBX9 -DSOFTWARE_VOLUME -DHARDWARE_POWER_MANAGEMENT_UNIT)
  add_definitions(-DEEPROM_VARIANT=0x0800)
  add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
  set(PXX2 ON)
  set(GUI_DIR 128x64)
  set(BITMAPS_DIR 128x64)
  set(NAVIGATION_TYPE x7)
  set(BITMAPS_TARGET 9x_bitmaps)
  set(FONTS_TARGET 9x_fonts_1bit)
  set(LCD_DRIVER lcd_driver_spi.cpp)
  set(GVAR_SCREEN model_gvars.cpp)
  set(STATUS_LEDS YES)
  set(ROTARY_ENCODER YES)
  add_definitions(-DMANUFACTURER_FRSKY)
elseif(PCB STREQUAL X9LITES)
  set(DEFAULT_INTERNAL_MODULE ISRM_PXX2 CACHE STRING "Default internal module")
  set(INTERNAL_MODULE_SERIAL YES)
  set(PXX_FREQUENCY "HIGH")
  set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
  set(CPU_TYPE STM32F2)
  set(CPU_TYPE_FULL STM32F205xE)  # for size report
  add_definitions(-DSTM32F205xx)
  set(LINKER_SCRIPT targets/taranis/stm32f2_flash.ld)
  set(HAPTIC YES)
  set(LUA_EXPORT lua_export_x9lites)
  set(FLAVOUR x9lites)
  add_definitions(-DBOARD_NAME="x9lites")
  add_definitions(-DPCBX9LITES -DPCBX9LITE -DPCBX9 -DSOFTWARE_VOLUME -DHARDWARE_POWER_MANAGEMENT_UNIT)
  add_definitions(-DEEPROM_VARIANT=0x0801)
  add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
  set(PXX2 ON)
  set(GUI_DIR 128x64)
  set(BITMAPS_DIR 128x64)
  set(NAVIGATION_TYPE x7)
  set(BITMAPS_TARGET 9x_bitmaps)
  set(FONTS_TARGET 9x_fonts_1bit)
  set(LCD_DRIVER lcd_driver_spi.cpp)
  set(GVAR_SCREEN model_gvars.cpp)
  set(STATUS_LEDS YES)
  set(BLUETOOTH YES)
  set(ROTARY_ENCODER YES)
  add_definitions(-DMANUFACTURER_FRSKY)
elseif(PCB STREQUAL XLITE)
  set(DEFAULT_INTERNAL_MODULE XJT_PXX1 CACHE STRING "Default internal module")
  set(INTERNAL_MODULE_SERIAL YES)
  set(PXX_FREQUENCY "HIGH" CACHE STRING "PXX frequency (LOW / HIGH)") # always use HIGH except on some prototype boards
  set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
  set(CPU_TYPE STM32F2)
  set(CPU_TYPE_FULL STM32F205xE)  # for size report
  add_definitions(-DSTM32F205xx)
  set(LINKER_SCRIPT targets/taranis/stm32f2_flash.ld)
  set(HAPTIC YES)
  set(LUA_EXPORT lua_export_xlite)
  set(FLAVOUR xlite)
  add_definitions(-DPCBXLITE -DSOFTWARE_VOLUME -DSTICKS_PWM)
  add_definitions(-DEEPROM_VARIANT=0x2000)
  add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
  set(PXX2 ON)
  set(GUI_DIR 128x64)
  set(BITMAPS_DIR 128x64)
  set(NAVIGATION_TYPE xlite)
  set(BITMAPS_TARGET 9x_bitmaps)
  set(FONTS_TARGET 9x_fonts_1bit)
  set(LCD_DRIVER lcd_driver_spi.cpp)
  set(GVAR_SCREEN model_gvars.cpp)
  set(STATUS_LEDS YES)
  set(BLUETOOTH YES)
  add_definitions(-DMANUFACTURER_FRSKY)
elseif(PCB STREQUAL XLITES)
  set(DEFAULT_INTERNAL_MODULE ISRM_PXX2 CACHE STRING "Default internal module")
  set(INTERNAL_MODULE_SERIAL YES)
  set(PXX_FREQUENCY "HIGH")
  set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
  set(CPU_TYPE STM32F2)
  set(CPU_TYPE_FULL STM32F205xE)  # for size report
  add_definitions(-DSTM32F205xx)
  set(LINKER_SCRIPT targets/taranis/stm32f2_flash.ld)
  set(HAPTIC YES)
  set(LUA_EXPORT lua_export_xlites)
  set(FLAVOUR xlites)
  add_definitions(-DPCBXLITES -DPCBXLITE -DSOFTWARE_VOLUME -DSTICKS_PWM -DHARDWARE_POWER_MANAGEMENT_UNIT)
  add_definitions(-DEEPROM_VARIANT=0x1000)
  add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
  set(GUI_DIR 128x64)
  set(BITMAPS_DIR 128x64)
  set(NAVIGATION_TYPE xlite)
  set(BITMAPS_TARGET 9x_bitmaps)
  set(FONTS_TARGET 9x_fonts_1bit)
  set(LCD_DRIVER lcd_driver_spi.cpp)
  set(GVAR_SCREEN model_gvars.cpp)
  set(STATUS_LEDS YES)
  set(PXX2 ON)
  set(BLUETOOTH YES)
  set(IMU YES)
  add_definitions(-DMANUFACTURER_FRSKY)
endif()

if(NOT PCB STREQUAL PCBXLITE)
  add_definitions(-DHARDWARE_TRAINER_JACK)
endif()

if(INTERNAL_MODULE_SERIAL)

  if("${PCBREV}" STREQUAL 2019 OR "${PCBREV}" STREQUAL ACCESS)
    # defines existing internal modules
    set(INTERNAL_MODULES PXX2 CACHE STRING "Internal modules")
  else()
    # defines existing internal modules
    set(INTERNAL_MODULES PXX1;PXX2;MULTI;CRSF CACHE STRING "Internal modules")
  endif()

  add_definitions(-DINTERNAL_MODULE_SERIAL)
  if(PXX_FREQUENCY STREQUAL HIGH)
    add_definitions(-DPXX_FREQUENCY_HIGH)
  endif()
  set(FIRMWARE_SRC
    ${FIRMWARE_SRC}
    targets/common/arm/stm32/intmodule_serial_driver.cpp
    targets/common/arm/stm32/stm32_usart_driver.cpp
    )

else()

  # defines existing internal modules
  set(INTERNAL_MODULES PXX1)

  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    intmodule_pulses_driver.cpp
    )
endif()

if(HARDWARE_EXTERNAL_MODULE)
  set(FIRMWARE_SRC
    ${FIRMWARE_SRC}
    targets/common/arm/stm32/extmodule_driver.cpp
    targets/common/arm/stm32/stm32_pulse_driver.cpp
    targets/common/arm/stm32/extmodule_serial_driver.cpp
    targets/common/arm/stm32/stm32_usart_driver.cpp
    )
endif()

if(ROTARY_ENCODER)
  set(FIRMWARE_SRC
    ${FIRMWARE_SRC}
    targets/common/arm/stm32/rotary_encoder_driver.cpp
    )
endif()

if(BIND_KEY)
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    bind_button_driver.cpp
    )
  add_definitions(-DBIND_KEY)
endif()

if(USB_CHARGER)
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC} usb_charger_driver.cpp)
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
  if(NOT PCB STREQUAL X9E)
    # Bluetooth on X9E is compatible with almost nothing
    add_definitions(-DHARDWARE_TRAINER_BLUETOOTH)
  endif()
endif()

if(AUTOUPDATE)
  add_definitions(-DAUTOUPDATE)
endif()

set(HSE_VALUE 12000000)
set(SDCARD YES)
if(YAML_STORAGE)
  set(STORAGE SDCARD)
  set(STORAGE_FORMAT  YAML)
  set(STORAGE_CONVERT EEPROM_RLC)
else()
  set(STORAGE EEPROM)
  set(STORAGE_FORMAT RLC)
endif()
set(TARGET_DIR taranis)
set(PPM_LIMITS_SYMETRICAL YES)

if(SPLASH STREQUAL OFF)
  message(FATAL_ERROR "Option SPLASH=OFF not supported on Taranis")
endif()

add_definitions(-DPCBTARANIS)
add_definitions(-DAUDIO -DVOICE -DRTCLOCK)

set(SRC
  ${SRC}
  io/frsky_firmware_update.cpp
  io/multi_firmware_update.cpp
  )

set(GUI_SRC
  ${GUI_SRC}
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

if(STATUS_LEDS)
  set(LED_DRIVER led_driver.cpp)
endif()

set(FIRMWARE_TARGET_SRC
  ${FIRMWARE_TARGET_SRC}
  ${LED_DRIVER}
  ${LCD_DRIVER}
  board.cpp
  volume_i2c.cpp
  eeprom_driver.cpp
  backlight_driver.cpp
  trainer_driver.cpp
  )

if(PCB STREQUAL XLITE OR PCB STREQUAL XLITES)
  set(FIRMWARE_SRC
    ${FIRMWARE_SRC}
    targets/common/arm/stm32/sticks_pwm_driver.cpp)
endif()

set(FIRMWARE_SRC
  ${FIRMWARE_SRC}
  hal/adc_driver.cpp
  targets/common/arm/loadboot.cpp
  targets/common/arm/stm32/stm32_hal_adc.cpp
  targets/common/arm/stm32/heartbeat_driver.cpp
  targets/common/arm/stm32/mixer_scheduler_driver.cpp
  targets/common/arm/stm32/timers_driver.cpp
  targets/common/arm/stm32/audio_dac_driver.cpp
  targets/common/arm/stm32/pwr_driver.cpp
  )

if(LCD_DUAL_BUFFER)
  add_definitions(-DLCD_DUAL_BUFFER)
endif()

set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} ${BITMAPS_TARGET})
