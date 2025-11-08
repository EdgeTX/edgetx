include(CMakeForceCompiler)
include(Bitmaps)

set(PCB_TYPES X9LITE X9LITES X7 XLITE XLITES X9D X9D+ X9E X10 X12S PL18 T15PRO ST16 PA01 TX15)
set(RADIO_LANGUAGES CN CZ DA DE EN ES FI FR HE HU IT JP KO NL PL PT RU SE SK TW UA)
set(TTS_LANGUAGES CN CZ DA DE EN ES FI FR HE HU IT JP KO NL PL PT RU SE SK TW UA)

set(PCB "X9D+" CACHE STRING "Radio type, one of: ${PCB_TYPES}")
set_property(CACHE PCB PROPERTY STRINGS ${PCB_TYPES})
set(TRANSLATIONS "EN" CACHE STRING "Radio language, one of: ${RADIO_LANGUAGES}")
set_property(CACHE TRANSLATIONS PROPERTY STRINGS ${RADIO_LANGUAGES})
set(DEFAULT_MODE "" CACHE STRING "Default sticks mode")
set(POPUP_LEVEL 2 CACHE STRING "Popup level")

option(HELI "Heli menu" ON)
option(FLIGHT_MODES "Flight Modes" ON)
option(CURVES "Curves" ON)
option(GVARS "Global variables" ON)
option(GUI "GUI enabled" ON)
option(PPM_CENTER_ADJUSTABLE "PPM center adjustable" ON)
option(PPM_LIMITS_SYMETRICAL "PPM limits symetrical" OFF)
option(OVERRIDE_CHANNEL_FUNCTION "OverrideChannel function available" ON)
option(DANGEROUS_MODULE_FUNCTIONS "Dangerous module functions (RangeCheck / Bind / Module OFF, etc.) available" OFF)
option(FAI "Competition mode (no telemetry)" OFF)
option(AUTOSOURCE "Automatic source detection in menus" ON)
option(AUTOSWITCH "Automatic switch detection in menus" ON)
option(SEMIHOSTING "Enable debugger semihosting" OFF)
option(JITTER_MEASURE "Enable ADC jitter measurement" OFF)
option(WATCHDOG "Enable hardware Watchdog" ON)
option(ASTERISK "Enable asterisk icon (test only firmware)" OFF)
option(LUA "Enable LUA support" ON)
option(SIMU_DISKIO "Enable disk IO simulation in simulator. Simulator will use FatFs module and simulated IO layer that  uses \"./sdcard.image\" file as image of SD card. This file must contain whole SD card from first to last sector" OFF)
option(SIMU_LUA_COMPILER "Pre-compile and save Lua scripts in simulator." ON)
option(FAS_PROTOTYPE "Support of old FAS prototypes (different resistors)" OFF)
option(RAS "RAS (SWR) enabled" ON)
option(TEMPLATES "Model templates menu" OFF)
option(TRACE_SIMPGMSPACE "Turn on traces in simpgmspace.cpp" ON)
option(TRACE_LUA_INTERNALS "Turn on traces for Lua internals" OFF)
option(TRACE_AUDIO "Traces audio enabled" OFF)
option(DEBUG_SEGGER_RTT "Debug output to Segger RTT" OFF)
option(DEBUG_WINDOWS "Turn on windows traces" OFF)
option(DEBUG_YAML "Turn on YAML traces" OFF)
option(DEBUG_LABELS "Turn on Labels traces" OFF)
option(NANO "Use nano newlib and binalloc" ON)
option(TEST_BUILD_WARNING "Warn this is a test build" OFF)
option(MODULE_PROTOCOL_FCC "Add support for FCC modules" ON)
option(MODULE_PROTOCOL_LBT "Add support for EU/LBT modules" ON)
option(MODULE_PROTOCOL_D8 "Add support for D8 modules" ON)
option(FRSKY_RELEASE "Used to build FrSky released firmware" OFF)
option(TBS_RELEASE "Used to build TBS released firmware" OFF)
option(IMRC_RELEASE "Used to build IMRC released firmware" OFF)
option(HARDWARE_TRAINER_MULTI "Allow multi trainer" OFF)
option(BOOTLOADER "Include Bootloader" ON)
option(FWDRIVE "Attach also firmware drive with USB" OFF)
option(DISABLE_MCUCHECK "Disable MCU check at start" OFF)
option(LUA_MIXER "Enable LUA mixer/model scripts support" ON)
if(PCB STREQUAL X9D+ AND PCBREV STREQUAL 2019)
  option(USBJ_EX "Enable USB Joystick Extension" OFF)
