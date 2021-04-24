/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

/** @file Main interface layer handler for Lua API. */

#include <ctype.h>
#include <stdio.h>
#include <algorithm>
#include "opentx.h"
#include "bin_allocator.h"
#include "lua_api.h"
#include "sdcard.h"

#if defined(LIBOPENUI)
  #include "api_colorlcd.h"
  #include "libopenui.h"
#else
  #include "libopenui/src/libopenui_file.h"
#endif

extern "C" {
  #include <lundump.h>
}

#define PERMANENT_SCRIPTS_MAX_INSTRUCTIONS (10000/100)
#define MANUAL_SCRIPTS_MAX_INSTRUCTIONS    (20000/100)
#define LUA_WARNING_INFO_LEN               64

lua_State *lsScripts = nullptr;
uint8_t luaState = 0;
uint8_t luaScriptsCount = 0;
bool    luaLcdAllowed = false;
ScriptInternalData scriptInternalData[MAX_SCRIPTS];
ScriptInputsOutputs scriptInputsOutputs[MAX_SCRIPTS];
ScriptInternalData standaloneScript;
uint16_t maxLuaInterval = 0;
uint16_t maxLuaDuration = 0;
uint8_t instructionsPercent = 0;
char lua_warning_info[LUA_WARNING_INFO_LEN+1];
struct our_longjmp * global_lj = 0;
#if defined(COLORLCD)
uint32_t luaExtraMemoryUsage = 0;
#endif

#if defined(LUA_ALLOCATOR_TRACER)

LuaMemTracer lsScriptsTrace;

#if defined(PCBHORUS)
  extern LuaMemTracer lsWidgetsTrace;
  #define GET_TRACER(L)    (L == lsScripts) ? &lsScriptsTrace : &lsWidgetsTrace
#else
  #define GET_TRACER(L)    &lsScriptsTrace
#endif

void *tracer_alloc(void * ud, void * ptr, size_t osize, size_t nsize)
{
  LuaMemTracer * tracer = (LuaMemTracer *)ud;
  if (ptr) {
    if (osize < nsize) {
      // TRACE("Lua alloc %u", nsize - osize);
      tracer->alloc += nsize - osize;
    }
    else {
      // TRACE("Lua free %u", osize - nsize);
      tracer->free += osize - nsize;
    }
  }
  else {
    // TRACE("Lua alloc %u (type %s)", nsize, osize < LUA_TOTALTAGS ? lua_typename(0, osize) : "unk");
    tracer->alloc += nsize;
  }
  return l_alloc(ud, ptr, osize, nsize);
}

#endif // #if defined(LUA_ALLOCATOR_TRACER)

/* custom panic handler */
int custom_lua_atpanic(lua_State * L)
{
  TRACE("PANIC: unprotected error in call to Lua API (%s)", lua_tostring(L, -1));
  if (global_lj) {
    longjmp(global_lj->b, 1);
    /* will never return */
  }
  return 0;
}

void luaHook(lua_State * L, lua_Debug *ar)
{
  if (ar->event == LUA_HOOKCOUNT) {
    instructionsPercent++;
#if defined(DEBUG)
  // Disable Lua script instructions limit in DEBUG mode,
  // just report max value reached
  static uint16_t max = 0;
  if (instructionsPercent > 100) {
    if (max + 10 < instructionsPercent) {
      max = instructionsPercent;
      TRACE("LUA instructionsPercent %u%%", (uint32_t)max);
    }
  }
  else if (instructionsPercent < 10) {
    max = 0;
  }
#else
    if (instructionsPercent > 100) {
      // From now on, as soon as a line is executed, error
      // keep erroring until you're script reaches the top
      lua_sethook(L, luaHook, LUA_MASKLINE, 0);
      luaL_error(L, "CPU limit");
    }
#endif
  }
#if defined(LUA_ALLOCATOR_TRACER)
  else if (ar->event == LUA_HOOKLINE) {
    lua_getinfo(L, "nSl", ar);
    LuaMemTracer * tracer = GET_TRACER(L);
    if (tracer->alloc || tracer->free) {
      TRACE("LT: [+%u,-%u] %s:%d", tracer->alloc, tracer->free, tracer->script, tracer->lineno);
    }
    tracer->script = ar->source;
    tracer->lineno = ar->currentline;
    tracer->alloc = 0;
    tracer->free = 0;
  }
#endif // #if defined(LUA_ALLOCATOR_TRACER)
}

void luaSetInstructionsLimit(lua_State * L, int count)
{
  instructionsPercent = 0;
#if defined(LUA_ALLOCATOR_TRACER)
  lua_sethook(L, luaHook, LUA_MASKCOUNT|LUA_MASKLINE, count);
#else
  lua_sethook(L, luaHook, LUA_MASKCOUNT, count);
#endif
}

