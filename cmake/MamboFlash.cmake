# Original TBS controller bootloader: USB HID, not STM32 DFU. The application
# allocation and its ELF are checked before the uploader can open USB.
set(MAMBO_FLASH_SCRIPT "${CMAKE_SOURCE_DIR}/tools/mambo-flash/mambo_flash.py")
set(MAMBO_FLASH_RUNNER "${CMAKE_SOURCE_DIR}/cmake/RunMamboFlash.cmake")

foreach(mode IN ITEMS PLAN PROBE FLASH)
  if(mode STREQUAL "PLAN")
    set(target_name flash-plan)
  elseif(mode STREQUAL "PROBE")
    set(target_name flash-probe)
  else()
    set(target_name flash)
  endif()
  add_custom_target(${target_name}
    COMMAND ${CMAKE_COMMAND}
      "-DMAMBO_FLASH_MODE=${mode}"
      "-DMAMBO_FLASH_PYTHON=${PYTHON_EXECUTABLE}"
      "-DMAMBO_FLASH_SCRIPT=${MAMBO_FLASH_SCRIPT}"
      "-DMAMBO_FLASH_IMAGE=${CMAKE_BINARY_DIR}/firmware.bin"
      "-DMAMBO_FLASH_ELF=${CMAKE_BINARY_DIR}/firmware.elf"
      "-DMAMBO_FLASH_LOG_ROOT=${CMAKE_BINARY_DIR}/mambo-flash-logs"
      -P "${MAMBO_FLASH_RUNNER}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    USES_TERMINAL
    VERBATIM)
  if(NOT mode STREQUAL "PROBE")
    add_dependencies(${target_name} firmware)
  endif()
endforeach()
