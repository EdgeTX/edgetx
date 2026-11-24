/*
** $Id: lmathlib.c,v 1.119.1.1 2017/04/19 17:20:42 roberto Exp $
** Standard mathematical library
** See Copyright Notice in lua.h
*/

#define lmathlib_c
#define LUA_LIB

#include "lprefix.h"


#include <stdlib.h>
#include <math.h>
#include <stdint.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#undef PI
#define PI	(l_mathop(3.141592653589793238462643383279502884))


/*
** xoshiro128** PRNG (as Lua 5.4, 32-bit variant). Fixed state at boot,
** so sequences are deterministic until math.randomseed() is called.
*/
static uint32_t rng_state[4] = {0x9E3779B9u, 0x243F6A88u, 0xB7E15162u, 0x6A09E667u};

static uint32_t rotl32 (uint32_t x, int n) {
  return (x << n) | (x >> (32 - n));
}

static uint32_t rng_next (void) {
  uint32_t *s = rng_state;
  uint32_t r = rotl32(s[1] * 5, 7) * 9;
  uint32_t t = s[1] << 9;
  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];
  s[2] ^= t;
  s[3] = rotl32(s[3], 11);
  return r;
}

static lua_Unsigned rng_unsigned (void) {
#if LUA_MAXINTEGER > 0x7FFFFFFF
  lua_Unsigned hi = rng_next();
  return (hi << 31 << 1) | rng_next();
#else
  return rng_next();
#endif
}


