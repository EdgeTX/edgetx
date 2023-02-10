
macro(add_lua_export_target target)
  add_custom_command(
    OUTPUT lua_exports_${target}.inc
    COMMAND ${CMAKE_C_COMPILER} -E ${ARGN} -DEXPORT ${RADIO_SRC_DIR}/dataconstants.h > lua_exports_${target}.txt
    COMMAND ${PYTHON_EXECUTABLE} ${RADIO_DIRECTORY}/util/luaexport.py ${VERSION} lua_exports_${target}.txt lua_exports_${target}.inc lua_fields_${target}.txt
    )
  add_custom_target(lua_export_${target} DEPENDS lua_exports_${target}.inc)
endmacro(add_lua_export_target)

set(LUA_INCLUDES
  -DCPUARM -DLUA -DLUA_INPUTS -DLUA_EXPORT_GENERATION -DSIMU ${SYSROOT_ARG}
  -I${RADIO_SRC_DIR}
  -I${RADIO_SRC_DIR}/targets/${TARGET_DIR}
  -I${RADIO_SRC_DIR}/thirdparty
  -I${RADIO_SRC_DIR}/targets/common/arm/stm32
  )

if(PYTHONINTERP_FOUND)
  add_lua_export_target(x9lite    ${LUA_INCLUDES} -DPCBTARANIS -DPCBX9LITE -DPCBX9)
  add_lua_export_target(x9lites   ${LUA_INCLUDES} -DPCBTARANIS -DPCBX9LITES -DPCBX9LITE -DPCBX9)
  add_lua_export_target(x7        ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_X7)
  add_lua_export_target(x7access  ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7ACCESS -DPCBX7 -DRADIO_X7ACCESS)
  add_lua_export_target(xlite     ${LUA_INCLUDES} -DPCBTARANIS -DPCBXLITE)
  add_lua_export_target(xlites    ${LUA_INCLUDES} -DPCBTARANIS -DPCBXLITES -DPCBXLITE -DIMU)
  add_lua_export_target(x9d       ${LUA_INCLUDES} -DPCBTARANIS -DPCBX9D -DPCBX9)
  add_lua_export_target(x9d+2019  ${LUA_INCLUDES} -DPCBTARANIS -DPCBX9D -DPCBX9 -DRADIO_X9DP2019)
  add_lua_export_target(x9e       ${LUA_INCLUDES} -DPCBTARANIS -DPCBX9E -DPCBX9)
  add_lua_export_target(x10       ${LUA_INCLUDES} -DPCBHORUS   -DPCBX10)
  add_lua_export_target(x12s      ${LUA_INCLUDES} -DPCBHORUS   -DPCBX12S)
  add_lua_export_target(t12       ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_T12)
  add_lua_export_target(tlite     ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_TLITE)
  add_lua_export_target(lr3pro    ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_LR3PRO)
  add_lua_export_target(tpro      ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_TPRO)
  add_lua_export_target(tx12      ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_TX12)
  add_lua_export_target(tx12mk2   ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_TX12MK2)
  add_lua_export_target(boxer     ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_BOXER)
  add_lua_export_target(zorro     ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_ZORRO)
  add_lua_export_target(t8        ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_T8)
  add_lua_export_target(commando8 ${LUA_INCLUDES} -DPCBTARANIS -DPCBX7 -DRADIO_COMMANDO8)
  add_lua_export_target(t16       ${LUA_INCLUDES} -DPCBHORUS   -DPCBX10 -DRADIO_T16)
  add_lua_export_target(nv14      ${LUA_INCLUDES} -I${RADIO_SRC_DIR}/targets/nv14 -DPCBNV14)
endif()

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

set(RADIO_DEPENDENCIES ${RADIO_DEPENDENCIES} ${LUA_EXPORT})

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

if(GUI_DIR STREQUAL colorlcd)
  set(SRC ${SRC} lua/api_colorlcd.cpp lua/widgets.cpp)
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
