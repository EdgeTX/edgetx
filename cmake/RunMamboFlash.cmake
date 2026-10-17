cmake_minimum_required(VERSION 3.20)

if(NOT MAMBO_FLASH_MODE MATCHES "^(PLAN|PROBE|FLASH)$")
  message(FATAL_ERROR "Unknown Mambo flashing mode '${MAMBO_FLASH_MODE}'")
endif()
if(NOT EXISTS "${MAMBO_FLASH_PYTHON}" OR NOT EXISTS "${MAMBO_FLASH_SCRIPT}")
  message(FATAL_ERROR "Mambo flashing requires the build Python and HID uploader")
endif()

if(MAMBO_FLASH_MODE STREQUAL "PROBE")
  set(args --probe)
else()
  set(args --image "${MAMBO_FLASH_IMAGE}" --elf "${MAMBO_FLASH_ELF}")
endif()

if(MAMBO_FLASH_MODE STREQUAL "FLASH")
  # Read at build time: setting these must not reconfigure/rebuild the firmware
  # after its SHA256 was approved with flash-plan.
  set(serial "$ENV{MAMBO_FLASH_SERIAL}")
  set(sha256 "$ENV{MAMBO_FLASH_SHA256}")
  if(serial STREQUAL "")
    message(FATAL_ERROR
      "Set MAMBO_FLASH_SERIAL to the controller serial shown by flash-probe. USB not opened.")
  endif()
  string(LENGTH "${sha256}" hash_length)
  if(NOT hash_length EQUAL 64 OR NOT sha256 MATCHES "^[0-9a-fA-F]+$")
    message(FATAL_ERROR
      "Set MAMBO_FLASH_SHA256 to the approved SHA256 shown by flash-plan. USB not opened.")
  endif()
  list(APPEND args --execute --expected-serial "${serial}"
    --expected-sha256 "${sha256}" --log-root "${MAMBO_FLASH_LOG_ROOT}")
endif()

if(DEFINED ENV{MAMBO_HID_LIBRARY} AND NOT "$ENV{MAMBO_HID_LIBRARY}" STREQUAL "")
  list(APPEND args --hid-library "$ENV{MAMBO_HID_LIBRARY}")
endif()

execute_process(COMMAND "${MAMBO_FLASH_PYTHON}" "${MAMBO_FLASH_SCRIPT}" ${args}
  RESULT_VARIABLE result)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Mambo ${MAMBO_FLASH_MODE} stopped (exit code ${result}); no automatic retry")
endif()
