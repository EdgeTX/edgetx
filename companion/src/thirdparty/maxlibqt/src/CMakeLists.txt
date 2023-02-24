cmake_minimum_required(VERSION 2.8.12)
project(maxLibQt)

set(SRC_SUBDIRS
	core
	itemmodels
	widgets
)

set(BUILT_LIBRARIES)  # populated by subprojects

foreach(sub_dir ${SRC_SUBDIRS})
	add_subdirectory(${sub_dir})
endforeach(sub_dir)

# maxLibQt is a "dummy" lib so that other binaries could link
# to this single one instead of the individual libs.
# The maxLibQt target cannot be built specifically.
if (BUILT_LIBRARIES)
	add_library(${PROJECT_NAME} INTERFACE)
	target_link_libraries(${PROJECT_NAME} INTERFACE ${BUILT_LIBRARIES})
endif()
