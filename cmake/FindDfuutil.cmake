# - try to find program dfu-util

set(DFU_UTIL_ROOT_DIR
	"${DFU_UTIL_ROOT_DIR}"
	CACHE
	PATH
  "Root directory to search for dfu-util")

find_program(DFU_UTIL_PROGRAM
  NAMES
    dfu-util
  HINTS
    "${DFU_UTIL_ROOT_DIR}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(DFU_UTIL
	DEFAULT_MSG
	DFU_UTIL_PROGRAM)

if(DFU_UTIL_FOUND)
  set(DFU_UTIL_PATH "${DFU_UTIL_PROGRAM}")
  mark_as_advanced(DFU_UTIL_ROOT_DIR)
endif()

mark_as_advanced(DFU_UTIL_PROGRAM)