else()
  option(USBJ_EX "Enable USB Joystick Extension" ON)
endif()
option(AUDIO "Enable audio" ON)

if(NATIVE_BUILD)
  option(ALL_LANGUAGES "Enable runtime language selection" ON)
else()
  option(ALL_LANGUAGES "Enable runtime language selection" OFF)
endif()

if(PCB STREQUAL X7 AND PCBREV STREQUAL LR3PRO)
  option(POWER_LED_BLUE  "Override Power LED to be normally Blue" ON)
else()
  option(POWER_LED_BLUE  "Override Power LED to be normally Blue" OFF)
endif()
option(VCONTROLS "Enable virtual controls support" OFF)

# since we reset all default CMAKE compiler flags for firmware builds, provide an alternate way for user to specify additional flags.
set(FIRMWARE_C_FLAGS "" CACHE STRING "Additional flags for firmware target c compiler (note: all CMAKE_C_FLAGS[_*] are ignored for firmware/bootloader).")
set(FIRMWARE_C_FLAGS_DEBUG "-g" CACHE STRING "Additional flags for firmware target (Debug config) c compiler (note: CMAKE_C_FLAGS_DEBUG is ignored for firmware/bootloader).")
set(FIRMWARE_CXX_FLAGS "" CACHE STRING "Additional flags for firmware target c++ compiler (note: all CMAKE_CXX_FLAGS[_*] are ignored for firmware/bootloader).")
set(FIRMWARE_CXX_FLAGS_DEBUG "-g" CACHE STRING "Additional flags for firmware target (Debug config) c++ compiler (note: CMAKE_CXX_FLAGS_DEBUG is ignored for firmware/bootloader).")

