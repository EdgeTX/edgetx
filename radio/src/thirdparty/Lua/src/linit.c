/*
** $Id: linit.c,v 1.32 2011/04/08 19:17:36 roberto Exp $
** Initialization of libraries for lua.c and other clients
** See Copyright Notice in lua.h
*/


/*
** If you embed Lua in your program and need to open the standard
** libraries, call luaL_openlibs in your program. If you need a
** different set of libraries, copy this file to your project and edit
** it to suit your needs.
*/


#define linit_c
#define LUA_LIB

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "lstring.h"
#include "ltable.h"
#include "lstate.h"
#include "lgc.h"

#include "lua/api_filesystem.h"
#include "lua/api_colorlcd.h"

extern LROT_TABLE(iolib);
extern LROT_TABLE(strlib);
extern LROT_TABLE(mathlib);
extern LROT_TABLE(bitlib);

extern LROT_TABLE(etxlib);
extern LROT_TABLE(etxcst);
extern LROT_TABLE(etxstr);
extern LROT_TABLE(etxdir);

extern LROT_TABLE(lcdlib);
extern LROT_TABLE(modellib);
extern LROT_TABLE(bitmaplib);

#if defined(COLORLCD)
extern LROT_TABLE(lvgllib);
extern LROT_TABLE(tablib);
#endif

/* _G __index -> rotables __index -> _index_hook_fct */
extern LROT_TABLE(rotables_meta);
extern LROT_TABLE(base_func);

static const ROTable* const _global_symbols[] = {
  LROT_TABLEREF(base_func),
  LROT_TABLEREF(etxlib),
  LROT_TABLEREF(etxdir),
  LROT_TABLEREF(etxcst),
  LROT_TABLEREF(etxstr),
  NULL,
};

static int _index_hook_fct(lua_State * L)
{
  const ROTable* const* t = _global_symbols;
  const TValue* res = luaO_nilobject;
  TString* key;

  lua_lock(L);
  key = rawtsvalue(L->top - 1);
  for(; *t; t++) {
    res = luaH_getstr((Table*)*t, key);
    if (!ttisnil(res)) break;
  }

  if (ttislightuserdata(res)) {
    /* strings are encoded as light user data */
    setsvalue2s(L, L->top - 1, luaS_new(L, pvalue(res)));
  } else {
    setobj2s(L, L->top - 1, res);
  }
  lua_unlock(L);

  return 1;
}

/* rotables is inserted in luaopen_base */
LROT_BEGIN(rotables_meta, NULL, LROT_MASK_INDEX)
  LROT_FUNCENTRY( __index, _index_hook_fct )
LROT_END(rotables_meta, NULL, LROT_MASK_INDEX)

LROT_BEGIN(rotables, LROT_TABLEREF(rotables_meta), 0)
  LROT_TABENTRY( _G, base_func)
  LROT_TABENTRY( io, iolib )
  LROT_TABENTRY( string, strlib )
  LROT_TABENTRY( math, mathlib )
  LROT_TABENTRY( bit32, bitlib )
  LROT_TABENTRY( lcd, lcdlib )
  LROT_TABENTRY( model, modellib )
  LROT_TABENTRY( bitmap, bitmaplib )
  LROT_TABENTRY( Bitmap, bitmaplib ) /* TODO: obsolete after 2.9 */
#if defined(COLORLCD)
  LROT_TABENTRY( lvgl, lvgllib )
  LROT_TABENTRY( table, tablib )
#endif
  LROT_TABENTRY( ROM, rotables )
LROT_END(rotables, LROT_TABLEREF(rotables_meta), 0)

LROT_BEGIN(lua_libs, NULL, 0)
  LROT_FUNCENTRY( _G,        luaopen_base )
  LROT_FUNCENTRY( io,        luaopen_io )
  LROT_FUNCENTRY( dir,       luaopen_etxdir )
  LROT_FUNCENTRY( bitmap_mt, luaopen_bitmap )
#if defined(COLORLCD)
  LROT_FUNCENTRY( lvgl_mt,   luaopen_lvgl )
  LROT_FUNCENTRY( package,   luaopen_package )
#endif
#if defined(LUA_ENABLE_STRLIB_MT)
  LROT_FUNCENTRY( string,    luaopen_string )
#endif
LROT_END(lua_libs, NULL, 0)


LUALIB_API void luaL_openlibs (lua_State *L) {
  const ROTable_entry *p = LROT_TABLEREF(lua_libs)->entry;
  /* Now do lua opens */
  for ( ; p->key; p++) {
    if (ttislcf(&p->value) && fvalue(&p->value))
      luaL_requiref(L, p->key, fvalue(&p->value), 1);
  }
}

