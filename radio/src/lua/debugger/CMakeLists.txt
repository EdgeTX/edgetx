set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${RADIO_SRC_DIR}/${NANOPB_DIR}/extra)
find_package(Nanopb REQUIRED)
include_directories(${NANOPB_INCLUDE_DIRS})

nanopb_generate_cpp(PROTO_SRCS PROTO_HDRS eldp.proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DLUA_DEBUGGER)

set(LUA_DEBUGGER_SRC
    lua_debugger.c
)

foreach(FILE ${LUA_DEBUGGER_SRC})
    set(SRC ${SRC} lua/debugger/${FILE})
endforeach()