set(THIRDPARTY_DIR thirdparty)
set(LUA_DIR ${THIRDPARTY_DIR}/Lua/src)
set(RTOS_DIR ${THIRDPARTY_DIR}/FreeRTOS)
set(FATFS_DIR ${THIRDPARTY_DIR}/FatFs)
set(RADIO_BIN_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

configure_file(stamp.h.in stamp.h @ONLY)

if(PCB STREQUAL X12S OR PCB STREQUAL X10)
  include(targets/horus/CMakeLists.txt)
elseif(PCB STREQUAL PL18)
  include(targets/pl18/CMakeLists.txt)
elseif(PCB STREQUAL T15PRO)
  include(targets/t15pro/CMakeLists.txt)
elseif(PCB STREQUAL ST16)
  include(targets/st16/CMakeLists.txt)
elseif(PCB STREQUAL PA01)
  include(targets/pa01/CMakeLists.txt)
elseif(PCB STREQUAL TX15)
  include(targets/tx15/CMakeLists.txt)
elseif(PCB STREQUAL STM32H7S78_DK)
  include(targets/stm32h7s78-dk/CMakeLists.txt)
elseif(PCB STREQUAL X9E OR PCB STREQUAL X9D+ OR PCB STREQUAL X9D OR PCB STREQUAL X7 OR PCB STREQUAL X9LITE OR PCB STREQUAL X9LITES OR PCB STREQUAL XLITE OR PCB STREQUAL XLITES)
  include(targets/taranis/CMakeLists.txt)
else()
  message(FATAL_ERROR "Unknown PCB '${PCB}'")
endif()

set(HW_DESC_JSON ${FLAVOUR}.json)
AddHardwareDefTarget(${HW_DESC_JSON})

# enable generating JSON definition separately for debugging
add_custom_target(hardware_defs DEPENDS ${HW_DESC_JSON} ${HW_DESC_JSON}.h)

include(hal/CMakeLists.txt)

add_subdirectory(bitmaps)
add_subdirectory(fonts)

if(CPU_TYPE STREQUAL STM32H7 OR CPU_TYPE STREQUAL STM32H7RS)
  include(targets/common/arm/stm32/h7/CMakeLists.txt)
endif()

if(CPU_TYPE STREQUAL STM32F4)
  include(targets/common/arm/stm32/f4/CMakeLists.txt)
endif()

if(CPU_TYPE STREQUAL STM32F2)
  include(targets/common/arm/stm32/f2/CMakeLists.txt)
endif()

if(CPU_FAMILY STREQUAL STM32)
  include(targets/common/arm/stm32/CMakeLists.txt)
  include(boards/generic_stm32/CMakeLists.txt)
endif()

if(ARCH STREQUAL ARM)
  include(targets/common/arm/CMakeLists.txt)
endif()

include_directories(targets/${TARGET_DIR} ${THIRDPARTY_DIR})

if(NATIVE_BUILD)
  include_directories(targets/simu)
endif()

foreach(LANGUAGE ${RADIO_LANGUAGES})
  if(TRANSLATIONS STREQUAL ${LANGUAGE})
    add_definitions(-DTRANSLATIONS_${LANGUAGE} -DTRANSLATIONS="${LANGUAGE}")
  endif()
endforeach()

set(RTL_LANGUAGES HE)
if(TRANSLATIONS IN_LIST RTL_LANGUAGES)
  set(TRANSLATION_IS_RTL 1)
else()
  set(TRANSLATION_IS_RTL 0)
endif()
add_definitions(-DTRANSLATION_IS_RTL=${TRANSLATION_IS_RTL})

set(SRC ${SRC} debug.cpp)

add_definitions(-DFLAVOUR="${FLAVOUR}")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/bitmaps/${BITMAPS_DIR})

include(gui/CMakeLists.txt)

# TODO: move to storage/
set(SRC ${SRC} storage/storage_common.cpp)
set(SRC ${SRC} storage/sdcard_common.cpp)
set(SRC ${SRC} storage/sdcard_yaml.cpp)
include(storage/yaml/CMakeLists.txt)
if(STORAGE_MODELSLIST)
  set(SRC ${SRC} storage/modelslist.cpp)
  add_definitions(-DSTORAGE_MODELSLIST)
endif()

if(RTC_BACKUP_RAM)
  add_definitions(-DRTC_BACKUP_RAM)

  GenerateDataCopy(datastructs_private.h datacopy.inc)

  set(SRC ${SRC}
    storage/rlc.cpp
    storage/rtc_backup.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/datacopy.inc
    )

  # Add custom target for debugging
  add_custom_target(datacopy DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/datacopy.inc)
endif()

if (LUA_MIXER AND NOT LUA)
  message(FATAL_ERROR "LUA_MIXER requires LUA!!")
endif()

if(LUA)
  include(lua/CMakeLists.txt)
endif()

if(HELI)
  add_definitions(-DHELI)
endif()

if(FLIGHT_MODES)
  add_definitions(-DFLIGHT_MODES)
endif()

if(CURVES)
  add_definitions(-DCURVES)
  set(SRC ${SRC} curves.cpp)
endif()

if(GVARS)
  add_definitions(-DGVARS)
  set(SRC ${SRC} gvars.cpp)
endif()

if(FAI STREQUAL YES)
  add_definitions(-DFAI)
elseif(FAI STREQUAL CHOICE)
  add_definitions(-DFAI_CHOICE)
endif()

if(NOT RAS)
  add_definitions(-DNO_RAS)
endif()

if(FAS_PROTOTYPE)
  add_definitions(-DFAS_PROTOTYPE)
endif()

if(AUTOSOURCE)
  add_definitions(-DAUTOSOURCE)
endif()

if(AUTOSWITCH)
  add_definitions(-DAUTOSWITCH)
endif()