int luaGetInputs(lua_State * L, ScriptInputsOutputs & sid)
{
  if (!lua_istable(L, -1))
    return -1;

  memclear(sid.inputs, sizeof(sid.inputs));
  sid.inputsCount = 0;
  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
    luaL_checktype(L, -2, LUA_TNUMBER); // key is number
    luaL_checktype(L, -1, LUA_TTABLE); // value is table
    if (sid.inputsCount<MAX_SCRIPT_INPUTS) {
      uint8_t field = 0;
      int type = 0;
      ScriptInput * si = &sid.inputs[sid.inputsCount];
      for (lua_pushnil(L); lua_next(L, -2) && field<5; lua_pop(L, 1), field++) {
        switch (field) {
          case 0:
            luaL_checktype(L, -2, LUA_TNUMBER); // key is number
            luaL_checktype(L, -1, LUA_TSTRING); // value is string
            si->name = lua_tostring(L, -1);
            break;
          case 1:
            luaL_checktype(L, -2, LUA_TNUMBER); // key is number
            luaL_checktype(L, -1, LUA_TNUMBER); // value is number
            type = lua_tointeger(L, -1);
            if (type >= INPUT_TYPE_FIRST && type <= INPUT_TYPE_LAST) {
              si->type = type;
            }
            if (si->type == INPUT_TYPE_VALUE) {
              si->min = -100;
              si->max = 100;
            }
            else {
              si->max = MIXSRC_LAST_TELEM;
            }
            break;
          case 2:
            luaL_checktype(L, -2, LUA_TNUMBER); // key is number
            luaL_checktype(L, -1, LUA_TNUMBER); // value is number
            if (si->type == INPUT_TYPE_VALUE) {
              si->min = lua_tointeger(L, -1);
            }
            break;
          case 3:
            luaL_checktype(L, -2, LUA_TNUMBER); // key is number
            luaL_checktype(L, -1, LUA_TNUMBER); // value is number
            if (si->type == INPUT_TYPE_VALUE) {
              si->max = lua_tointeger(L, -1);
            }
            break;
          case 4:
            luaL_checktype(L, -2, LUA_TNUMBER); // key is number
            luaL_checktype(L, -1, LUA_TNUMBER); // value is number
            if (si->type == INPUT_TYPE_VALUE) {
              si->def = lua_tointeger(L, -1);
            }
            break;
        }
      }
      sid.inputsCount++;
    }
  }

  return 0;
}

int luaGetOutputs(lua_State * L, ScriptInputsOutputs & sid)
{
  if (!lua_istable(L, -1))
    return -1;

  sid.outputsCount = 0;
  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
    luaL_checktype(L, -2, LUA_TNUMBER); // key is number
    luaL_checktype(L, -1, LUA_TSTRING); // value is string
    if (sid.outputsCount<MAX_SCRIPT_OUTPUTS) {
      sid.outputs[sid.outputsCount++].name = lua_tostring(L, -1);
    }
  }

  return 0;
}

void luaDisable()
{
  POPUP_WARNING("Lua disabled!");
  luaState = INTERPRETER_PANIC;
}

void luaClose(lua_State ** L)
{
  if (*L) {
    PROTECT_LUA() {
      TRACE("luaClose %p", *L);
      lua_close(*L);  // this should not panic, but we make sure anyway
#if defined(LUA_ALLOCATOR_TRACER)
      LuaMemTracer * tracer = GET_TRACER(*L);
      if (tracer->alloc || tracer->free) {
        TRACE("LT: [+%u,-%u] luaClose(%s)", tracer->alloc, tracer->free, (*L == lsScripts) ? "scipts" : "widgets");
      }
      tracer->alloc = 0;
      tracer->free = 0;
#endif // #if defined(LUA_ALLOCATOR_TRACER)
    }
    else {
      // we can only disable Lua for the rest of the session
      if (*L == lsScripts) luaDisable();
    }
    UNPROTECT_LUA();
    *L = nullptr;
  }
}

void luaRegisterLibraries(lua_State * L)
{
  luaL_openlibs(L);
#if defined(COLORLCD)
  registerBitmapClass(L);
#endif
}

#define GC_REPORT_TRESHOLD    (2*1024)

void luaDoGc(lua_State * L, bool full)
{
  if (L) {
    PROTECT_LUA() {
      if (full) {
        lua_gc(L, LUA_GCCOLLECT, 0);
      }
      else {
        lua_gc(L, LUA_GCSTEP, 10);
      }
#if defined(DEBUG)
      if (L == lsScripts) {
        static uint32_t lastgcSctipts = 0;
        uint32_t gc = luaGetMemUsed(L);
        if (gc > (lastgcSctipts + GC_REPORT_TRESHOLD) || (gc + GC_REPORT_TRESHOLD) < lastgcSctipts) {
          lastgcSctipts = gc;
          TRACE("GC Use Scripts: %u bytes", gc);
        }
      }
#if defined(COLORLCD)
      if (L == lsWidgets) {
        static uint32_t lastgcWidgets = 0;
        uint32_t gc = luaGetMemUsed(L);
        if (gc > (lastgcWidgets + GC_REPORT_TRESHOLD) || (gc + GC_REPORT_TRESHOLD) < lastgcWidgets) {
          lastgcWidgets = gc;
          TRACE("GC Use Widgets: %u bytes + Extra %u", gc, luaExtraMemoryUsage);
        }
      }
#endif
#endif
    }
    else {
      // we disable Lua for the rest of the session
      if (L == lsScripts) luaDisable();
#if defined(COLORLCD)
      if (L == lsWidgets) lsWidgets = 0;
#endif
    }
    UNPROTECT_LUA();
  }
}

void luaFree(lua_State * L, ScriptInternalData & sid)
{
  PROTECT_LUA() {
    if (sid.run) {
      luaL_unref(L, LUA_REGISTRYINDEX, sid.run);
      sid.run = 0;
    }
    if (sid.background) {
      luaL_unref(L, LUA_REGISTRYINDEX, sid.background);
      sid.background = 0;
    }
  }
  else {
    luaDisable();
  }
  UNPROTECT_LUA();

  luaDoGc(L, true);
}

#if defined(LUA_COMPILER)
/// callback for luaU_dump()
static int luaDumpWriter(lua_State * L, const void* p, size_t size, void* u)
{
  UNUSED(L);
  UINT written;
  FRESULT result = f_write((FIL *)u, p, size, &written);
  return (result != FR_OK && !written);
}

