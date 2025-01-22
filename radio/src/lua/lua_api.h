/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#pragma once

#if defined(LUA)

// prevent C++ code to be included from lua.h
#include "rtos.h"

extern "C" {
  #include "lua.h"
};

#include "dataconstants.h"
#include "edgetx_types.h"

#ifndef LUA_SCRIPT_LOAD_MODE
  // Can force loading of binary (.luac) or plain-text (.lua) versions of scripts specifically, and control
  //  compilation options. See interface.cpp:luaLoadScriptFileToState() <mode> parameter description for details.
  #if !defined(LUA_COMPILER) || defined(SIMU) || defined(DEBUG)
    #define LUA_SCRIPT_LOAD_MODE    "T"   // prefer loading .lua source file for full debug info
  #else
    #define LUA_SCRIPT_LOAD_MODE    "bt"  // binary or text, whichever is newer
  #endif
#endif

// LUA serial connection
#define LUA_FIFO_SIZE 256
void luaAllocRxFifo();
void luaFreeRxFifo();
void luaReceiveData(uint8_t* buf, uint32_t len);

void luaSetSendCb(void* ctx, void (*cb)(void*, uint8_t));
void luaSetGetSerialByte(void* ctx, int (*fct)(void*, uint8_t*));


extern bool luaLcdAllowed;

#if defined(COLORLCD)
extern bool luaLcdAllowed;

class BitmapBuffer;
extern BitmapBuffer* luaLcdBuffer;

class LuaWidget;
extern LuaWidget* runningFS;

class LuaLvglManager;
extern LuaLvglManager* luaLvglManager;

extern uint32_t luaExtraMemoryUsage;
void luaInitThemesAndWidgets();
#endif

void luaInit();
void luaClose();

void luaEmptyEventBuffer();

#define lua_registernumber(L, n, i)    (lua_pushnumber(L, (i)), lua_setglobal(L, (n)))
#define lua_registerint(L, n, i)       (lua_pushinteger(L, (i)), lua_setglobal(L, (n)))
#define lua_pushtableboolean(L, k, v)  (lua_pushstring(L, (k)), lua_pushboolean(L, (v)), lua_settable(L, -3))
#define lua_pushtableinteger(L, k, v)  (lua_pushstring(L, (k)), lua_pushinteger(L, (v)), lua_settable(L, -3))
#define lua_pushtablenumber(L, k, v)   (lua_pushstring(L, (k)), lua_pushnumber(L, (v)), lua_settable(L, -3))

// size based string (possibly no null-termination)
#define __lua_strncpy(s)              \
  char tmp[sizeof(s) + 1];            \
  strncpy(tmp, (s), sizeof(tmp) - 1); \
  tmp[sizeof(s)] = '\0';

// size based string (possibly no null-termination)
#define lua_pushnstring(L, s)          { __lua_strncpy(s); lua_pushstring(L, tmp); }
#define lua_pushtablenstring(L, k, v)  { __lua_strncpy(v); lua_pushstring(L, (k)); lua_pushstring(L, tmp); lua_settable(L, -3); }

// null-terminated string
#define lua_pushtablestring(L, k, v)   (lua_pushstring(L, (k)), lua_pushstring(L, (v)), lua_settable(L, -3))

#define lua_registerlib(L, name, tab)  (luaL_newmetatable(L, name), luaL_setfuncs(L, tab, 0), lua_setglobal(L, name))

// Must match first two entries in ZoneOpton::Type enum
// TODO: should be cleaned up
enum luaScriptInputType {
  INPUT_TYPE_FIRST = 0,
  INPUT_TYPE_VALUE = INPUT_TYPE_FIRST,
  INPUT_TYPE_SOURCE,
  INPUT_TYPE_LAST = INPUT_TYPE_SOURCE
};

struct ScriptInput {
  const char *name;
  uint8_t type;
  int16_t min;
  int16_t max;
  int16_t def;
};

struct ScriptOutput {
  const char *name;
  int16_t value;
};

enum ScriptState {
  SCRIPT_OK,
  SCRIPT_NOFILE,
  SCRIPT_SYNTAX_ERROR,
  SCRIPT_PANIC
};

