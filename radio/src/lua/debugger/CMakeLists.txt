set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH};${RADIO_SRC_DIR}/${NANOPB_DIR}/extra)
# set(NANOPB_OPTIONS "-v") # for debugging nanopb
find_package(Nanopb REQUIRED)
include_directories(${NANOPB_INCLUDE_DIRS})

nanopb_generate_cpp(PROTO_SRCS PROTO_HDRS RELPATH ${RADIO_SRC_DIR}/lua/debugger ${RADIO_SRC_DIR}/lua/debugger/eldp.proto)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DELDB -DELDB_SCRIPT_LOAD_MODE="T")

set(ELDB_SRC
    internal/eldb.tpp
    internal/encode_decode.cpp
    internal/messages.tpp
    internal/session.cpp
)

foreach(FILE ${ELDB_SRC})
    set(SRC ${SRC} lua/debugger/${FILE})
endforeach()
set(SRC ${SRC} ${PROTO_SRCS} ${PROTO_HDRS})