/*
  @fn luaDumpState(lua_State * L, const char * filename, const FILINFO * finfo, int stripDebug)

  Save compiled bytecode from a given Lua stack to a file.

  @param L The Lua stack to dump.
  @param filename Full path and name of file to save to (typically with .luac extension).
  @param finfo Can be NULL. If not NULL, sets timestamp of created file to match the one in finfo->fdate/ftime
  @param stripDebug This is passed directly to luaU_dump()
    1 = remove debug info from bytecode (smaller but errors are less informative)
    0 = keep debug info
*/
static void luaDumpState(lua_State * L, const char * filename, const FILINFO * finfo, int stripDebug)
{
  FIL D;
  if (f_open(&D, filename, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
    lua_lock(L);
    luaU_dump(L, getproto(L->top - 1), luaDumpWriter, &D, stripDebug);
    lua_unlock(L);
    if (f_close(&D) == FR_OK) {
      if (finfo != nullptr)
        f_utime(filename, finfo);  // set the file mod time
      TRACE("luaDumpState(%s): Saved bytecode to file.", filename);
    }
  } else
    TRACE_ERROR("luaDumpState(%s): Error: Could not open output file.", filename);
}
#endif  // LUA_COMPILER

/**
  @fn luaLoadScriptFileToState(lua_State * L, const char * filename, const char * mode)

  Load a Lua script file into a given lua_State (stack).  May use OpenTx's optional pre-compilation
   feature to save memory and time during load.

  @param L (lua_State) the Lua stack to load into.

  @param filename (string) full path and file name of script.

  @param mode (string) controls whether the file can be text or binary (that is, a pre-compiled file).
   Possible values are:
    "b" only binary.
    "t" only text.
    "T" (default on simulator) prefer text but load binary if that is the only version available.
    "bt" (default on radio) either binary or text, whichever is newer (binary preferred when timestamps are equal).
    Add "x" to avoid automatic compilation of source file to .luac version.
      Eg: "tx", "bx", or "btx".
    Add "c" to force compilation of source file to .luac version (even if existing version is newer than source file).
      Eg: "tc" or "btc" (forces "t", overrides "x").
    Add "d" to keep extra debug info in the compiled binary.
      Eg: "td", "btd", or "tcd" (no effect with just "b" or with "x").

  @retval (int)
  SCRIPT_OK on success (LUA_OK)
  SCRIPT_NOFILE if file wasn't found for specified mode or Lua could not open file (LUA_ERRFILE)
  SCRIPT_SYNTAX_ERROR if Lua returned a syntax error during pre/de-compilation (LUA_ERRSYNTAX)
  SCRIPT_PANIC for Lua memory errors (LUA_ERRMEM or LUA_ERRGCMM)
*/
int luaLoadScriptFileToState(lua_State * L, const char * filename, const char * mode)
{
  if (luaState == INTERPRETER_PANIC) {
    return SCRIPT_PANIC;
  } else if (filename == nullptr) {
    return SCRIPT_NOFILE;
  }

  int lstatus;
  char lmode[6] = "bt";
  uint8_t ret = SCRIPT_NOFILE;

  if (mode != nullptr) {
    strncpy(lmode, mode, sizeof(lmode)-1);
    lmode[sizeof(lmode)-1] = '\0';
  }

#if defined(LUA_COMPILER)
  uint16_t fnamelen;
  uint8_t extlen;
  char filenameFull[LEN_FILE_PATH_MAX + FF_MAX_LFN + 1] = "\0";
  FILINFO fnoLuaS, fnoLuaC;
  FRESULT frLuaS, frLuaC;

  bool scriptNeedsCompile = false;
  uint8_t loadFileType = 0;  // 1=text, 2=binary

  memset(&fnoLuaS, 0, sizeof(FILINFO));
  memset(&fnoLuaC, 0, sizeof(FILINFO));

  fnamelen = strlen(filename);
  // check if file extension is already in the file name and strip it
  getFileExtension(filename, fnamelen, 0, nullptr, &extlen);
  fnamelen -= extlen;
  if (fnamelen > sizeof(filenameFull) - sizeof(SCRIPT_BIN_EXT)) {
    TRACE_ERROR("luaLoadScriptFileToState(%s, %s): Error loading script: filename buffer overflow.\n", filename, lmode);
    return ret;
  }
  strncat(filenameFull, filename, fnamelen);

  // check if binary version exists
  strcpy(filenameFull + fnamelen, SCRIPT_BIN_EXT);
  frLuaC = f_stat(filenameFull, &fnoLuaC);

  // check if text version exists
  strcpy(filenameFull + fnamelen, SCRIPT_EXT);
  frLuaS = f_stat(filenameFull, &fnoLuaS);

  // decide which version to load, text or binary
  if (frLuaC != FR_OK && frLuaS == FR_OK) {
    // only text version exists
    loadFileType = 1;
    scriptNeedsCompile = true;
  }
  else if (frLuaC == FR_OK && frLuaS != FR_OK) {
    // only binary version exists
    loadFileType = 2;
  }
  else if (frLuaS == FR_OK) {
    // both versions exist, compare them
    if (strchr(lmode, 'c') || (uint32_t)((fnoLuaC.fdate << 16) + fnoLuaC.ftime) < (uint32_t)((fnoLuaS.fdate << 16) + fnoLuaS.ftime)) {
      // text version is newer than binary or forced by "c" mode flag, rebuild it
      scriptNeedsCompile = true;
    }
    if (scriptNeedsCompile || !strchr(lmode, 'b')) {
      // text version needs compilation or forced by mode
      loadFileType = 1;
    } else {
      // use binary file
      loadFileType = 2;
    }
  }
  // else both versions are missing

  // skip compilation based on mode flags? ("c" overrides "x")
  if (scriptNeedsCompile && strchr(lmode, 'x') && !strchr(lmode, 'c')) {
    scriptNeedsCompile = false;
  }

  if (loadFileType == 2) {
    // change file extension to binary version
    strcpy(filenameFull + fnamelen, SCRIPT_BIN_EXT);
  }

//  TRACE_DEBUG("luaLoadScriptFileToState(%s, %s):\n", filename, lmode);
//  TRACE_DEBUG("\tldfile='%s'; ldtype=%u; compile=%u;\n", filenameFull, loadFileType, scriptNeedsCompile);
//  TRACE_DEBUG("\t%-5s: %s; mtime: %04X%04X = %u/%02u/%02u %02u:%02u:%02u;\n", SCRIPT_EXT, (frLuaS == FR_OK ? "ok" : "nf"), fnoLuaS.fdate, fnoLuaS.ftime,
//      (fnoLuaS.fdate >> 9) + 1980, (fnoLuaS.fdate >> 5) & 15, fnoLuaS.fdate & 31, fnoLuaS.ftime >> 11, (fnoLuaS.ftime >> 5) & 63, (fnoLuaS.ftime & 31) * 2);
//  TRACE_DEBUG("\t%-5s: %s; mtime: %04X%04X = %u/%02u/%02u %02u:%02u:%02u;\n", SCRIPT_BIN_EXT, (frLuaC == FR_OK ? "ok" : "nf"), fnoLuaC.fdate, fnoLuaC.ftime,
//      (fnoLuaC.fdate >> 9) + 1980, (fnoLuaC.fdate >> 5) & 15, fnoLuaC.fdate & 31, fnoLuaC.ftime >> 11, (fnoLuaC.ftime >> 5) & 63, (fnoLuaC.ftime & 31) * 2);

  // final check that file exists and is allowed by mode flags
  if (!loadFileType || (loadFileType == 1 && !strpbrk(lmode, "tTc")) || (loadFileType == 2 && !strpbrk(lmode, "bT"))) {
    TRACE_ERROR("luaLoadScriptFileToState(%s, %s): Error loading script: file not found.\n", filename, lmode);
    return SCRIPT_NOFILE;
  }

#else  // !defined(LUA_COMPILER)

  // use passed file name as-is
  const char *filenameFull = filename;

#endif

  TRACE("luaLoadScriptFileToState(%s, %s): loading %s", filename, lmode, filenameFull);

  // we don't pass <mode> on to loadfilex() because we want lua to load whatever file we specify, regardless of content
  lstatus = luaL_loadfilex(L, filenameFull, nullptr);
#if defined(LUA_COMPILER)
  // Check for bytecode encoding problem, eg. compiled for x64. Unfortunately Lua doesn't provide a unique error code for this. See Lua/src/lundump.c.
  if (lstatus == LUA_ERRSYNTAX && loadFileType == 2 && frLuaS == FR_OK && strstr(lua_tostring(L, -1), "precompiled")) {
    loadFileType = 1;
    scriptNeedsCompile = true;
    strcpy(filenameFull + fnamelen, SCRIPT_EXT);
    TRACE_ERROR("luaLoadScriptFileToState(%s, %s): Error loading script: %s\n\tRetrying with %s\n", filename, lmode, lua_tostring(L, -1), filenameFull);
    lstatus = luaL_loadfilex(L, filenameFull, nullptr);
  }
  if (lstatus == LUA_OK) {
    if (scriptNeedsCompile && loadFileType == 1) {
      strcpy(filenameFull + fnamelen, SCRIPT_BIN_EXT);
      luaDumpState(L, filenameFull, &fnoLuaS, (strchr(lmode, 'd') ? 0 : 1));
    }
    ret = SCRIPT_OK;
  }
#else
  if (lstatus == LUA_OK) {
    ret = SCRIPT_OK;
  }
#endif
  else {
    TRACE_ERROR("luaLoadScriptFileToState(%s, %s): Error loading script: %s\n", filename, lmode, lua_tostring(L, -1));
    if (lstatus == LUA_ERRFILE) {
      ret = SCRIPT_NOFILE;
    }
    else if (lstatus == LUA_ERRSYNTAX) {
      ret = SCRIPT_SYNTAX_ERROR;
    }
    else {  //  LUA_ERRMEM or LUA_ERRGCMM
      ret = SCRIPT_PANIC;
    }
  }

  return ret;
}

static int luaLoad(lua_State * L, const char * filename, ScriptInternalData & sid, ScriptInputsOutputs * sio=nullptr)
{
  int init = 0;
  int lstatus = 0;

  sid.instructions = 0;
  sid.state = SCRIPT_OK;

  if (luaState == INTERPRETER_PANIC) {
    return SCRIPT_PANIC;
  }

  luaSetInstructionsLimit(L, MANUAL_SCRIPTS_MAX_INSTRUCTIONS);

  PROTECT_LUA() {
    sid.state = luaLoadScriptFileToState(L, filename, LUA_SCRIPT_LOAD_MODE);
    if (sid.state == SCRIPT_OK && (lstatus = lua_pcall(L, 0, 1, 0)) == LUA_OK && lua_istable(L, -1)) {
      for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
        const char * key = lua_tostring(L, -2);
        if (!strcmp(key, "init")) {
          init = luaL_ref(L, LUA_REGISTRYINDEX);
          lua_pushnil(L);
        }
        else if (!strcmp(key, "run")) {
          sid.run = luaL_ref(L, LUA_REGISTRYINDEX);
          lua_pushnil(L);
        }
        else if (!strcmp(key, "background")) {
          sid.background = luaL_ref(L, LUA_REGISTRYINDEX);
          lua_pushnil(L);
        }
        else if (sio && !strcmp(key, "input")) {
          luaGetInputs(L, *sio);
        }
        else if (sio && !strcmp(key, "output")) {
          luaGetOutputs(L, *sio);
        }
      }

      if (init) {
        lua_rawgeti(L, LUA_REGISTRYINDEX, init);
        if (lua_pcall(L, 0, 0, 0) != 0) {
          TRACE_ERROR("luaLoad(%s): Error in script init(): %s\n", filename, lua_tostring(L, -1));
          sid.state = SCRIPT_SYNTAX_ERROR;
        }
        luaL_unref(L, LUA_REGISTRYINDEX, init);
        lua_gc(L, LUA_GCCOLLECT, 0);
      }
    }
    else if (sid.state == SCRIPT_OK) {
      TRACE_ERROR("luaLoad(%s): Error parsing script (%d): %s\n", filename, lstatus, lua_tostring(L, -1));
      sid.state = SCRIPT_SYNTAX_ERROR;
    }
  }
  else {
    luaDisable();
    return SCRIPT_PANIC;
  }
  UNPROTECT_LUA();

  if (sid.state != SCRIPT_OK) {
    luaFree(L, sid);
  }

  luaDoGc(L, true);

  return sid.state;
}

