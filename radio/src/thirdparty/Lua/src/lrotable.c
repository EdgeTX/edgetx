/* Read-only tables for Lua */

#include <string.h>
#include "lua.h"
#include "lauxlib.h"
#include "lstring.h"
#include "lrotable.h"

/* Utility function: find a key in a given table of functions */
static luaR_result luaR_findfunctionkey(const luaL_Reg* pf, const char * key, TValue * found) {
  if(!pf)
    return 0;  

  while(1) {
    if (!pf->name)
      break;
    if (!strcmp(pf->name, key)) {
      setfvalue(found, pf->func);
      return 1;
    }
    pf++;
  }
  return 0;
}

/* Utility function: find a key in a given table of constants */
static luaR_result luaR_findconstantkey(const luaR_value_entry* pv, const char * key, TValue * found) {
  if(!pv)
    return 0;  

  while(1) {
    if (!pv->name)
      break;
    if (!strcmp(pv->name, key)) {
      setnvalue(found, pv->value);
      return 1;
    }
    pv++;
  }
  return 0;
}

/* Utility function: find a key in a given table of strings */
static luaR_result luaR_findstringkey(lua_State *L, const luaR_string_entry* ps, const char * key, TValue * found) {
  if(!ps)
    return 0;  

  while(1) {
    if (!ps->name)
      break;
    if (!strcmp(ps->name, key)) {
      TString *ts = luaS_new(L, ps->string);
      setsvalue(L, found, ts);
      return 1;
    }
    ps++;
  }
  return 0;
}

/* Find a global "read only table" in the constant lua_rotable array */
luaR_result luaR_findglobal(lua_State *L, const char * name, TValue * val) {
  unsigned i;
  if (strlen(name) > LUA_MAX_ROTABLE_NAME) {
    TRACE_LUA_INTERNALS("luaR_findglobal('%s') = NAME TOO LONG", name);
    return 0;
  }
  for (i=0; lua_rotable[i].name; i++) {
    void * table = (void *)(&lua_rotable[i]);
    if (!strcmp(lua_rotable[i].name, name)) {
      setrvalue(val, table);
      TRACE_LUA_INTERNALS("luaR_findglobal('%s') = TABLE %p (%s)", name, table, lua_rotable[i].name);
      return 1;
    }
    if (!strncmp(lua_rotable[i].name, "__", 2)) {
      if (luaR_findentry(L, table, name, val)) {
        TRACE_LUA_INTERNALS("luaR_findglobal('%s') = FOUND in table '%s'", name, lua_rotable[i].name);
        return 1;
      }
    }
  }
  TRACE_LUA_INTERNALS("luaR_findglobal() '%s' = NOT FOUND", name);
  return 0;
}


luaR_result luaR_findentry(lua_State *L, void *data, const char * key, TValue * val) {
  luaR_table * table = (luaR_table *)data;
  /* First look at the functions */
  if (luaR_findfunctionkey(table->pfuncs, key, val)) {
    TRACE_LUA_INTERNALS("luaR_findentry(%p[%s], '%s') = FUNCTION %p", table, table->name, key, lfvalue(val));
    return 1;
  }
  else if (luaR_findconstantkey(table->pvalues, key, val)) {
    /* Then at the values */
    TRACE_LUA_INTERNALS("luaR_findentry(%p[%s], '%s') = NUMBER %g", table, table->name, key, nvalue(val));
    return 1;
  }
  else if (luaR_findstringkey(L, table->pstrings, key, val)) {
    /* Then at the strings */
    TRACE_LUA_INTERNALS("luaR_findentry(%p[%s], '%s') = STRING %s", table, table->name, key, svalue(val));
    return 1;
  }
  TRACE_LUA_INTERNALS("luaR_findentry(%p[%s], '%s') = NOT FOUND", table, table->name, key);
  return 0;
}
