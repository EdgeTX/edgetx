set(TIMERS 3 CACHE STRING "Timers count (2 or 3)")
set_property(CACHE TIMERS PROPERTY STRINGS 2 3)
option(CLI "Command Line Interface" OFF)
option(ENABLE_SERIAL_PASSTHROUGH "Enable serial passthrough" OFF)
option(DEBUG "Debug mode" OFF)
option(LOG_TELEMETRY "Telemetry Logs on SD card" OFF)
option(LOG_BLUETOOTH "Bluetooth Logs on SD card" OFF)
option(TRACE_SD_CARD "Traces SD enabled" OFF)
option(TRACE_FATFS "Traces FatFS enabled" OFF)
option(TRACE_AUDIO "Traces audio enabled" OFF)
option(DEBUG_TRACE_BUFFER "Debug Trace Screen" OFF)
option(XJT "XJT TX Module" ON)
option(MODULE_SIZE_STD "Standard size TX Module" ON)
option(PPM "PPM TX Module" ON)
option(DSM2 "DSM2 TX Module" ON)
option(SBUS "SBUS TX Module" ON)
option(CROSSFIRE "Crossfire TX Module" ON)
option(AFHDS2 "Support for AFHDS2" OFF)
option(AFHDS3 "Support for AFHDS3" OFF)
option(MULTIMODULE "DIY Multiprotocol TX Module (https://github.com/pascallanger/DIY-Multiprotocol-TX-Module)" ON)
option(DEBUG_INTERRUPTS "Count interrupts" OFF)
option(DEBUG_LATENCY "Debug latency" OFF)
option(DEBUG_USB_INTERRUPTS "Count individual USB interrupts" OFF)
option(DEBUG_TIMERS "Time critical parts of the code" OFF)
option(DEBUG_BLUETOOTH "Debug Bluetooth" OFF)

# option to select the default internal module
#set(DEFAULT_INTERNAL_MODULE NONE CACHE STRING "Default internal module")
set_property(CACHE DEFAULT_INTERNAL_MODULE PROPERTY STRINGS ${INTERNAL_MODULES})

# define variables for internal modules
foreach(module ${INTERNAL_MODULES})
  set(INTERNAL_MODULE_${module} ON CACHE BOOL "Support for ${module} internal module")
  if (INTERNAL_MODULE_${module})
    message("-- Adding support for ${module} as internal module")
  endif()
endforeach()

if(INTERNAL_MODULE_PXX1)
  add_definitions(-DHARDWARE_INTERNAL_MODULE)
  add_definitions(-DINTERNAL_MODULE_PXX1)
endif()

if(INTERNAL_MODULE_PXX2)
  set(PXX2 ON)
  add_definitions(-DHARDWARE_INTERNAL_MODULE)
  add_definitions(-DINTERNAL_MODULE_PXX2)
endif()

if(INTERNAL_MODULE_MULTI)
  add_definitions(-DHARDWARE_INTERNAL_MODULE)
  add_definitions(-DINTERNAL_MODULE_MULTI)
  add_definitions(-DINTERNAL_MODULE_SERIAL_TELEMETRY)
endif()

if(INTERNAL_MODULE_CRSF)
  add_definitions(-DHARDWARE_INTERNAL_MODULE)
  add_definitions(-DINTERNAL_MODULE_CRSF)
  add_definitions(-DINTERNAL_MODULE_SERIAL_TELEMETRY)
endif()

if(INTERNAL_MODULE_AFHDS2A)
  add_definitions(-DHARDWARE_INTERNAL_MODULE)
  add_definitions(-DINTERNAL_MODULE_AFHDS2A)
endif()

if(INTERNAL_MODULE_AFHDS3)
  add_definitions(-DHARDWARE_INTERNAL_MODULE)
  add_definitions(-DINTERNAL_MODULE_AFHDS3)
endif()

if(INTERNAL_MODULE_PPM)
  add_definitions(-DHARDWARE_INTERNAL_MODULE)
  add_definitions(-DINTERNAL_MODULE_PPM)
