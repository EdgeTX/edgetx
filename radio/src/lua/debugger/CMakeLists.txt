set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${RADIO_SRC_DIR}/${NANOPB_DIR}/extra)
find_package(Nanopb REQUIRED)
include_directories(${NANOPB_INCLUDE_DIRS})

nanopb_generate_cpp(PROTO_SRCS PROTO_HDRS eldp.proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DELDB)

set(ELDB_SRC
    lua_debugger.c
)

include_directories(${RADIO_SRC_DIR}/lua/debugger)

foreach(FILE ${ELDB_SRC})
    set(SRC ${SRC} lua/debugger/${FILE})
endforeach()