option(DISK_CACHE "Enable SD card disk cache" OFF)
option(SHUTDOWN_CONFIRMATION "Shutdown confirmation" OFF)
option(LCD_DUAL_BUFFER "Dual LCD Buffer" OFF)
option(PXX1 "PXX1 protocol support" ON)
option(PXX2 "PXX2 protocol support" OFF)

option(CROSSFIRE_TASK "Crossfire task" YES)
set(EXTERNAL_MODULE YES)
set(ALLOW_TRAINER_MULTI YES)
set(PWR_BUTTON "PRESS" CACHE STRING "Pwr button type (PRESS/SWITCH)")
set(STICKS "STANDARD" CACHE STRING "Sticks type (STANDARD/HORUS)")
set(CPU_TYPE STM32F4)
set(CPU_TYPE_FULL STM32F407xE)  # for size report
set(LINKER_SCRIPT targets/tbs/stm32f4_flash.ld)
set(RTC_BACKUP_RAM YES)
set(PPM_UNIT "PPM display unit" PERCENT_PREC0)
set(HAPTIC YES)
set(USB_CHARGER YES)
set(HAVE_NO_TRAINER NO)
set(RADIO_CALIBRATION_HALL YES)
set(MODEL_HAVE_NO_BITMAP YES)

set(STORAGE SDCARD)
set(STORAGE_FORMAT  YAML)
set(STORAGE_CONVERT NONE)

set(INTERNAL_MODULES CRSF CACHE STRING "Internal modules")
set(DEFAULT_INTERNAL_MODULE CROSSFIRE CACHE STRING "Default internal module")

set(BOOTLOADER NO CACHE BOOL "Default internal module" FORCE)

if (PCB STREQUAL TANGO)
  set(FLAVOUR tango)
  set(LCD_DEPTH 4)
  set(LUA_EXPORT lua_export_tango)
  set(XJT OFF CACHE BOOL "Disable XJT")
  add_definitions(-DRADIO_TANGO)
  add_definitions(-DSTM32F40_41xxx -DPCBTANGO)
elseif(PCB STREQUAL MAMBO)
  set(FLAVOUR mambo)
  set(LUA_EXPORT lua_export_mambo)
  add_definitions(-DHARDWARE_TRIMS)
  add_definitions(-DRADIO_MAMBO)
  add_definitions(-DSTM32F40_41xxx -DPCBMAMBO)
endif()

add_definitions(-DPCBTARANIS)
add_definitions(-DPCBX7)
add_definitions(-DRADIO_FAMILY_TBS)
add_definitions(-DEEPROM_VARIANT=0x8000)
add_definitions(-DLCD_VERTICAL_INVERT -DLCD_HORIZONTAL_INVERT)
add_definitions(-DPWR_BUTTON_${PWR_BUTTON})
add_definitions(-DSOFTWARE_VOLUME)
add_definitions(-DBATT_CRITICAL_SHUTDOWN)
add_definitions(-DMANUFACTURER_TBS)

if(HAVE_NO_TRAINER)
  add_definitions(-DRADIO_HAVE_NO_TRAINER)
endif()

if(MODEL_HAVE_NO_BITMAP)
  add_definitions(-DMODEL_HAVE_NO_BITMAP)
endif()

if(RADIO_CALIBRATION_HALL)
  add_definitions(-DRADIO_CALIBRATION_HALL)
endif()

if(DEBUG)
  if(PCB STREQUAL TANGO)
    set(XJT OFF CACHE BOOL "Disable XJT")
    set(MODULE_SIZE_STD OFF CACHE BOOL "Disable MODULE_SIZE_STD")
    set(PPM OFF CACHE BOOL "Disable PPM")
    set(DSM2 OFF CACHE BOOL "Disable DSM2")
    set(SBUS OFF CACHE BOOL "Disable SBUS")
    set(MULTIMODULE OFF CACHE BOOL "Disable MULTIMODULE")
  endif()
  set(AUX_SERIAL_DRIVER ../common/arm/stm32/aux_serial_driver.cpp)
endif()

set(FIRMWARE_TARGET_SRC
  ${FIRMWARE_TARGET_SRC}
  startup_stm32f40_41xxx.s
  )

if (CROSSFIRE_TASK)
  add_definitions(-DCROSSFIRE_TASK)
endif()

add_definitions(-DLIBCRSF_ENABLE_OPENTX_RELATED -DLIBCRSF_ENABLE_SD -DLIBCRSF_ENABLE_COMMAND -DLIBCRSF_ENABLE_PARAMETER)
add_definitions(-DDEFAULT_MODE=2)

set(GUI_DIR 128x64)
set(BITMAPS_DIR 128x64)
set(NAVIGATION_TYPE x7)
set(BITMAPS_TARGET 9x_bitmaps)
set(FONTS_TARGET 9x_fonts_1bit)
set(LCD_DRIVER lcd_driver_spi.cpp)
set(GVAR_SCREEN model_gvars.cpp)

add_definitions(-DESP_SERIAL)

if(INTERNAL_MODULE_CRSF)
  add_definitions(-DHARDWARE_INTERNAL_MODULE)
  add_definitions(-DINTERNAL_MODULE_CRSF)
endif()

set(TARGET_SRC
  ${TARGET_SRC}
  ../tbs/usbd_hid_agent.c
  )
  
set(FIRMWARE_TARGET_SRC
  ${FIRMWARE_TARGET_SRC}
  ../../io/crsf/crossfire.cpp
  ../../io/crsf/crsf.cpp
  ../../io/crsf/crsf_write.cpp
  ../../io/crsf/crsf_utilities.cpp
  ../../io/crsf/crc8.cpp
  usb_agent_driver.cpp
  )