static int math_abs (lua_State *L) {
  if (lua_isinteger(L, 1)) {
    lua_Integer n = lua_tointeger(L, 1);
    if (n < 0) n = (lua_Integer)(0u - (lua_Unsigned)n);
    lua_pushinteger(L, n);
  }
  else
    lua_pushnumber(L, l_mathop(fabs)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_sin (lua_State *L) {
  lua_pushnumber(L, l_mathop(sin)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_cos (lua_State *L) {
  lua_pushnumber(L, l_mathop(cos)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_tan (lua_State *L) {
  lua_pushnumber(L, l_mathop(tan)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_asin (lua_State *L) {
  lua_pushnumber(L, l_mathop(asin)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_acos (lua_State *L) {
  lua_pushnumber(L, l_mathop(acos)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_atan (lua_State *L) {
  lua_Number y = luaL_checknumber(L, 1);
  lua_Number x = luaL_optnumber(L, 2, 1);
  lua_pushnumber(L, l_mathop(atan2)(y, x));
  return 1;
}


static int math_toint (lua_State *L) {
  int valid;
  lua_Integer n = lua_tointegerx(L, 1, &valid);
  if (valid)
    lua_pushinteger(L, n);
  else {
    luaL_checkany(L, 1);
    lua_pushnil(L);  /* value is not convertible to integer */
  }
  return 1;
}


static void pushnumint (lua_State *L, lua_Number d) {
  lua_Integer n;
  if (lua_numbertointeger(d, &n))  /* does 'd' fit in an integer? */
    lua_pushinteger(L, n);  /* result is integer */
  else
    lua_pushnumber(L, d);  /* result is float */
}


static int math_floor (lua_State *L) {
  if (lua_isinteger(L, 1))
    lua_settop(L, 1);  /* integer is its own floor */
  else {
    lua_Number d = l_mathop(floor)(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  return 1;
}


static int math_ceil (lua_State *L) {
  if (lua_isinteger(L, 1))
    lua_settop(L, 1);  /* integer is its own ceil */
  else {
    lua_Number d = l_mathop(ceil)(luaL_checknumber(L, 1));
    pushnumint(L, d);
  }
  return 1;
}


static int math_fmod (lua_State *L) {
  if (lua_isinteger(L, 1) && lua_isinteger(L, 2)) {
    lua_Integer d = lua_tointeger(L, 2);
    if ((lua_Unsigned)d + 1u <= 1u) {  /* special cases: -1 or 0 */
      luaL_argcheck(L, d != 0, 2, "zero");
      lua_pushinteger(L, 0);  /* avoid overflow with 0x80000... / -1 */
    }
    else
      lua_pushinteger(L, lua_tointeger(L, 1) % d);
  } else {
    lua_Number m, a=luaL_checknumber(L, 1), b=luaL_checknumber(L, 2);
    if (b==0) luaL_error(L,"modulo by zero");
    m = a/b;
    lua_pushnumber(L, a - b*(m > 0.0f ? l_mathop(floor)(m) : l_mathop(ceil)(m)));
  }
  return 1;
}


/*
** next function does not use 'modf', avoiding problems with 'double*'
** (which is not compatible with 'float*') when lua_Number is not
** 'double'.
*/
static int math_modf (lua_State *L) {
  if (lua_isinteger(L ,1)) {
    lua_settop(L, 1);  /* number is its own integer part */
    lua_pushnumber(L, 0);  /* no fractional part */
  }
  else {
    lua_Number n = luaL_checknumber(L, 1);
    /* integer part (rounds toward zero) */
    lua_Number ip = (n < 0) ? l_mathop(ceil)(n) : l_mathop(floor)(n);
    pushnumint(L, ip);
    /* fractional part (test needed for inf/-inf) */
    lua_pushnumber(L, (n == ip) ? l_mathop(0.0) : (n - ip));
  }
  return 2;
}


static int math_sqrt (lua_State *L) {
  lua_pushnumber(L, l_mathop(sqrt)(luaL_checknumber(L, 1)));
  return 1;
}


static int math_ult (lua_State *L) {
  lua_Integer a = luaL_checkinteger(L, 1);
  lua_Integer b = luaL_checkinteger(L, 2);
  lua_pushboolean(L, (lua_Unsigned)a < (lua_Unsigned)b);
  return 1;
}

static int math_log (lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number res;
  if (lua_isnoneornil(L, 2))
    res = l_mathop(log)(x);
  else {
    lua_Number base = luaL_checknumber(L, 2);
#if !defined(LUA_USE_C89)
    if (base == l_mathop(2.0))
      res = l_mathop(log2)(x); else
#endif
    if (base == l_mathop(10.0))
      res = l_mathop(log10)(x);
    else
      res = l_mathop(log)(x)/l_mathop(log)(base);
  }
  lua_pushnumber(L, res);
  return 1;
}

static int math_exp (lua_State *L) {
  lua_pushnumber(L, l_mathop(exp)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_deg (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (l_mathop(180.0) / PI));
  return 1;
}

static int math_rad (lua_State *L) {
  lua_pushnumber(L, luaL_checknumber(L, 1) * (PI / l_mathop(180.0)));
  return 1;
}


static int math_min (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int imin = 1;  /* index of current minimum value */
  int i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (lua_compare(L, i, imin, LUA_OPLT))
      imin = i;
  }
  lua_pushvalue(L, imin);
  return 1;
}


static int math_max (lua_State *L) {
  int n = lua_gettop(L);  /* number of arguments */
  int imax = 1;  /* index of current maximum value */
  int i;
  luaL_argcheck(L, n >= 1, 1, "value expected");
  for (i = 2; i <= n; i++) {
    if (lua_compare(L, imax, i, LUA_OPLT))
      imax = i;
  }
  lua_pushvalue(L, imax);
  return 1;
}

static int math_random (lua_State *L) {
  lua_Integer low, up;
  lua_Unsigned n, lim, r;
  switch (lua_gettop(L)) {  /* check number of arguments */
    case 0: {  /* no arguments: float in [0,1) with 24 random bits */
      lua_pushnumber(L, (lua_Number)(rng_next() >> 8) * l_mathop(5.9604644775390625e-8));  /* 2^-24 */
      return 1;
    }
    case 1: {  /* only upper limit */
      low = 1;
      up = luaL_checkinteger(L, 1);
      break;
    }
    case 2: {  /* lower and upper limits */
      low = luaL_checkinteger(L, 1);
      up = luaL_checkinteger(L, 2);
      break;
    }
    default: return luaL_error(L, "wrong number of arguments");
  }
  /* random integer in the interval [low, up] */
  luaL_argcheck(L, low <= up, 1, "interval is empty");
  luaL_argcheck(L, low >= 0 || up <= LUA_MAXINTEGER + low, 1,
                   "interval too large");
  /* unbiased: mask to the smallest 2^b-1 >= n, reject (< 2 tries on average) */
  n = (lua_Unsigned)up - (lua_Unsigned)low;
  lim = n;
  lim |= lim >> 1; lim |= lim >> 2; lim |= lim >> 4; lim |= lim >> 8;
  lim |= lim >> 16;
#if LUA_MAXINTEGER > 0x7FFFFFFF
  lim |= lim >> 31 >> 1;
#endif
  while ((r = rng_unsigned() & lim) > n) {}
  lua_pushinteger(L, (lua_Integer)(r + (lua_Unsigned)low));
  return 1;
}


static int math_randomseed (lua_State *L) {
  int ok;
  lua_Integer n = lua_tointegerx(L, 1, &ok);
  int i;
  if (!ok) luaL_checknumber(L, 1);  /* out-of-range floats seed 0 */
  rng_state[0] = (uint32_t)n;
  rng_state[1] = 0xff;  /* avoid an all-zero state */
  rng_state[2] = 0;
  rng_state[3] = 0;
  for (i = 0; i < 16; i++) rng_next();  /* discard initial values */
  return 0;
}


static int math_type (lua_State *L) {
  if (lua_type(L, 1) == LUA_TNUMBER) {
      if (lua_isinteger(L, 1))
        lua_pushliteral(L, "integer");
      else
        lua_pushliteral(L, "float");
  }
  else {
    luaL_checkany(L, 1);
    lua_pushnil(L);
  }
  return 1;
}


/*
** {==================================================================
** Deprecated functions (for compatibility only)
** ===================================================================
*/
#if defined(LUA_COMPAT_MATHLIB)

static int math_cosh (lua_State *L) {
  lua_pushnumber(L, l_mathop(cosh)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_sinh (lua_State *L) {
  lua_pushnumber(L, l_mathop(sinh)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_tanh (lua_State *L) {
  lua_pushnumber(L, l_mathop(tanh)(luaL_checknumber(L, 1)));
  return 1;
}

static int math_pow (lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  lua_Number y = luaL_checknumber(L, 2);
  lua_pushnumber(L, l_mathop(pow)(x, y));
  return 1;
}

static int math_frexp (lua_State *L) {
  int e;
  lua_pushnumber(L, l_mathop(frexp)(luaL_checknumber(L, 1), &e));
  lua_pushinteger(L, e);
  return 2;
}

static int math_ldexp (lua_State *L) {
  lua_Number x = luaL_checknumber(L, 1);
  int ep = (int)luaL_checkinteger(L, 2);
  lua_pushnumber(L, l_mathop(ldexp)(x, ep));
  return 1;
}

static int math_log10 (lua_State *L) {
  lua_pushnumber(L, l_mathop(log10)(luaL_checknumber(L, 1)));
  return 1;
}

#endif
/* }================================================================== */


LROT_BEGIN(mathlib, NULL, 0)
  LROT_FUNCENTRY( abs,   math_abs)
  LROT_FUNCENTRY( acos,  math_acos)
  LROT_FUNCENTRY( asin,  math_asin)
  LROT_FUNCENTRY( atan,  math_atan)
  LROT_FUNCENTRY( ceil,  math_ceil)
  LROT_FUNCENTRY( cos,   math_cos)
  LROT_FUNCENTRY( deg,   math_deg)
  LROT_FUNCENTRY( exp,   math_exp)
  LROT_FUNCENTRY( tointeger, math_toint)
  LROT_FUNCENTRY( floor, math_floor)
  LROT_FUNCENTRY( fmod,  math_fmod)
  LROT_FUNCENTRY( ult,   math_ult)
  LROT_FUNCENTRY( log,   math_log)
  LROT_FUNCENTRY( max,   math_max)
  LROT_FUNCENTRY( min,   math_min)
  LROT_FUNCENTRY( modf,  math_modf)
  LROT_FUNCENTRY( rad,   math_rad)
  LROT_FUNCENTRY( random, math_random)
  LROT_FUNCENTRY( randomseed, math_randomseed)
  LROT_FUNCENTRY( sin,   math_sin)
  LROT_FUNCENTRY( sqrt,  math_sqrt)
  LROT_FUNCENTRY( tan,   math_tan)
  LROT_FUNCENTRY( type,  math_type)
#if defined(LUA_COMPAT_MATHLIB)
  LROT_FUNCENTRY( atan2, math_atan)
  LROT_FUNCENTRY( cosh,  math_cosh)
  LROT_FUNCENTRY( sinh,  math_sinh)
  LROT_FUNCENTRY( tanh,  math_tanh)
  LROT_FUNCENTRY( pow,   math_pow)
  LROT_FUNCENTRY( frexp, math_frexp)
  LROT_FUNCENTRY( ldexp, math_ldexp)
  LROT_FUNCENTRY( log10, math_log10)
#endif
  LROT_FLOATENTRY( pi, PI)
  LROT_FLOATENTRY( huge,(lua_Number)HUGE_VAL)
  LROT_INTENTRY( maxinteger, LUA_MAXINTEGER)
  LROT_INTENTRY( mininteger, LUA_MININTEGER)
LROT_END(mathlib, NULL, 0)


/*
** Open math library
*/
LUAMOD_API int luaopen_math (lua_State *L) {
  return 0;
}

