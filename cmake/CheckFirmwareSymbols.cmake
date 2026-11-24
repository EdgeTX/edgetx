# Fails the firmware build if the linked ELF contains newlib's
# double-precision conversion chain or its scanf engine.
#
# The firmware replaces newlib's printf with thirdparty/printf (float only)
# and strtof/strtod with a float-only implementation (--wrap). Anything that
# still reaches newlib's _strtod_r / _dtoa_r drags in its Bignum code again:
# ~12 KB on monochrome targets, ~50 KB on color targets (full newlib).
#
# Usage: cmake -DNM=<nm> -DELF=<firmware.elf> -P CheckFirmwareSymbols.cmake

set(FORBIDDEN_SYMBOLS
  _dtoa_r         # double -> string (newlib printf %e/%f/%g, ecvt/fcvt/gcvt)
  _strtod_r       # string -> double (scanf %f, strtold, ...)
  _strtod_l
  __ssvfscanf_r   # scanf engine behind sscanf()/vsscanf()
  __svfscanf_r
  _svfscanf_r
)

execute_process(
  COMMAND ${NM} -g --defined-only ${ELF}
  OUTPUT_VARIABLE NM_OUTPUT
  RESULT_VARIABLE NM_RESULT
)
if(NOT NM_RESULT EQUAL 0)
  message(FATAL_ERROR "CheckFirmwareSymbols: '${NM}' failed on ${ELF}")
endif()

set(FOUND "")
foreach(SYM ${FORBIDDEN_SYMBOLS})
  if(NM_OUTPUT MATCHES " ${SYM}(\n|$)")
    list(APPEND FOUND ${SYM})
  endif()
endforeach()

if(FOUND)
  # Remove the ELF so that the next build relinks and fails again,
  # instead of considering the target up to date.
  file(REMOVE ${ELF})
  list(JOIN FOUND " " FOUND_STR)
  message(FATAL_ERROR
    "Firmware links newlib code it must not use: ${FOUND_STR}\n"
    "This brings back newlib's double-precision conversion and/or scanf code.\n"
    "Usual culprits:\n"
    "  - sscanf()/vsscanf(): parse with strtol()/strtoul()/strtof() instead\n"
    "  - printf-family functions not provided by thirdparty/printf (fprintf, asprintf, ...)\n"
    "Use 'grep -rn sscanf radio/src' or the cross-reference table in firmware.map to find the caller."
  )
endif()