if(JITTER_MEASURE)
  add_definitions(-DJITTER_MEASURE)
endif()

if(ASTERISK)
  add_definitions(-DASTERISK)
endif()

if(WATCHDOG AND NOT NATIVE_BUILD)
  add_definitions(-DUSE_WATCHDOG)
endif()

if(SIMU_DISKIO)
  add_definitions(-DSIMU_DISKIO)
endif()

set(SRC ${SRC} sdcard.cpp rtc.cpp logs.cpp lib_file.cpp)

if(BLUETOOTH)
  add_definitions(-DBLUETOOTH)
  set(SRC ${SRC} bluetooth.cpp)
endif()

if(SHUTDOWN_CONFIRMATION)
  add_definitions(-DSHUTDOWN_CONFIRMATION)
endif()

if(PPM_CENTER_ADJUSTABLE)
  add_definitions(-DPPM_CENTER_ADJUSTABLE)
endif()

if(PPM_LIMITS_SYMETRICAL)
  add_definitions(-DPPM_LIMITS_SYMETRICAL)
endif()

if(OVERRIDE_CHANNEL_FUNCTION)
  add_definitions(-DOVERRIDE_CHANNEL_FUNCTION)
endif()

if(DANGEROUS_MODULE_FUNCTIONS)
  add_definitions(-DDANGEROUS_MODULE_FUNCTIONS)
endif()

if(NOT DEFAULT_MODE STREQUAL "")
  add_definitions(-DDEFAULT_MODE=${DEFAULT_MODE})
endif()

if(TRACE_SIMPGMSPACE)
  add_definitions(-DTRACE_SIMPGMSPACE)
endif()

if(TRACE_LUA_INTERNALS)
  add_definitions(-DTRACE_LUA_INTERNALS_ENABLED)
endif()

if(TRACE_AUDIO)
  add_definitions(-DTRACE_AUDIO)
  set(DEBUG ON)
  set(DEBUG_TRACE_BUFFER ON)
endif()

if(DEBUG STREQUAL RTT)
  set(DEBUG ON)
  set(DEBUG_SEGGER_RTT ON)
endif()

if(DEBUG STREQUAL RTT_NO_USB)
  set(DEBUG ON)
  set(DEBUG_SEGGER_RTT ON)
  add_definitions(-DDEBUG_DISABLE_USB)
endif()

if(DEBUG STREQUAL RTT_SYSVIEW)
  set(DEBUG ON)
  set(DEBUG_SEGGER_RTT ON)
  set(DEBUG_SEGGER_SYSVIEW ON)
endif()

if(DEBUG_SEGGER_RTT AND NOT NATIVE_BUILD)
  add_definitions(-DDEBUG_SEGGER_RTT -DSEGGER_RTT_BUFFER_SECTION=".ram")
  include_directories(${THIRDPARTY_DIR}/Segger/Config)
  include_directories(${THIRDPARTY_DIR}/Segger/Sample)
  include_directories(${THIRDPARTY_DIR}/Segger/SEGGER)
  set(SRC ${SRC} ${THIRDPARTY_DIR}/Segger/SEGGER/SEGGER_RTT.c)
  set(SRC ${SRC} ${THIRDPARTY_DIR}/Segger/SEGGER/SEGGER_RTT_ASM_ARMv7M.S)
  set(SRC ${SRC} ${THIRDPARTY_DIR}/Segger/SEGGER/SEGGER_RTT_printf.c)
endif()

if(DEBUG_SEGGER_SYSVIEW AND NOT NATIVE_BUILD)
  add_definitions(-DDEBUG_SEGGER_SYSVIEW)
  include_directories(${THIRDPARTY_DIR}/Segger/Sample/FreeRTOSV11)
  set(SRC ${SRC} ${THIRDPARTY_DIR}/Segger/SEGGER/SEGGER_SYSVIEW.c)
  set(SRC ${SRC} ${THIRDPARTY_DIR}/Segger/Sample/FreeRTOSV11/SEGGER_SYSVIEW_FreeRTOS.c)
  set(SRC ${SRC} ${THIRDPARTY_DIR}/Segger/Sample/FreeRTOSV11/Config/Cortex-M/SEGGER_SYSVIEW_Config_FreeRTOS.c)
