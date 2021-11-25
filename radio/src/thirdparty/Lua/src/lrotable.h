// Read-only tables for Lua

#ifndef lrotable_h
#define lrotable_h

#include "lua.h"
#include "llimits.h"
#include "lauxlib.h"
#include "lobject.h"

typedef uint8_t luaR_result;

// A number entry in the read only table
typedef struct
{
  const char *name;
  lua_Number value;
} luaR_value_entry;

// A string entry in the read only table
typedef struct
{
  const char *name;
  const char *string;
} luaR_string_entry;

extern const luaR_value_entry baselib_vals[];
extern const luaR_value_entry mathlib_vals[];
extern const luaR_value_entry opentxConstants[];
extern const luaR_string_entry edgetxStrings[];

// A mapping between table name and its entries
typedef struct
{
  const char *name;
  const luaL_Reg *pfuncs;
  const luaR_value_entry *pvalues;
  const luaR_string_entry *pstrings;
} luaR_table;

extern const luaR_table lua_rotable[];

luaR_result luaR_findglobal(lua_State *L, const char * name, TValue * val);
luaR_result luaR_findentry(lua_State *L, void * data, const char * key, TValue * val);

#endif