bool luaLoadMixScript(uint8_t index)
{
  ScriptData & sd = g_model.scriptsData[index];

  if (ZEXIST(sd.file)) {
    ScriptInternalData & sid = scriptInternalData[luaScriptsCount++];
    ScriptInputsOutputs * sio = &scriptInputsOutputs[index];
    sid.reference = SCRIPT_MIX_FIRST+index;
    sid.state = SCRIPT_NOFILE;
    char filename[sizeof(SCRIPTS_MIXES_PATH) + LEN_SCRIPT_FILENAME + sizeof(SCRIPT_EXT)] = SCRIPTS_MIXES_PATH "/";
    strncpy(filename + sizeof(SCRIPTS_MIXES_PATH), sd.file, LEN_SCRIPT_FILENAME);
    filename[sizeof(SCRIPTS_MIXES_PATH) + LEN_SCRIPT_FILENAME] = '\0';
    strcat(filename + sizeof(SCRIPTS_MIXES_PATH), SCRIPT_EXT);
    if (luaLoad(lsScripts, filename, sid, sio) == SCRIPT_PANIC) {
      return false;
    }
  }
  return true;
}

bool luaLoadFunctionScript(uint8_t index, uint8_t ref)
{
  CustomFunctionData * fn;

  if (ref < SCRIPT_GFUNC_FIRST)
    fn = &g_model.customFn[index];
  else if (!g_model.noGlobalFunctions)
    fn = &g_eeGeneral.customFn[index];
  else
    return true;

  if (fn->func == FUNC_PLAY_SCRIPT && ZEXIST(fn->play.name)) {
    if (luaScriptsCount < MAX_SCRIPTS) {
      ScriptInternalData & sid = scriptInternalData[luaScriptsCount++];
      sid.reference = ref + index;
      sid.state = SCRIPT_NOFILE;
      char filename[sizeof(SCRIPTS_FUNCS_PATH) + LEN_FUNCTION_NAME + sizeof(SCRIPT_EXT)] = SCRIPTS_FUNCS_PATH "/";
      strncpy(filename + sizeof(SCRIPTS_FUNCS_PATH), fn->play.name, LEN_FUNCTION_NAME);
      filename[sizeof(SCRIPTS_FUNCS_PATH) + LEN_FUNCTION_NAME] = '\0';
      strcat(filename + sizeof(SCRIPTS_FUNCS_PATH), SCRIPT_EXT);
      if (luaLoad(lsScripts, filename, sid) == SCRIPT_PANIC) {
        return false;
      }
    }
    else {
      POPUP_WARNING(STR_TOO_MANY_LUA_SCRIPTS);
      return false;
    }
  }
  return true;
}

