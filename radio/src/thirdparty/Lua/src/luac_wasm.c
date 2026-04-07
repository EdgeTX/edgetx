/*
** EdgeTX Lua cross-compiler — WASM entry point
**
** Exposes a minimal API for compiling Lua source to bytecode entirely
** in memory, suitable for running in a browser via WebAssembly.
**
** Exported functions:
**   compile(src_ptr, src_len, strip) → positive length on success, negative on error
**   get_output_ptr()                 → pointer to compiled bytecode
**   get_error_ptr()                  → pointer to error message string
**   get_error_len()                  → length of error message
**   wasm_malloc(size)                → allocate memory (for JS to write source into)
**   wasm_free(ptr)                   → free allocated memory
*/

#define luac_c
#define LUA_CORE

#include "lprefix.h"

#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"

/* --------------- output buffer --------------- */

static unsigned char *out_buf;
static size_t         out_len;
static size_t         out_cap;

static int writer(lua_State *L, const void *p, size_t size, void *ud) {
  (void)L; (void)ud;
  if (out_len + size > out_cap) {
    size_t new_cap = out_cap ? out_cap * 2 : 4096;
    while (new_cap < out_len + size)
      new_cap *= 2;
    unsigned char *nb = realloc(out_buf, new_cap);
    if (!nb) return 1;
    out_buf = nb;
    out_cap = new_cap;
  }
  memcpy(out_buf + out_len, p, size);
  out_len += size;
  return 0;
}

/* --------------- error message --------------- */

static char error_msg[512];
static int  error_len;

/* --------------- exported API --------------- */

#define EXPORT __attribute__((export_name(#name)))
/* clang wasm export attribute helper */
#define WASM_EXPORT(name) __attribute__((export_name(#name))) name

/*
** Compile Lua source to bytecode.
** Returns the bytecode length (>0) on success, or a negative error code:
**   -1  could not create Lua state
**   -2  syntax / compile error (call get_error_ptr/len for message)
**   -3  bytecode dump failed
*/
int WASM_EXPORT(compile)(const char *source, int source_len, int strip) {
  error_msg[0] = '\0';
  error_len = 0;

  /* reset output */
  free(out_buf);
  out_buf = NULL;
  out_len = 0;
  out_cap = 0;

  lua_State *L = luaL_newstate();
  if (!L) return -1;

  int status = luaL_loadbuffer(L, source, (size_t)source_len, "input");
  if (status != LUA_OK) {
    const char *err = lua_tostring(L, -1);
    if (err) {
      size_t n = strlen(err);
      if (n >= sizeof(error_msg)) n = sizeof(error_msg) - 1;
      memcpy(error_msg, err, n);
      error_msg[n] = '\0';
      error_len = (int)n;
    }
    lua_close(L);
    return -2;
  }

  /* The compiled chunk is a function at the top of the stack.
   * lua_dump serialises it via the writer callback. */
  status = lua_dump(L, writer, NULL, strip);
  lua_close(L);

  return status == 0 ? (int)out_len : -3;
}

unsigned char *WASM_EXPORT(get_output_ptr)(void) {
  return out_buf;
}

char *WASM_EXPORT(get_error_ptr)(void) {
  return error_msg;
}

int WASM_EXPORT(get_error_len)(void) {
  return error_len;
}

void *WASM_EXPORT(wasm_malloc)(int size) {
  return malloc((size_t)size);
}

void WASM_EXPORT(wasm_free)(void *ptr) {
  free(ptr);
}
