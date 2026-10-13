# Experimental application-only Mambo target. The existing TBS bootloader and
# RF firmware are not built or included; internal Tracer remains disabled.
option(MAMBO_EXTERNAL_CRSF "Experimental Mambo external Crossfire bay" OFF)
if(MAMBO_EXTERNAL_CRSF)
  add_definitions(-DMAMBO_EXTERNAL_CRSF)
endif()
set(BOOTLOADER OFF CACHE BOOL "Preserve original TBS bootloader" FORCE)
set(FWDRIVE OFF CACHE BOOL "Do not expose a firmware-write USB drive" FORCE)
set(FIRMWARE_STRIP_BOOTLOADER YES)
set(RTC_BACKUP_RAM OFF CACHE BOOL "Preserve TBS bootloader backup SRAM metadata" FORCE)
set(TEST_BUILD_WARNING ON CACHE BOOL "Experimental Mambo bench build" FORCE)
include(targets/taranis/CMakeLists.txt)
target_sources(board PRIVATE targets/mambo/inputs.cpp targets/mambo/gimbals.cpp
  targets/mambo/gimbals_stm32.cpp targets/mambo/calibration.cpp)
if(MAMBO_EXTERNAL_CRSF)
  target_sources(board PRIVATE targets/mambo/external_rf.cpp)
endif()
