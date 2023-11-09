# - try to find libssl-1 library

if (DEFINED ENV{LIBSSL1_ROOT_DIR})
  set(LIBSSL1_ROOT_DIR "$ENV{LIBSSL1_ROOT_DIR}")
endif()

find_library(LIBSSL1_LIBRARY
	NAMES
    libssl.so.1.1
    libssl-1_1.dll
    libssl-1_1.dylib
  HINTS
    "${LIBSSL1_ROOT_DIR}")

get_filename_component(LIBSSL1_LIBRARY_PATH ${LIBSSL1_LIBRARY} PATH)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LIBSSL1
	DEFAULT_MSG
	LIBSSL1_LIBRARY_PATH)

if(LIBSSL1_FOUND)
	set(LIBSSL1_LIBRARY_DIR "${LIBSSL1_LIBRARY_PATH}")
	mark_as_advanced(LIBSSL1_ROOT_DIR)
endif()

mark_as_advanced(LIBSSL1_LIBRARY_PATH)
