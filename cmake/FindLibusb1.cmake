# - try to find libusb-1 library

set(LIBUSB1_ROOT_DIR
	"${LIBUSB1_ROOT_DIR}"
	CACHE
	PATH
	"Root directory to search for libusb-1")

find_library(LIBUSB1_LIBRARY
	NAMES
    libusb-1.0
    libusb-1.0.so
    libusb-1.0.so.0
    libusb-1.0.dll
    libusb-1.0.dylib
  HINTS
    "${LIBUSB1_ROOT_DIR}")

get_filename_component(LIBUSB1_LIBRARY_PATH ${LIBUSB1_LIBRARY} PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBUSB1
	DEFAULT_MSG
	LIBUSB1_LIBRARY_PATH)

if(LIBUSB1_FOUND)
	set(LIBUSB1_LIBRARY_DIR "${LIBUSB1_LIBRARY_PATH}")
	mark_as_advanced(LIBUSB1_ROOT_DIR)
endif()

mark_as_advanced(LIBUSB1_LIBRARY_PATH)