#if defined(PCBTARANIS)
bool luaLoadTelemetryScript(uint8_t index)
{
  TelemetryScreenType screenType = TELEMETRY_SCREEN_TYPE(index);

  if (screenType == TELEMETRY_SCREEN_TYPE_SCRIPT) {
    TelemetryScriptData & script = g_model.screens[index].script;
    if (ZEXIST(script.file)) {
      if (luaScriptsCount < MAX_SCRIPTS) {
        ScriptInternalData & sid = scriptInternalData[luaScriptsCount++];
        sid.reference = SCRIPT_TELEMETRY_FIRST+index;
        sid.state = SCRIPT_NOFILE;
        char filename[sizeof(SCRIPTS_TELEM_PATH) + LEN_SCRIPT_FILENAME + sizeof(SCRIPT_EXT)] = SCRIPTS_TELEM_PATH "/";
        strncpy(filename + sizeof(SCRIPTS_TELEM_PATH), script.file, LEN_SCRIPT_FILENAME);
        filename[sizeof(SCRIPTS_TELEM_PATH) + LEN_SCRIPT_FILENAME] = '\0';
        strcat(filename + sizeof(SCRIPTS_TELEM_PATH), SCRIPT_EXT);
        if (luaLoad(lsScripts, filename, sid) == SCRIPT_PANIC) {
          return false;
        }
      }
      else {
        POPUP_WARNING(STR_TOO_MANY_LUA_SCRIPTS);
        return false;
      }
    }
  }
  return true;
}
#endif

uint8_t isTelemetryScriptAvailable(uint8_t index)
{
  for (int i=0; i<luaScriptsCount; i++) {
    ScriptInternalData & sid = scriptInternalData[i];
    if (sid.reference == SCRIPT_TELEMETRY_FIRST+index) {
      return sid.state;
    }
  }
  return SCRIPT_NOFILE;
}

void luaLoadPermanentScripts()
{
  luaScriptsCount = 0;
  memset(scriptInternalData, 0, sizeof(scriptInternalData));
  memset(scriptInputsOutputs, 0, sizeof(scriptInputsOutputs));

  // Load model scripts
  for (int i=0; i<MAX_SCRIPTS; i++) {
    if (!luaLoadMixScript(i)) {
      return;
    }
  }

  // Load custom function scripts
  for (int i=0; i<MAX_SPECIAL_FUNCTIONS; i++) {
    if (!luaLoadFunctionScript(i, SCRIPT_FUNC_FIRST) || !luaLoadFunctionScript(i, SCRIPT_GFUNC_FIRST)) {
      return;
    }
  }

#if defined(PCBTARANIS)
  // Load custom telemetry scripts
  for (int i=0; i<MAX_TELEMETRY_SCREENS; i++) {
    if (!luaLoadTelemetryScript(i)) {
      return;
    }
  }
#endif
}