endif()

if(DEFAULT_INTERNAL_MODULE)
  add_definitions(-DDEFAULT_INTERNAL_MODULE=MODULE_TYPE_${DEFAULT_INTERNAL_MODULE})
endif()

if(HARDWARE_EXTERNAL_MODULE)
  add_definitions(-DHARDWARE_EXTERNAL_MODULE)
endif()

if(TIMERS EQUAL 3)
  add_definitions(-DTIMERS=3)
else()
  add_definitions(-DTIMERS=2)
endif()

if(LOG_TELEMETRY)
  add_definitions(-DLOG_TELEMETRY)
endif()

if(LOG_BLUETOOTH)
  add_definitions(-DLOG_BLUETOOTH)
endif()

if(TRACE_SD_CARD)
  add_definitions(-DTRACE_SD_CARD)
  set(DEBUG ON)
  set(DEBUG_TRACE_BUFFER ON)
endif()

if(TRACE_FATFS)
  add_definitions(-DTRACE_FATFS)
  set(DEBUG ON)
  set(DEBUG_TRACE_BUFFER ON)
endif()

if(TRACE_AUDIO)
  add_definitions(-DTRACE_AUDIO)
  set(DEBUG ON)
  set(DEBUG_TRACE_BUFFER ON)
endif()

if(DEBUG_TRACE_BUFFER)
  add_definitions(-DDEBUG_TRACE_BUFFER)
endif()

if(DEBUG_INTERRUPTS)
  add_definitions(-DDEBUG_INTERRUPTS)
  set(DEBUG ON)
  if(DEBUG_USB_INTERRUPTS)
    add_definitions(-DDEBUG_USB_INTERRUPTS)
  endif()
endif()

if(DEBUG_TIMERS)
  add_definitions(-DDEBUG_TIMERS)
  set(DEBUG ON)
endif()

if(DEBUG_LATENCY STREQUAL MIXER_RF)
  add_definitions(-DDEBUG_LATENCY)
  add_definitions(-DDEBUG_LATENCY_MIXER_RF)
endif()

if(DEBUG_LATENCY STREQUAL RF_ONLY)
  add_definitions(-DDEBUG_LATENCY)
  add_definitions(-DDEBUG_LATENCY_RF_ONLY)
endif()

if(DEBUG_LATENCY STREQUAL END_TO_END)
  add_definitions(-DDEBUG_LATENCY)
  add_definitions(-DDEBUG_LATENCY_END_TO_END)
endif()

if(DEBUG_BLUETOOTH)
  add_definitions(-DDEBUG_BLUETOOTH)
  option(DEBUG_BLUETOOTH_VERBOSE "Debug Bluetooth Verbose" OFF)
  if(DEBUG_BLUETOOTH_VERBOSE)
    add_definitions(-DDEBUG_BLUETOOTH_VERBOSE)
  endif()
endif()

if(CLI)
  add_definitions(-DCLI)
  set(FIRMWARE_SRC ${FIRMWARE_SRC} cli.cpp)
endif()

if(CLI OR DEBUG)
  set(SRC ${SRC} dump.cpp)
endif()

if(ENABLE_SERIAL_PASSTHROUGH)
  add_definitions(-DENABLE_SERIAL_PASSTHROUGH)
endif()

if(DEBUG)
  add_definitions(-DDEBUG)
endif()

if(HAPTIC)
  add_definitions(-DHAPTIC)
  set(SRC ${SRC} haptic.cpp)
  set(FIRMWARE_TARGET_SRC ${FIRMWARE_TARGET_SRC} haptic_driver.cpp)
endif()

# Protocols supported

if(PPM)
  add_definitions(-DPPM)
endif()

if(DSM2)
  add_definitions(-DDSM2)
endif()

if(SBUS)
  add_definitions(-DSBUS)
endif()