endif()

if(DEBUG_WINDOWS)
  add_definitions(-DDEBUG_WINDOWS)
endif()

if(DEBUG_YAML)
  add_definitions(-DDEBUG_YAML)
endif()

if(DEBUG_LABELS)
  add_definitions(-DDEBUG_LABELS)
endif()

if(IMU_LSM6DS33)
    add_definitions(-DIMU_LSM6DS33)
endif()

if(TEST_BUILD_WARNING)
  add_definitions(-DTEST_BUILD_WARNING)
endif()

if(MODULE_PROTOCOL_FCC)
  add_definitions(-DMODULE_PROTOCOL_FCC)
endif()

if(MODULE_PROTOCOL_LBT)
  add_definitions(-DMODULE_PROTOCOL_LBT)
endif()

if(MODULE_PROTOCOL_D8)
  add_definitions(-DMODULE_PROTOCOL_D8)
endif()

if(JUMPER_RELEASE)
  add_definitions(-DJUMPER_RELEASE)
endif()

if(RADIOMASTER_RELEASE)
  add_definitions(-DRADIOMASTER_RELEASE)
endif()

if(RADIOMASTER_RTF_RELEASE)
  add_definitions(-DRADIOMASTER_RTF_RELEASE)
endif()

if(TBS_RELEASE)
  add_definitions(-DTBS_RELEASE)
endif()

if(IMRC_RELEASE)
  add_definitions(-DIMRC_RELEASE)
endif()

if(FRSKY_RELEASE)
  add_definitions(-DFRSKY_RELEASE)
  set(POPUP_LEVEL 3)
endif()

if(IFLIGHT_RELEASE)
  add_definitions(-DIFLIGHT_RELEASE)
endif()

if(BETAFPV_RELEASE)
  add_definitions(-DBETAFPV_RELEASE)
endif()

if(HARDWARE_TRAINER_MULTI)
  add_definitions(-DHARDWARE_TRAINER_MULTI)
endif()

add_definitions(-DPOPUP_LEVEL=${POPUP_LEVEL})

if(INTERNAL_MODULE_MULTI OR INTERNAL_MODULE_CRSF)
  set(DEFAULT_TEMPLATE_SETUP 21 CACHE STRING "")
else()
  set(DEFAULT_TEMPLATE_SETUP 0 CACHE STRING "")
endif()
add_definitions(-DDEFAULT_TEMPLATE_SETUP=${DEFAULT_TEMPLATE_SETUP})

if(FWDRIVE)
  add_definitions(-DFWDRIVE)
endif()

if(DISABLE_MCUCHECK)
  add_definitions(-DDISABLE_MCUCHECK)
endif()

if(VCONTROLS)
  add_definitions(-DVCONTROLS)
endif()

set(SRC
  ${SRC}
  edgetx.cpp
  datastructs_model.cpp
  datastructs_radio.cpp
  functions.cpp
  strhelpers.cpp
  switches.cpp
  analogs.cpp
  mixes.cpp
  mixer.cpp
  mixer_scheduler.cpp
  stamp.cpp
  timers.cpp
  trainer.cpp
  model_init.cpp
  serial.cpp
  audio.cpp
  model_audio.cpp
  sbus.cpp
  input_mapping.cpp
  inactivity_timer.cpp
  tasks/mixer_task.cpp
  )

if(GUI)
  add_definitions(-DGUI)
  set(SRC
    ${SRC}
    keys.cpp
    maths.cpp
    translations/translations.cpp
    cfn_sort.cpp
    gui/gui_common.cpp
    )
  foreach(FILE ${GUI_SRC})
    set(SRC ${SRC} gui/${GUI_DIR}/${FILE})
  endforeach()
endif()

if(AUDIO)
  add_definitions(-DAUDIO)
endif()