void displayLuaError(const char * title)
{
#if !defined(COLORLCD)
  drawMessageBox(title);
#endif
  if (lua_warning_info[0]) {
    char * split = strstr(lua_warning_info, ": ");
    if (split) {
#if LCD_W == 128
      if (strlen(split + 2) <= 20) {
        lcdDrawSizedText(WARNING_LINE_X, WARNING_LINE_Y + FH + 3, lua_warning_info, split - lua_warning_info, SMLSIZE);
        lcdDrawSizedText(WARNING_LINE_X, WARNING_LINE_Y + 2 * FH + 2, split + 2, strlen(split + 2), SMLSIZE);
      }
      else {
        lcdDrawSizedText(WARNING_LINE_X, WARNING_LINE_Y + FH, lua_warning_info, split - lua_warning_info, SMLSIZE);
        lcdDrawSizedText(WARNING_LINE_X, WARNING_LINE_Y + 2 * FH, split + 2, 20, SMLSIZE);
        lcdDrawSizedText(WARNING_LINE_X, WARNING_LINE_Y + 3 * FH, split + 22, strlen(split + 22), SMLSIZE);
      }
#elif LCD_W == 64
      lcdDrawSizedText(WARNING_LINE_X, WARNING_LINE_Y + FH + 3, lua_warning_info, split - lua_warning_info, SMLSIZE);
      lcdDrawSizedText(WARNING_LINE_X, WARNING_LINE_Y + 2 * FH + 2, split + 2, lua_warning_info + LUA_WARNING_INFO_LEN - split, SMLSIZE);
#endif
    }
    else {
      // TODO lcdDrawSizedText(WARNING_LINE_X, WARNING_LINE_Y + FH + 3, lua_warning_info, 40, SMLSIZE);
    }
  }
}

void displayAcknowledgeLuaError(event_t event)
{
  warningResult = false;
  displayLuaError(warningText);
  if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    warningText = nullptr;
  }
}

void luaError(lua_State * L, uint8_t error, bool acknowledge)
{
  const char * errorTitle;

  switch (error) {
    case SCRIPT_SYNTAX_ERROR:
      errorTitle = STR_SCRIPT_SYNTAX_ERROR;
      break;
    case SCRIPT_KILLED:
      errorTitle = STR_SCRIPT_KILLED;
      break;
    case SCRIPT_PANIC:
      errorTitle = STR_SCRIPT_PANIC;
      break;
    default:
      errorTitle = STR_SCRIPT_ERROR;
      break;
  }

  const char * msg = lua_tostring(L, -1);
  if (msg) {
#if defined(SIMU)
    if (!strncmp(msg, ".", 2)) msg += 1;
#endif
#if LCD_W == 128
      msg = strrchr(msg, '/') + 1;
#else
    if (!strncmp(msg, "/SCRIPTS/", 9)) msg += 9;
#endif
    strncpy(lua_warning_info, msg, LUA_WARNING_INFO_LEN);
    lua_warning_info[LUA_WARNING_INFO_LEN] = '\0';
  }
  else {
    lua_warning_info[0] = '\0';
  }

  if (acknowledge) {
    warningText = errorTitle;
    warningType = WARNING_TYPE_INFO;
    popupFunc = displayAcknowledgeLuaError;
  }
  else {
    displayLuaError(errorTitle);
  }
}

void luaExec(const char * filename)
{
  luaInit();

  if (luaState != INTERPRETER_PANIC) {
    standaloneScript.state = SCRIPT_NOFILE;
    int result = luaLoad(lsScripts, filename, standaloneScript);
    // TODO the same with run ...
    if (result == SCRIPT_OK) {
      luaState = INTERPRETER_RUNNING_STANDALONE_SCRIPT;
    }
    else {
      luaError(lsScripts, result);
      luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS;
    }
  }
}

void luaDoOneRunStandalone(event_t evt)
{
  static uint8_t luaDisplayStatistics = false;

  if (standaloneScript.state == SCRIPT_OK && standaloneScript.run) {
    luaSetInstructionsLimit(lsScripts, MANUAL_SCRIPTS_MAX_INSTRUCTIONS);
    lua_rawgeti(lsScripts, LUA_REGISTRYINDEX, standaloneScript.run);
    lua_pushunsigned(lsScripts, evt);
    if (lua_pcall(lsScripts, 1, 1, 0) == 0) {
      if (!lua_isnumber(lsScripts, -1)) {
        if (instructionsPercent > 100) {
          TRACE("Script killed");
          standaloneScript.state = SCRIPT_KILLED;
          luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS;
        }
        else if (lua_isstring(lsScripts, -1)) {
          char nextScript[FF_MAX_LFN+1];
          strncpy(nextScript, lua_tostring(lsScripts, -1), FF_MAX_LFN);
          nextScript[FF_MAX_LFN] = '\0';
          luaExec(nextScript);
        }
        else {
          TRACE("Script run function returned unexpected value");
          standaloneScript.state = SCRIPT_SYNTAX_ERROR;
          luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS;
        }
      }
      else {
        int scriptResult = lua_tointeger(lsScripts, -1);
        lua_pop(lsScripts, 1);  /* pop returned value */
        if (scriptResult != 0) {
          TRACE("Script finished with status %d", scriptResult);
          standaloneScript.state = SCRIPT_NOFILE;
          luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS;
          return;
        }
        else if (luaDisplayStatistics) {
#if !defined(COLORLCD)
          lcdDrawSolidHorizontalLine(0, 7*FH-1, lcdLastRightPos+6, ERASE);
          lcdDrawText(0, 7*FH, "GV Use: ");
          lcdDrawNumber(lcdLastRightPos, 7*FH, luaGetMemUsed(lsScripts), LEFT);
          lcdDrawChar(lcdLastRightPos, 7*FH, 'b');
          lcdDrawSolidHorizontalLine(0, 7*FH-2, lcdLastRightPos+6, FORCE);
          lcdDrawVerticalLine(lcdLastRightPos+6, 7*FH-2, FH+2, SOLID, FORCE);
#endif
        }
      }
    }
    else {
      TRACE("Script error: %s", lua_tostring(lsScripts, -1));
      standaloneScript.state = (instructionsPercent > 100 ? SCRIPT_KILLED : SCRIPT_SYNTAX_ERROR);
      luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS;
    }

    if (standaloneScript.state != SCRIPT_OK) {
      luaError(lsScripts, standaloneScript.state);
      luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS;
    }

    if (evt == EVT_KEY_LONG(KEY_EXIT)) {
      TRACE("Script force exit");
      killEvents(evt);
      standaloneScript.state = SCRIPT_NOFILE;
      luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS;
    }
#if defined(KEYS_GPIO_REG_MENU)
  // TODO find another key and add a #define
    else if (evt == EVT_KEY_LONG(KEY_MENU)) {
      killEvents(evt);
      luaDisplayStatistics = !luaDisplayStatistics;
    }
#endif
  }
  else {
    TRACE("Script run method missing");
    standaloneScript.state = SCRIPT_SYNTAX_ERROR;
    luaState = INTERPRETER_RELOAD_PERMANENT_SCRIPTS;
  }
}

