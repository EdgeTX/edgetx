#ifndef lro_defs_h
#define lro_defs_h

#include "lua.h"
#include "lobject.h"
#include "llimits.h"
#include "ltm.h"

/* Macros used to declare rotable entries */

#define LRO_STRKEY(k)        k
#define LRO_FUNCVAL(v)       {{.f = v}, LUA_TLCF}
#define LRO_LUDATA(v)        {{.p = (void *) v}, LUA_TLIGHTUSERDATA}
#define LRO_NILVAL           {{.p = NULL}, LUA_TNIL}
#define LRO_NUMVAL(v)        {{.i = (lua_Integer)(v)}, LUA_TNUMINT}
#define LRO_INTVAL(v)        LRO_NUMVAL(v)
#define LRO_FLOATVAL(v)      {{.n = v}, LUA_TNUMFLT}
#define LRO_ROVAL(v)         {{.gc = cast(GCObject *, &(v ## _ROTable))}, LUA_TTBLROF}

#define LROT_MARKED          0 //<<<<<<<<<<  *** TBD *** >>>>>>>>>>>

#define LROT_FUNCENTRY(n,f)  {LRO_STRKEY(#n), LRO_FUNCVAL(f)},
#define LROT_LUDENTRY(n,x)   {LRO_STRKEY(#n), LRO_LUDATA(x)},
#define LROT_NUMENTRY(n,x)   {LRO_STRKEY(#n), LRO_NUMVAL(x)},
#define LROT_INTENTRY(n,x)   LROT_NUMENTRY(n,x)
#define LROT_FLOATENTRY(n,x) {LRO_STRKEY(#n), LRO_FLOATVAL(x)},
#define LROT_TABENTRY(n,t)   {LRO_STRKEY(#n), LRO_ROVAL(t)},

#define LROT_TABLE(rt)       const ROTable rt ## _ROTable
#define LROT_ENTRYREF(rt)    (rt ##_entries)
#define LROT_TABLEREF(rt)    (&rt ##_ROTable)
#define LROT_BEGIN(rt,mt,f)  extern LROT_TABLE(rt); \
  static const ROTable_entry rt ## _entries[] = {
#define LROT_END(rt,mt,f)    {NULL, LRO_NILVAL} }; \
  const ROTable rt ## _ROTable = { \
    (GCObject *)1, LUA_TTBLROF, LROT_MARKED, \
    cast(lu_byte, ~(f)), (sizeof(rt ## _entries)/sizeof(ROTable_entry)) - 1, \
    cast(Table *, mt), cast(const ROTable_entry *, rt ## _entries) };
#define LROT_BREAK(rt)       };

#define LROT_MASK(m)         cast(lu_byte, 1<<TM_ ## m)

/*
 * These are statically coded can be any combination of the fast index tags
 * listed in ltm.h: EQ, GC, INDEX, LEN, MODE, NEWINDEX or combined by anding
 * GC+INDEX is the only common combination used, hence the combinaton macro
 */
#define LROT_MASK_EQ         LROT_MASK(EQ)
#define LROT_MASK_GC         LROT_MASK(GC)
#define LROT_MASK_INDEX      LROT_MASK(INDEX)
#define LROT_MASK_LEN        LROT_MASK(LEN)
#define LROT_MASK_MODE       LROT_MASK(MODE)
#define LROT_MASK_NEWINDEX   LROT_MASK(NEWINDEX)
#define LROT_MASK_GC_INDEX   (LROT_MASK_GC | LROT_MASK_INDEX)

#endif