if(ALL_LANGUAGES)
  add_definitions(-DALL_LANGS)
  set(SRC
    ${SRC}
    translations/sim/lang_cn.cpp
    translations/sim/lang_cz.cpp
    translations/sim/lang_da.cpp
    translations/sim/lang_de.cpp
    translations/sim/lang_en.cpp
    translations/sim/lang_es.cpp
    translations/sim/lang_fi.cpp
    translations/sim/lang_fr.cpp
    translations/sim/lang_he.cpp
    translations/sim/lang_it.cpp
    translations/sim/lang_jp.cpp
    translations/sim/lang_ko.cpp
    translations/sim/lang_nl.cpp
    translations/sim/lang_pl.cpp
    translations/sim/lang_pt.cpp
    translations/sim/lang_ru.cpp
    translations/sim/lang_se.cpp
    translations/sim/lang_tw.cpp
    translations/sim/lang_ua.cpp
  )
endif()
if(VCONTROLS)
  set(SRC ${SRC} vcontrols.cpp)
endif()

if(USBJ_EX)
  add_definitions(-DUSBJ_EX)
  set(SRC ${SRC}
    usb_joystick.cpp
  )
endif()

if(POWER_LED_BLUE)
  add_definitions(-DPOWER_LED_BLUE)
endif()

foreach(FILE ${TARGET_SRC})
  set(SRC targets/${TARGET_DIR}/${FILE} ${SRC})
endforeach()

foreach(FILE ${FIRMWARE_TARGET_SRC})
  set(FIRMWARE_SRC ${FIRMWARE_SRC} targets/${TARGET_DIR}/${FILE})
endforeach()

foreach(FILE ${PULSES_SRC})
  set(SRC ${SRC} pulses/${FILE})
endforeach()

add_definitions(-DCORRECT_NEGATIVE_SHIFTS)

if(NOT MSVC)
  set(WARNING_FLAGS "${WARNING_FLAGS} -Wall -Wno-address-of-packed-member -Wno-strict-aliasing -Wformat -Wreturn-type -Wunused -Wuninitialized -Wunknown-pragmas -Wno-switch -Wtype-limits")
  if(WARNINGS_AS_ERRORS)
    set(WARNING_FLAGS "${WARNING_FLAGS} -Werror")
  endif(WARNINGS_AS_ERRORS)
else()
  add_definitions(-DHAVE_STRUCT_TIMESPEC)  # this is for pthread.h
  if(WARNINGS_AS_ERRORS)
    set(WARNING_FLAGS "${WARNING_FLAGS} /WX")
  endif(WARNINGS_AS_ERRORS)
endif()

if(NATIVE_BUILD)
  set(RADIOLIB_NATIVE_SRC ${SRC})

  # Pack all radio sources into an object lib
  add_library(radiolib_native OBJECT EXCLUDE_FROM_ALL
    ${RADIOLIB_NATIVE_SRC}
  )

  # Add SIMU and NATIVE_THREAD here PUBLIC, so it can be imported by other targets
  # via INTERFACE_COMPILE_OPTIONS
  target_compile_options(radiolib_native PUBLIC -DSIMU -DNATIVE_THREADS)
  target_sources(radiolib_native PUBLIC
    os/async_native.cpp
    os/sleep_native.cpp
    os/task_native.cpp
    os/time_native.cpp
    os/timer_native.cpp
  )
  add_dependencies(radiolib_native ${RADIO_DEPENDENCIES})
  set_property(TARGET radiolib_native PROPERTY POSITION_INDEPENDENT_CODE ON)

  add_subdirectory(targets/simu)
  add_subdirectory(tests)
endif()

set(SRC ${SRC} ${FIRMWARE_SRC})

##### firmware target #####

if(NATIVE_BUILD)
  message(STATUS "firmware target disabled")
  return()
endif()

# Silence CMake 2.8+ warning about unused variable (ie. if specified when not relevant)
set(ignore_USE_UNSUPPORTED_TOOLCHAIN "${USE_UNSUPPORTED_TOOLCHAIN}")