bool luaDoOneRunPermanentScript(event_t evt, int i, uint32_t scriptType)
{
  ScriptInternalData & sid = scriptInternalData[i];
  if (sid.state != SCRIPT_OK) return false;

  luaSetInstructionsLimit(lsScripts, PERMANENT_SCRIPTS_MAX_INSTRUCTIONS);
  int inputsCount = 0;
#if defined(SIMU) || defined(DEBUG)
  const char *filename;
#endif
  ScriptInputsOutputs * sio = nullptr;
#if SCRIPT_MIX_FIRST > 0
  if ((scriptType & RUN_MIX_SCRIPT) && (sid.reference >= SCRIPT_MIX_FIRST && sid.reference <= SCRIPT_MIX_LAST)) {
#else
  if ((scriptType & RUN_MIX_SCRIPT) && (sid.reference <= SCRIPT_MIX_LAST)) {
#endif
    ScriptData & sd = g_model.scriptsData[sid.reference-SCRIPT_MIX_FIRST];
    sio = &scriptInputsOutputs[sid.reference-SCRIPT_MIX_FIRST];
    inputsCount = sio->inputsCount;
#if defined(SIMU) || defined(DEBUG)
    filename = sd.file;
#endif
    lua_rawgeti(lsScripts, LUA_REGISTRYINDEX, sid.run);
    for (int j=0; j<sio->inputsCount; j++) {
      if (sio->inputs[j].type == INPUT_TYPE_SOURCE)
        luaGetValueAndPush(lsScripts, sd.inputs[j].source);
      else
        lua_pushinteger(lsScripts, sd.inputs[j].value + sio->inputs[j].def);
    }
  }
  else if ((scriptType & RUN_FUNC_SCRIPT) && (sid.reference >= SCRIPT_FUNC_FIRST && sid.reference <= SCRIPT_GFUNC_LAST)) {
    CustomFunctionData & fn = (sid.reference < SCRIPT_GFUNC_FIRST ? g_model.customFn[sid.reference-SCRIPT_FUNC_FIRST] : g_eeGeneral.customFn[sid.reference-SCRIPT_GFUNC_FIRST]);
#if defined(SIMU) || defined(DEBUG)
    filename = fn.play.name;
#endif
    if (getSwitch(fn.swtch))
      lua_rawgeti(lsScripts, LUA_REGISTRYINDEX, sid.run);
    else if (sid.background)
      lua_rawgeti(lsScripts, LUA_REGISTRYINDEX, sid.background);
    else
      return false;
  }
  else {
#if defined(PCBTARANIS)
#if defined(SIMU) || defined(DEBUG)
    TelemetryScriptData & script = g_model.screens[sid.reference-SCRIPT_TELEMETRY_FIRST].script;
    filename = script.file;
#endif
    if ((scriptType & RUN_TELEM_FG_SCRIPT) && (menuHandlers[0]==menuViewTelemetry && sid.reference==SCRIPT_TELEMETRY_FIRST+s_frsky_view)) {
      lua_rawgeti(lsScripts, LUA_REGISTRYINDEX, sid.run);
      lua_pushunsigned(lsScripts, evt);
      inputsCount = 1;
    }
    else if ((scriptType & RUN_TELEM_BG_SCRIPT) && (sid.background)) {
      lua_rawgeti(lsScripts, LUA_REGISTRYINDEX, sid.background);
    }
    else {
      return false;
    }
#else // !PCBTARANIS
    // COLORLCD
    return false;
#endif
  }

  if (lua_pcall(lsScripts, inputsCount, sio ? sio->outputsCount : 0, 0) == 0) {
    if (sio) {
      for (int j=sio->outputsCount-1; j>=0; j--) {
        if (!lua_isnumber(lsScripts, -1)) {
          sid.state = (instructionsPercent > 100 ? SCRIPT_KILLED : SCRIPT_SYNTAX_ERROR);
          TRACE("Script %8s disabled", filename);
          break;
        }
        sio->outputs[j].value = lua_tointeger(lsScripts, -1);
        lua_pop(lsScripts, 1);
      }
    }
  }
  else {
    if (instructionsPercent > 100) {
      TRACE("Script %8s killed", filename);
      sid.state = SCRIPT_KILLED;
    }
    else {
      TRACE("Script %8s error: %s", filename, lua_tostring(lsScripts, -1));
      sid.state = SCRIPT_SYNTAX_ERROR;
    }
  }

  if (sid.state != SCRIPT_OK) {
    luaFree(lsScripts, sid);
  }
  else {
    if (instructionsPercent > sid.instructions) {
      sid.instructions = instructionsPercent;
    }
  }
  return true;
}


// This loop-pass should run in checkEvents().
// However, libopenui has 2 main separated routines:
// - checkEvents(): runs the event loop and can effect an async redraw (if visible)
// - paint(): (if visible) the redraw itself
//
bool luaTask(event_t evt, uint8_t scriptType, bool allowLcdUsage)
{
  if (luaState == INTERPRETER_PANIC) return false;
  luaLcdAllowed = allowLcdUsage;
  bool scriptWasRun = false;

  // TRACE("luaTask(0x%x,0x%x,%s)",
  //       evt, scriptType,
  //       allowLcdUsage ? "true" : "false");
  
  // we run either standalone script or permanent scripts
  if (luaState & INTERPRETER_RUNNING_STANDALONE_SCRIPT) {
    // run standalone script
    if ((scriptType & RUN_STNDAL_SCRIPT) == 0) return false;
    PROTECT_LUA() {
      luaDoOneRunStandalone(evt);
      scriptWasRun = true;
    }
    else {
      luaDisable();
      return false;
    }
    UNPROTECT_LUA();
  }
  else {
    // run permanent scripts
    if (luaState & INTERPRETER_RELOAD_PERMANENT_SCRIPTS) {
      luaState = 0;
      luaInit();
      if (luaState == INTERPRETER_PANIC) return false;
      luaLoadPermanentScripts();
      if (luaState == INTERPRETER_PANIC) return false;
    }

    for (int i=0; i<luaScriptsCount; i++) {
      PROTECT_LUA() {
        scriptWasRun |= luaDoOneRunPermanentScript(evt, i, scriptType);
      }
      else {
        luaDisable();
        break;
      }
      UNPROTECT_LUA();
      //todo gc step between scripts
    }
  }
  luaDoGc(lsScripts, false);
#if defined(COLORLCD)
  luaDoGc(lsWidgets, false);
#endif
  return scriptWasRun;
}

void checkLuaMemoryUsage()
{
#if (LUA_MEM_MAX > 0)
  uint32_t totalMemUsed = luaGetMemUsed(lsScripts);
#if defined(COLORLCD)
  totalMemUsed += luaGetMemUsed(lsWidgets);
  totalMemUsed += luaExtraMemoryUsage;
#endif
  if (totalMemUsed > LUA_MEM_MAX) {
    TRACE("checkLuaMemoryUsage(): max limit reached (%u), killing Lua", totalMemUsed);
    // disable Lua scripts
    luaClose(&lsScripts);
    luaDisable();
#if defined(COLORLCD)
    // disable widgets
    luaClose(&lsWidgets);
#endif
  }
#endif
}

uint32_t luaGetMemUsed(lua_State * L)
{
  return L ? (lua_gc(L, LUA_GCCOUNT, 0) << 10) + lua_gc(L, LUA_GCCOUNTB, 0) : 0;
}

void luaInit()
{
  TRACE("luaInit");

  luaClose(&lsScripts);

  if (luaState != INTERPRETER_PANIC) {
#if defined(USE_BIN_ALLOCATOR)
    lsScripts = lua_newstate(bin_l_alloc, nullptr);   //we use our own allocator!
#elif defined(LUA_ALLOCATOR_TRACER)
    memset(&lsScriptsTrace, 0 , sizeof(lsScriptsTrace));
    lsScriptsTrace.script = "lua_newstate(scripts)";
    lsScripts = lua_newstate(tracer_alloc, &lsScriptsTrace);   //we use tracer allocator
#else
    lsScripts = lua_newstate(l_alloc, nullptr);   //we use Lua default allocator
#endif
    if (lsScripts) {
      // install our panic handler
      lua_atpanic(lsScripts, &custom_lua_atpanic);

#if defined(LUA_ALLOCATOR_TRACER)
      lua_sethook(lsScripts, luaHook, LUA_MASKLINE, 0);
#endif

      // protect libs and constants registration
      PROTECT_LUA() {
        luaRegisterLibraries(lsScripts);
      }
      else {
        // if we got panic during registration
        // we disable Lua for this session
        luaDisable();
      }
      UNPROTECT_LUA();
      TRACE("lsScripts %p", lsScripts);
    }
    else {
      /* log error and return */
      luaDisable();
    }
  }
}

bool readToolName(char * toolName, const char * filename)
{
  FIL file;
  char buffer[1024];
  UINT count;

  if (f_open(&file, filename, FA_READ) != FR_OK) {
    return "Error opening file";
  }

  FRESULT res = f_read(&file, &buffer, sizeof(buffer), &count);
  f_close(&file);

  if (res != FR_OK)
    return false;

  const char * tns = "TNS|";
  auto * start = std::search(buffer, buffer + sizeof(buffer), tns, tns + 4);
  if (start >= buffer + sizeof(buffer))
    return false;

  start += 4;

  const char * tne = "|TNE";
  auto * end = std::search(buffer, buffer + sizeof(buffer), tne, tne + 4);
  if (end >= buffer + sizeof(buffer) || end <= start)
    return false;

  uint8_t len = end - start;
  if (len > RADIO_TOOL_NAME_MAXLEN)
    return false;

  strncpy(toolName, start, len);
  memclear(toolName + len, RADIO_TOOL_NAME_MAXLEN + 1 - len);

  return true;
}

bool isRadioScriptTool(const char * filename)
{
  const char * ext = getFileExtension(filename);
  return ext && !strcasecmp(ext, SCRIPT_EXT);
}
