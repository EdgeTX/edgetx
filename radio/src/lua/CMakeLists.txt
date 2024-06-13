
add_definitions(-DLUA)

if(LUA_COMPILER)
  add_definitions(-DLUA_COMPILER)
endif()

if(LUA_ALLOCATOR_TRACER AND DEBUG)
  add_definitions(-DLUA_ALLOCATOR_TRACER)
endif()

if(NOT "${LUA_SCRIPT_LOAD_MODE}" STREQUAL "")
  add_definitions(-DLUA_SCRIPT_LOAD_MODE="${LUA_SCRIPT_LOAD_MODE}")
endif()

include_directories(${LUA_DIR})

if(LUA_MIXER)
  add_definitions(-DLUA_MODEL_SCRIPTS)
endif()

set(SRC ${SRC}
  lua/interface.cpp
  lua/api_general.cpp
  lua/api_model.cpp
  lua/api_filesystem.cpp
  lua/lua_event.cpp
)

AddHWGenTarget(${HW_DESC_JSON} lua_inputs lua_inputs.inc)
AddHWGenTarget(${HW_DESC_JSON} lua_mixsrc lua_mixsrc.inc)
AddHWGenTarget(${HW_DESC_JSON} lua_keys lua_keys.inc)

set(SRC ${SRC}
  ${CMAKE_CURRENT_BINARY_DIR}/lua_inputs.inc
  ${CMAKE_CURRENT_BINARY_DIR}/lua_mixsrc.inc
  ${CMAKE_CURRENT_BINARY_DIR}/lua_keys.inc
)

# helper target for testing purposes
add_custom_target(lua_inputs DEPENDS ${HW_DESC_JSON} lua_inputs.inc)
add_custom_target(lua_mixsrc DEPENDS ${HW_DESC_JSON} lua_mixsrc.inc)
add_custom_target(lua_keys DEPENDS ${HW_DESC_JSON} lua_keys.inc)

if(GUI_DIR STREQUAL colorlcd)
  set(SRC ${SRC} lua/api_colorlcd.cpp lua/api_colorlcd_lvgl.cpp lua/widgets.cpp)
else()
  set(SRC ${SRC} lua/api_stdlcd.cpp)
endif()

set(LUA_SRC
  lapi.c
  lcode.c
  lctype.c
  ldebug.c
  ldo.c
  ldump.c
  lfunc.c
  lgc.c
  llex.c
  lmem.c
  lobject.c
  lopcodes.c
  lparser.c
  lstate.c
  lstring.c
  ltable.c
  ltm.c
  lundump.c
  lvm.c
  lzio.c
  linit.c
  lbaselib.c
  lmathlib.c
  lbitlib.c
  loadlib.c
  lauxlib.c
  ltablib.c
  lcorolib.c
  liolib.c
  lstrlib.c
)

foreach(FILE ${LUA_SRC})
  set(SRC ${SRC} ${LUA_DIR}/${FILE})
endforeach()