if(NOT USE_UNSUPPORTED_TOOLCHAIN AND NOT CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "14.2.1")
  message("CPP Compiler: ${CMAKE_CXX_COMPILER}, version ${CMAKE_CXX_COMPILER_VERSION}")
  message(FATAL_ERROR "Only Arm GNU Toolchain version 14.2.rel1 is supported!!")
endif()

set(CMAKE_C_FLAGS "${FIRMWARE_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${FIRMWARE_C_FLAGS_DEBUG}")
set(CMAKE_C_FLAGS_RELEASE "${FIRMWARE_C_FLAGS}")

set(CMAKE_CXX_FLAGS "${FIRMWARE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${FIRMWARE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_FLAGS_RELEASE "${FIRMWARE_CXX_FLAGS}")

find_program(BASH bash
  HINTS "c:/cygwin/bin/" "c:/msys/bin/"
  DOC "bash shell for firmware size report (eg. c:/cygwin/bin/bash.exe on Windows)."
  )

set(OPT "s" CACHE STRING "Optimization mode (0/1/2/s)")

set(COMMON_FLAGS "-mcpu=${MCU} -mthumb -fomit-frame-pointer -fverbose-asm -Wa,-ahlms=firmware.lst")
set(COMMON_FLAGS "${COMMON_FLAGS} -gdwarf-2 -fno-exceptions -fdata-sections -ffunction-sections")
set(COMMON_FLAGS "${COMMON_FLAGS} -DHSE_VALUE=${HSE_VALUE}")
set(COMMON_FLAGS "${COMMON_FLAGS} -O${OPT} ${FPU_FLAGS}")
set(COMMON_FLAGS "${COMMON_FLAGS} ${WARNING_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS} -Wimplicit")

# "-Wno-register" because ${THIRDPARTY_DIR}/STM32F2xx_HAL_Driver uses invalid C++17 storage class specifier
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS} -fno-rtti -Wno-register")

if(SDRAM)
  # Target with SDRAM do not need a custom allocator for now
  # as the custom allocator has alignement issues
  message("Target has SDRAM, do not use BIN_ALLOCATOR")
else()
  # Nano's malloc does work well with lua, use our own
  add_definitions(-DUSE_CUSTOM_ALLOCATOR)
  set(SRC ${SRC} lua/custom_allocator.cpp)
endif()

# Bootloader
if(BOOTLOADER)
  add_subdirectory(bootloader)
  include_directories(${CMAKE_CURRENT_BINARY_DIR}/bootloader)
  set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} bootloader)
endif()

add_executable(firmware ${SRC})
link_libraries(firmware -lstdc++)
add_dependencies(firmware ${RADIO_DEPENDENCIES})
set_target_properties(firmware PROPERTIES EXCLUDE_FROM_ALL TRUE)

if(ARCH STREQUAL ARM)
  target_link_libraries(firmware PRIVATE cmsis)
endif()

target_compile_definitions(board PUBLIC FREE_RTOS)
target_compile_definitions(board_lib PUBLIC FREE_RTOS)
target_compile_definitions(firmware PUBLIC FREE_RTOS)
target_compile_definitions(stm32_drivers_w_dbg_fw PUBLIC FREE_RTOS)

target_sources(firmware PUBLIC
  os/async_freertos.cpp
  os/sleep_freertos.cpp
  os/task_freertos.cpp
  os/time_freertos.cpp
  os/timer_freertos.cpp
)

if(DEBUG)
  target_compile_definitions(board PRIVATE -DDEBUG)
  target_compile_definitions(firmware PRIVATE -DDEBUG)
  target_compile_definitions(stm32_drivers_w_dbg_fw PRIVATE -DDEBUG)
endif()

set(LINKER_SCRIPT firmware.ld)

target_link_options(firmware PRIVATE
  -lm -T${LINKER_DIR}/${LINKER_SCRIPT}
  -Wl,-Map=firmware.map,--cref,--no-warn-mismatch,--gc-sections
)

if(NANO)
  target_link_options(firmware PRIVATE
    --specs=nano.specs
  )
endif()