enum ScriptReference {
#if defined(LUA_MODEL_SCRIPTS)
  SCRIPT_MIX_FIRST,
  SCRIPT_MIX_LAST=SCRIPT_MIX_FIRST+MAX_SCRIPTS-1,
#endif
  SCRIPT_FUNC_FIRST,
  SCRIPT_FUNC_LAST=SCRIPT_FUNC_FIRST+MAX_SPECIAL_FUNCTIONS-1,    // model functions
  SCRIPT_GFUNC_FIRST,
  SCRIPT_GFUNC_LAST=SCRIPT_GFUNC_FIRST+MAX_SPECIAL_FUNCTIONS-1,  // global functions
#if defined(PCBTARANIS)
  SCRIPT_TELEMETRY_FIRST,
  SCRIPT_TELEMETRY_LAST=SCRIPT_TELEMETRY_FIRST+MAX_SCRIPTS,      // telem0 and telem1 .. telem7
#endif
  SCRIPT_STANDALONE                                              // Standalone script
};

struct ScriptInternalData {
  uint8_t reference;
  uint8_t state;
  int run;
  int background;
  uint8_t instructions;
#if defined(COLORLCD)  
  bool useLvgl;
#endif
};

struct ScriptInputsOutputs {
  uint8_t inputsCount;
  ScriptInput inputs[MAX_SCRIPT_INPUTS];
  uint8_t outputsCount;
  ScriptOutput outputs[MAX_SCRIPT_OUTPUTS];
};


enum InterpreterState {
  INTERPRETER_RELOAD_PERMANENT_SCRIPTS = 1,
  INTERPRETER_LOADING,
  INTERPRETER_START_RUNNING,
  INTERPRETER_RUNNING,
#if defined(COLORLCD)
  INTERPRETER_PAUSED,
#endif
  INTERPRETER_PANIC = 255
};

extern uint8_t luaState;
extern uint8_t luaScriptsCount;
extern bool    luaLcdAllowed;

extern ScriptInternalData scriptInternalData[MAX_SCRIPTS];
extern ScriptInputsOutputs scriptInputsOutputs[MAX_SCRIPTS];

bool luaTask(bool allowLcdUsage);
void checkLuaMemoryUsage();
void luaExec(const char * filename);
bool isTelemetryScriptAvailable();

#define luaGetCpuUsed(idx) scriptInternalData[idx].instructions
#define LUA_LOAD_MODEL_SCRIPTS()   luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS
#define LUA_LOAD_MODEL_SCRIPT(idx) luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS

// Lua PROTECT/UNPROTECT
#include <setjmp.h>

struct our_longjmp {
  struct our_longjmp *previous;
  jmp_buf b;
  volatile int status;  /* error code */
};

extern struct our_longjmp * global_lj;

#define PROTECT_LUA()   { struct our_longjmp lj; \
                        lj.previous = global_lj;  /* chain new error handler */ \
                        global_lj = &lj;  \
                        if (setjmp(lj.b) == 0)
#define UNPROTECT_LUA() global_lj = lj.previous; }   /* restore old error handler */

extern uint16_t maxLuaInterval;
extern uint16_t maxLuaDuration;
extern uint8_t instructionsPercent;

struct LuaField {
  uint16_t id;
  char name[20];
  char desc[50];
};

bool luaFindFieldByName(const char * name, LuaField & field, unsigned int flags=0);
bool luaFindFieldById(int id, LuaField & field, unsigned int flags=0);
void luaLoadThemes();

// Unregister LUA widget factories
void luaUnregisterWidgets();

#if LCD_W > 350
  #define RADIO_TOOL_NAME_MAXLEN  40
#else
  #define RADIO_TOOL_NAME_MAXLEN  16
#endif

bool readToolName(char * toolName, const char * filename);
bool isRadioScriptTool(const char * filename);

struct LuaMemTracer {
  const char * script;
  int lineno;
  uint32_t alloc;
  uint32_t free;
};

void * tracer_alloc(void * ud, void * ptr, size_t osize, size_t nsize);

void l_pushtableint(lua_State* ls, const char * key, int value);
void l_pushtablebool(lua_State* ls, const char * key, bool value);

#else  // defined(LUA)

#define luaInit()
#define LUA_LOAD_MODEL_SCRIPTS()

#endif // defined(LUA)