if(USB_CHARGER)
  set(TARGET_SRC ${TARGET_SRC} usb_charger_driver.cpp)
  add_definitions(-DUSB_CHARGER)
endif()

set(HSE_VALUE 8000000)
set(SDCARD YES)
set(EEPROM SDCARD)
set(TARGET_DIR tbs)
set(PPM_LIMITS_SYMETRICAL YES)

set(RADIO_DEPENDENCIES
   ${BITMAPS_TARGET}
   radio_translations
  )

set(FIRMWARE_DEPENDENCIES datacopy)

if(SPLASH STREQUAL OFF)
  message(FATAL_ERROR "Option SPLASH=OFF not supported on TANGO2/MAMBO")
endif()

add_definitions(-DAUDIO -DVOICE -DRTCLOCK)

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
  ../screenshot.cpp
  )

if(DISK_CACHE)
  set(SRC ${SRC} disk_cache.cpp)
  add_definitions(-DDISK_CACHE)
endif()

set(TARGET_SRC
  ${TARGET_SRC}
  extmodule_driver.cpp
  ../common/arm/stm32/timers_driver.cpp
  ../common/arm/stm32/audio_dac_driver.cpp
  ../../hal/adc_driver.cpp
  ../common/arm/stm32/stm32_hal_adc.cpp
  ../common/arm/stm32/rotary_encoder_driver.cpp
  ../common/arm/stm32/mixer_scheduler_driver.cpp
  malloc.c
  )

set(FIRMWARE_TARGET_SRC
  ${FIRMWARE_TARGET_SRC}
  ${LCD_DRIVER}
  board.cpp
  backlight_driver.cpp
  crsf_tasks.cpp
  ../common/arm/stm32/pwr_driver.cpp
  ../common/arm/stm32/sdio_sd.c
  )

if (PCB STREQUAL TANGO)
  set(FIRMWARE_TARGET_SRC
    ${FIRMWARE_TARGET_SRC}
    led_driver.cpp
    )
endif()

if(LCD_DUAL_BUFFER)
  add_definitions(-DLCD_DUAL_BUFFER)
endif()

set(STM32LIB_SRC
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_ltdc.c
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_tim.c
  STM32F4xx_StdPeriph_Driver/src/stm32f4xx_flash.c
  )

if(EXTERNAL_MODULE)
  add_definitions(-DHARDWARE_EXTERNAL_MODULE)
endif()

set(FIRMWARE_DEPENDENCIES ${FIRMWARE_DEPENDENCIES} ${BITMAPS_TARGET})

MACRO(GET_GCC_INCLUDE_PATH gcc_include_path)
  if (WIN32)
    execute_process(COMMAND arm-none-eabi-gcc -v -x c -E NUL ERROR_VARIABLE _gcc_output OUTPUT_QUIET)
  else()
    execute_process(COMMAND arm-none-eabi-gcc -v -x c -E - INPUT_FILE /dev/null ERROR_VARIABLE _gcc_output OUTPUT_QUIET)
  endif()

  # Build an array of string from the GCC output
  string(REPLACE "\n" ";" _gcc_output "${_gcc_output}")

  set(_capture_include FALSE)
  set(_include_path "")

  # Go through the lines and capture between '"#include <...> search starts here:"' and 'End of search list.'
  foreach(_line ${_gcc_output})
    if(${_line} STREQUAL "End of search list.")
      set(_capture_include FALSE)
    endif()

    if(_capture_include)
      # Remove the leading and trailing empty characters
      string(REPLACE "\r" "" _line ${_line})
      string(SUBSTRING "${_line}" 1 -1 _line)
      set(_include_path "${_include_path}" "-I${_line}")
    endif()

    if(${_line} STREQUAL "#include <...> search starts here:")
      set(_capture_include TRUE)
    endif()
  endforeach()
  set(${gcc_include_path} ${_include_path})
ENDMACRO()

message("PCB = ${PCB}")

if(PYTHONINTERP_FOUND)
  GET_GCC_INCLUDE_PATH(arm_gcc_include_path)

  add_custom_command(
    OUTPUT ${PROJECT_BINARY_DIR}/radio/src/datacopy.cpp
    WORKING_DIRECTORY ${RADIO_DIRECTORY}/src
    COMMAND ${PYTHON_EXECUTABLE} ${RADIO_DIRECTORY}/util/generate_datacopy.py datastructs_private.h -DPCBTARANIS -DRADIO_${PCB} -DCPUARM -DSTM32F4 -DSTM32F40_41xxx -DBACKUP -DSIMU -I. -Itargets/tbs -Itargets/common/arm/stm32 -I${THIRDPARTY_DIR} -I${THIRDPARTY_DIR}/STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/STM32F4xx_StdPeriph_Driver/inc -I${THIRDPARTY_DIR}/STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/Device/ST/STM32F4xx/Include -I${THIRDPARTY_DIR}/STM32F4xx_DSP_StdPeriph_Lib_V1.8.0/Libraries/CMSIS/Include ${gcc_include_path} > ${PROJECT_BINARY_DIR}/radio/src/datacopy.cpp
    DEPENDS ${RADIO_DIRECTORY}/src/datastructs_private.h ${RADIO_DIRECTORY}/util/generate_datacopy.py
    )    
  add_custom_target(datacopy
    DEPENDS ${PROJECT_BINARY_DIR}/radio/src/datacopy.cpp
    )
endif()