if (DEFINED TARGET_FLASH_SIZE)
  # message("TARGET_FLASH_SIZE = ${TARGET_FLASH_SIZE}")
  target_link_options(firmware PRIVATE
    -Wl,--defsym=__flash_size=${TARGET_FLASH_SIZE}
  )
endif()

if (DEFINED TARGET_EXTRAM_START)
  target_link_options(firmware PRIVATE
    -Wl,--defsym=__EXTRAM_START__=${TARGET_EXTRAM_START}
  )
endif()

if (DEFINED TARGET_EXTRAM_SIZE)
  target_link_options(firmware PRIVATE
    -Wl,--defsym=__EXTRAM_SIZE__=${TARGET_EXTRAM_SIZE}
  )
endif()

if (DEFINED TARGET_PSRAM_START)
  target_link_options(firmware PRIVATE
    -Wl,--defsym=__PSRAM_START__=${TARGET_PSRAM_START}
  )
endif()

if (LUA)
  # Lua needs %g and %f
  target_link_options(firmware PRIVATE
    "SHELL:-u _printf_float" "SHELL:-u _scanf_float"
  )
endif()

if(ACCESS_DENIED)
  add_subdirectory(thirdparty/AccessDenied)
  include_directories(thirdparty/AccessDenied)
  target_link_libraries(firmware PRIVATE AccessDenied)
endif()

if(FIRMWARE_STRIP_BOOTLOADER)
  # Strip bootloader from binary
  message("-- Strip bootloader from firmware binary")
  set(OBJCOPY_ARGS -R .bootloader)
else()
  message("-- Bootloader contained in firmware binary")
endif()

if(FIRMWARE_FORMAT_UF2)
  if(VERSION_TAG)
    set(UF2_VERSION_STRING "edgetx-${VERSION_TAG} (${GIT_STR})")
  else()
    set(UF2_VERSION_STRING "edgetx-${VERSION_PREFIX}${VERSION}${VERSION_SUFFIX} (${GIT_STR})")
  endif()
  
  add_custom_command(
    TARGET firmware POST_BUILD
    COMMAND ${PYTHON_EXECUTABLE} ${RADIO_DIRECTORY}/util/elf2uf2.py
        --reboot auto --device "${FLAVOUR}"
        --version "${UF2_VERSION_STRING}"
        -i firmware.elf firmware.uf2
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    VERBATIM
  )
endif()

add_custom_command(
  TARGET firmware POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary ${OBJCOPY_ARGS} firmware.elf firmware.bin
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  )


find_program(GAWK gawk
  HINTS "c:/cygwin/bin/" "c:/msys/bin/"
  DOC "gawk for firmware size report.")  # OS X might only have "awk"

if (BASH AND GAWK)
  add_custom_target(firmware-size
    COMMAND ${BASH} -kc '${RADIO_DIRECTORY}/util/elf-size-report.sh --mcu=${CPU_TYPE_FULL} ${SIZE_TARGET_MEM_DEFINE} firmware.elf'
    DEPENDS firmware
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
else()
  add_custom_target(firmware-size
    COMMAND ${CMAKE_SIZE_UTIL} -A firmware.elf
    DEPENDS firmware
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

if(CPU_FAMILY STREQUAL STM32)
  add_custom_target(flash
    COMMAND dfu-util --alt 0 --dfuse-address 0x08000000:leave -d 0483:df11 -D firmware.bin
    DEPENDS firmware
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

  add_custom_target(flash-bl
    COMMAND dfu-util --alt 0 --dfuse-address 0x08000000:leave -d 0483:df11 -D bootloader.bin
    DEPENDS bootloader
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

  add_custom_target(flash-jtag
    COMMAND st-flash --reset write firmware.bin 0x8000000
    DEPENDS firmware
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )

  add_custom_target(flash-stlink
    COMMAND ST-LINK_CLI -c SWD -P firmware.bin 0x8000000 -Rst -Q
    DEPENDS firmware-size
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

PrintTargetReport("firmware")

AddHardwareDefTarget(hardware_defs ${RADIO_DIRECTORY}/src/targets/hw_defs/${FLAVOUR}.json)
