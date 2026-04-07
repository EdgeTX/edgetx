cmake_minimum_required(VERSION 3.10)
project(edgetx-luac C)

# Native host build of luac that produces bytecode compatible with
# EdgeTX's 32-bit Lua 5.3 configuration (float numbers, int integers).
#
# Usage:
#   cmake -S radio/src/thirdparty/Lua -B build-luac
#   cmake --build build-luac
#
# The resulting 'edgetx-luac' binary can compile scripts for any EdgeTX radio:
#   edgetx-luac -s -o script.luac script.lua

set(LUA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LUA_CORE_SRC
  ${LUA_DIR}/lapi.c
  ${LUA_DIR}/lcode.c
  ${LUA_DIR}/lctype.c
  ${LUA_DIR}/ldebug.c
  ${LUA_DIR}/ldo.c
  ${LUA_DIR}/ldump.c
  ${LUA_DIR}/lfunc.c
  ${LUA_DIR}/lgc.c
  ${LUA_DIR}/llex.c
  ${LUA_DIR}/lmem.c
  ${LUA_DIR}/lobject.c
  ${LUA_DIR}/lopcodes.c
  ${LUA_DIR}/lparser.c
  ${LUA_DIR}/lstate.c
  ${LUA_DIR}/lstring.c
  ${LUA_DIR}/ltable.c
  ${LUA_DIR}/ltm.c
  ${LUA_DIR}/lundump.c
  ${LUA_DIR}/lvm.c
  ${LUA_DIR}/lzio.c
  ${LUA_DIR}/lauxlib.c
)

add_executable(edgetx-luac
  ${LUA_DIR}/luac.c
  ${LUA_CORE_SRC}
)

target_compile_definitions(edgetx-luac PRIVATE
  LUA_HOST_BUILD
  LUA_CROSS_COMPILER
  LUA_COMPAT_5_2
)

target_include_directories(edgetx-luac PRIVATE ${LUA_DIR})

# Link math library on Unix
if(UNIX)
  target_link_libraries(edgetx-luac m)
endif()

set_target_properties(edgetx-luac PROPERTIES
  OUTPUT_NAME edgetx-luac
  C_STANDARD 99
)