if(PXX1)
  add_definitions(-DPXX -DPXX1)
  set(PULSES_SRC
    ${PULSES_SRC}
    pxx1.cpp
    )
endif()

if(PXX2)
  add_definitions(-DPXX -DPXX2)
  set(PULSES_SRC
    ${PULSES_SRC}
    pxx2.cpp
    )
endif()

if(XJT)
  add_definitions(-DXJT)
endif()

if(MODULE_SIZE_STD)
  add_definitions(-DHARDWARE_EXTERNAL_MODULE_SIZE_STD)
endif()

if(MULTIMODULE STREQUAL INTERNAL)
  add_definitions(-DINTERNAL_MODULE_MULTI)
endif()

if(MULTIMODULE)
  add_definitions(-DMULTIMODULE)
  set(SRC ${SRC} pulses/multi.cpp telemetry/hitec.cpp telemetry/hott.cpp telemetry/mlink.cpp telemetry/multi.cpp io/multi_firmware_update.cpp)
endif()
if(MULTIMODULE OR AFHDS3)
  set(SRC ${SRC} telemetry/flysky_ibus.cpp)
endif()

if(AFHDS2 AND INTERNAL_MODULE_AFHDS2A)
  add_definitions(-DAFHDS2)
  set(SRC ${SRC} telemetry/flysky_nv14.cpp)
  set(PULSES_SRC ${PULSES_SRC} flysky.cpp)
endif()

if(AFHDS3)
  add_definitions(-DAFHDS3)
  set(PULSES_SRC
    ${PULSES_SRC}
    afhds3.cpp
    afhds3_transport.cpp
    )
endif()

if(CROSSFIRE)
  add_definitions(-DCROSSFIRE)
  set(PULSES_SRC
    ${PULSES_SRC}
    crossfire.cpp
    )
  set(SRC
    ${SRC}
    telemetry/crossfire.cpp
    )
endif()

if(MULTIMODULE OR AFHDS3)
 set(SRC ${SRC} telemetry/flysky_ibus.cpp )
endif()

if(GHOST)
  add_definitions(-DGHOST)
  set(PULSES_SRC
    ${PULSES_SRC}
    ghost.cpp
    )
  set(SRC
    ${SRC}
    telemetry/ghost.cpp
    )
endif()

add_definitions(-g)

add_definitions(-DCPUARM)
add_definitions(-DGPS)
add_definitions(-DBATTGRAPH -DTHRTRACE)
add_definitions(-DSBUS_TRAINER)

foreach(LANGUAGE ${TTS_LANGUAGES})
  string(TOLOWER ${LANGUAGE} lang_lower)
  set(SRC ${SRC} translations/tts_${lang_lower}.cpp)
endforeach()

set(SRC
  ${SRC}
  main.cpp
  tasks.cpp
  audio.cpp
  telemetry/telemetry.cpp
  telemetry/telemetry_sensors.cpp
  telemetry/frsky.cpp
  telemetry/frsky_d.cpp
  telemetry/frsky_sport.cpp
  telemetry/frsky_pxx2.cpp
  telemetry/spektrum.cpp
  crc.cpp
  vario.cpp
  )

set(FIRMWARE_TARGET_SRC
  ${FIRMWARE_TARGET_SRC}
  keys_driver.cpp
  diskio.cpp
  telemetry_driver.cpp
)

set(PULSES_SRC
  ${PULSES_SRC}
  pulses.cpp
  ppm.cpp
  modules_helpers.cpp
  )

if(DSM2)
  set(PULSES_SRC
    ${PULSES_SRC}
    dsm2.cpp
    )
endif()

if(SBUS)
  set(PULSES_SRC
    ${PULSES_SRC}
    sbus.cpp
    )
endif()

if(AFHDS2)
  set(PULSES_SRC
    ${PULSES_SRC}
    afhds2.cpp
    )
endif()

if(AFHDS3)
  set(PULSES_SRC
    ${PULSES_SRC}
    afhds3.cpp
    )
endif()
