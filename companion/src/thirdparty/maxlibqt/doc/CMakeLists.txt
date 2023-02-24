## Generate Doxygen docs from source

set(DOC_OUTPUT_DIR "${DOC_DIR}" CACHE STRING "Documentation output path.")

find_package(Doxygen)

if(DOXYGEN_FOUND AND DOC_DIR AND DOC_OUTPUT_DIR)
	set(DOXYGEN_IN ${DOC_DIR}/Doxyfile.in)
	set(DOXYGEN_OUT "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

	configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

	add_custom_target(maxLibQt-doxygen
		COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
		WORKING_DIRECTORY ${DOC_DIR}
		COMMENT "Generating API documentation with Doxygen."
		VERBATIM )

	## clean
	file(TO_NATIVE_PATH "html/*.html html/*.js html/*.css html/*.png" DOCFILES)
	if(WIN32)
		set(RM del /Q)
		separate_arguments(DOCFILES WINDOWS_COMMAND "${DOCFILES}")
	else()
		set(RM rm)
		separate_arguments(DOCFILES UNIX_COMMAND "${DOCFILES}")
	endif()
	add_custom_target(maxLibQt-doxygen-clean
		COMMAND ${RM} ${DOCFILES}
		COMMAND ${CMAKE_COMMAND} -E remove_directory html/search
		WORKING_DIRECTORY ${DOC_OUTPUT_DIR}
		COMMENT "Deleting stale documentation files."
		VERBATIM )
endif()
