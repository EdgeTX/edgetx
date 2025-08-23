/*
** $Id: luaconf.h,v 1.259.1.1 2017/04/19 17:29:57 roberto Exp $
** Configuration file for Lua
** See Copyright Notice in lua.h
*/


#ifndef luaconf_h
#define luaconf_h

#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define USE_FATFS

// force ANSI mode: lua_number2integer() behaves the same way on all platforms (#3826)
#define LUA_ANSI

// #include "user_config.h"

#if __wasm__
	#define l_signalT long
#endif

// prevent localeconv()
#define getlocaledecpoint() ('.')

/*
** ===================================================================
** The EdgeTX Lua environment support 2 compilation targets:
**
** *  STM32 embedded runtimes which are compiled using
**    the ARM GCC cross-compiler toolchain.
**
** *  An extend version of 'luac' for cross-compiling Lua sources
**    for providing pre-compiled apps on SD card.  This is a command
**    line only and does not support any interactive dialogue or
**    dynamically loaded libraries.
**
** Note that we've removed the "how to fill this in comments so you
** can now see the actual content more easily.  Also the two big
** boilerplate conditional sections "Configuration for Numbers" and
** "Dependencies with C99 and other C details" have been moved to
** the end of the include file to keep the information dense content
** at the front.
** ===================================================================
*/
#if defined(NATIVE_TARGET)
# define LUA_USE_HOST
# define LUA_CROSS_COMPILER
#endif

#if !defined(LUA_USE_C89) && defined(_WIN32) && !defined(_WIN32_WCE)
#  define LUA_USE_WINDOWS  /* enable goodies for regular Windows */
#elif defined(__APPLE__)
#  define LUA_USE_MACOSX
#  define LUA_USE_POSIX
#elif defined(__linux__)
#  define LUA_USE_LINUX
#  define LUA_USE_POSIX
#endif

#define LUA_NODEMCU_NOCLOADERS

#define LUAI_BITSINT	32
#define LUA_32BITS

/* predefined options for LUA_INT_TYPE */
#define LUA_INT_INT		1
#define LUA_INT_LONG		2
#define LUA_INT_LONGLONG	3

/* predefined options for LUA_FLOAT_TYPE */
#define LUA_FLOAT_FLOAT		1
#define LUA_FLOAT_DOUBLE	2
#define LUA_FLOAT_LONGDOUBLE	3

#if defined(LUA_32BITS)		/* { */
/*
** 32-bit integers and 'float'
*/
#if LUAI_BITSINT >= 32  /* use 'int' if big enough */
#define LUA_INT_TYPE    LUA_INT_INT
#else
#define LUA_INT_TYPE	LUA_INT_LONG
#endif
#define LUA_FLOAT_TYPE  LUA_FLOAT_FLOAT

#elif defined(LUA_C89_NUMBERS)	/* }{ */
/*
** largest types available for C89 ('long' and 'double')
*/
#define LUA_INT_TYPE	LUA_INT_LONG
#define LUA_FLOAT_TYPE	LUA_FLOAT_DOUBLE

#endif				/* } */


/*
** default configuration for 64-bit Lua ('long long' and 'double')
*/
#if !defined(LUA_INT_TYPE)
#define LUA_INT_TYPE    LUA_INT_LONGLONG
#endif

#if !defined(LUA_FLOAT_TYPE)
#define LUA_FLOAT_TYPE  LUA_FLOAT_DOUBLE
#endif

/*
** Configuration for Paths.
**
** LUA_CPATH_DEFAULT is the default path that Lua uses to look for
** Dynamic C libraries are not used and ditto LUA_CPATH_DEFAULT
*/
#define LUA_PATH_SEP            ";"
#define LUA_PATH_MARK           "?"
#define LUA_EXEC_DIR            "!"

#define LUA_PATH_DEFAULT  "?.lua"

#if defined(_WIN32)
#define LUA_DIRSEP	"\\"
#else
#define LUA_DIRSEP	"/"
#endif

/*
** {==================================================================
** Marks for exported symbols in the C code
** ===================================================================
**
@@ LUA_API is a mark for all core API functions.
@@ LUALIB_API is a mark for all auxiliary library functions.
@@ LUAMOD_API is a mark for all standard library opening functions.
*/
#define LUA_API		extern
#define LUALIB_API	LUA_API
#define LUAMOD_API	LUALIB_API


/*
@@ LUAI_FUNC, LUAI_DDEF and LUAI_DDEC are used to mark visibilty when
** building lua as a shared library.  Used to tag private inter-module
** Lua internal functions.
*/
#define LUAI_FUNC	extern
#define LUAI_DDEC	LUAI_FUNC
#define LUAI_DDEF

/*
** {==================================================================
** Compatibility with previous versions
** ===================================================================
*/

/*
** We need to force floats to be converted to integers, even if
** not integral in nature. It allows most Edgetx API functions
** to accept unrounded floats when integers are required.
*/
#define LUA_FLOORN2I 1

#if defined(LUA_COMPAT_5_2)
#define LUA_COMPAT_MATHLIB     /* retains several deprecated functions in math. */
#define LUA_COMPAT_BITLIB      /* bit32 is separately implemented as a NodeMCU lib */
#define LUA_COMPAT_IPAIRS      /* enables __ipairs meta which isn't used in NodeMCU */
#define LUA_COMPAT_APIINTCASTS /* macros for manipulating other integer types */
#endif

/* Compatbililty for some API calls withdrawn in Lua53 */

#define lua_strlen(L,i)		lua_rawlen(L, (i))
#define lua_objlen(L,i)		lua_rawlen(L, (i))
#define lua_equal(L,idx1,idx2)		lua_compare(L,(idx1),(idx2),LUA_OPEQ)
#define lua_lessthan(L,idx1,idx2)	lua_compare(L,(idx1),(idx2),LUA_OPLT)

#define LUA_KCONTEXT	ptrdiff_t

/*
@@ lua_getlocaledecpoint gets the locale "radix character" (decimal point).
** Change that if you do not want to use C locales. (Code using this
** macro must include header 'locale.h'.)
*/
#define lua_getlocaledecpoint() '.'

// #define LUA_NOCVTN2S               // enable automatic coercion between
// #define LUA_NOCVTS2N               // strings and numbers
                                      //  of a function in debug information.

#if defined(LUA_USE_APICHECK)
#include <assert.h>
#define luai_apicheck(l,e)	assert(e)
#endif

#define LUA_EXTRASPACE	(sizeof(void *)) // raw memory area associated with a Lua state
#define LUAI_MAXSTACK   12000            // Maximum Lua stack size
#define LUA_IDSIZE	60               // Maximum size for the description of the source

#define LUAL_BUFFERSIZE	256              // EdgeTX setting because of stack limits

// #define LUA_QL(x)       "'" x "'"        // No longer used in lua53, but still used
// #define LUA_QS          LUA_QL("%s")     //  in some of our application modules

/*
** {==================================================================
** Other NodeMCU configuration.
** ===================================================================
*/

// #ifdef LUA_USE_ESP
// #define LUAI_USER_ALIGNMENT_T size_t
// #endif

#define LUAI_GCPAUSE	110  /* 110% (wait memory to grow 10% before next gc) */

/* }================================================================== */

/*
** {==================================================================
** Configuration for Numbers.
** Change these definitions if no predefined LUA_FLOAT_* / LUA_INT_*
** satisfy your needs.
** ===================================================================
**
@@ LUA_NUMBER is the floating-point type used by Lua.
@@ LUAI_UACNUMBER is the result of a 'default argument promotion' over a floating number.
@@ l_mathlim(x) corrects limit name 'x' to the proper float type by prefixing it with one of FLT/DBL/LDBL.
@@ LUA_NUMBER_FRMLEN is the length modifier for writing floats.
@@ LUA_NUMBER_FMT is the format for writing floats.
@@ lua_number2str converts a float to a string.
@@ l_mathop allows the addition of an 'l' or 'f' to all math operations.
@@ l_floor takes the floor of a float.
@@ lua_str2number converts a decimal numeric string to a number.
*/

/* The following definitions are good for most cases here */

#define l_floor(x)		(l_mathop(floor)(x))

#define lua_number2str(s,sz,n)  \
	l_sprintf((s), sz, LUA_NUMBER_FMT, (LUAI_UACNUMBER)(n))

/*
@@ lua_numbertointeger converts a float number to an integer, or
** returns 0 if float is not within the range of a lua_Integer.
** (The range comparisons are tricky because of rounding. The tests
** here assume a two-complement representation, where MININTEGER always
** has an exact representation as a float; MAXINTEGER may not have one,
** and therefore its conversion to float may have an ill-defined value.)
*/
#define lua_numbertointeger(n,p) \
  ((n) >= (LUA_NUMBER)(LUA_MININTEGER) && \
   (n) < -(LUA_NUMBER)(LUA_MININTEGER) && \
      (*(p) = (LUA_INTEGER)(n), 1))


/* now the variable definitions */

#if LUA_FLOAT_TYPE == LUA_FLOAT_FLOAT		/* { single float */

#define LUA_NUMBER	float

#define l_mathlim(n)		(FLT_##n)

#define LUAI_UACNUMBER	double

#define LUA_NUMBER_FRMLEN	""
#define LUA_NUMBER_FMT		"%.7g"

#define l_mathop(op)		op##f

#define lua_str2number(s,p)	strtof((s), (p))


#elif LUA_FLOAT_TYPE == LUA_FLOAT_LONGDOUBLE	/* }{ long double */

#define LUA_NUMBER	long double

#define l_mathlim(n)		(LDBL_##n)

#define LUAI_UACNUMBER	long double

#define LUA_NUMBER_FRMLEN	"L"
#define LUA_NUMBER_FMT		"%.19Lg"

#define l_mathop(op)		op##l

#define lua_str2number(s,p)	strtold((s), (p))

#elif LUA_FLOAT_TYPE == LUA_FLOAT_DOUBLE	/* }{ double */

#define LUA_NUMBER	double

#define l_mathlim(n)		(DBL_##n)

#define LUAI_UACNUMBER	double

#define LUA_NUMBER_FRMLEN	""
#define LUA_NUMBER_FMT		"%.14g"

#define l_mathop(op)		op

#define lua_str2number(s,p)	strtod((s), (p))

#else						/* }{ */

#error "numeric float type not defined"

#endif					/* } */
#define LUA_FLOAT	LUA_NUMBER


/*
@@ LUA_INTEGER is the integer type used by Lua.
**
@@ LUA_UNSIGNED is the unsigned version of LUA_INTEGER.
**
@@ LUAI_UACINT is the result of a 'default argument promotion'
@@ over a lUA_INTEGER.
@@ LUA_INTEGER_FRMLEN is the length modifier for reading/writing integers.
@@ LUA_INTEGER_FMT is the format for writing integers.
@@ LUA_MAXINTEGER is the maximum value for a LUA_INTEGER.
@@ LUA_MININTEGER is the minimum value for a LUA_INTEGER.
@@ lua_integer2str converts an integer to a string.
*/


/* The following definitions are good for most cases here */

#define LUA_INTEGER_FMT		"%" LUA_INTEGER_FRMLEN "d"

#define LUAI_UACINT		LUA_INTEGER

#define lua_integer2str(s,sz,n)  \
	l_sprintf((s), sz, LUA_INTEGER_FMT, (LUAI_UACINT)(n))

/*
** use LUAI_UACINT here to avoid problems with promotions (which
** can turn a comparison between unsigneds into a signed comparison)
*/
#define LUA_UNSIGNED		unsigned LUAI_UACINT


/* now the variable definitions */

#if LUA_INT_TYPE == LUA_INT_INT		/* { int */

#define LUA_INTEGER		int
#define LUA_INTEGER_FRMLEN	""

#define LUA_MAXINTEGER		INT_MAX
#define LUA_MININTEGER		INT_MIN

#elif LUA_INT_TYPE == LUA_INT_LONG	/* }{ long */

#define LUA_INTEGER		long
#define LUA_INTEGER_FRMLEN	"l"

#define LUA_MAXINTEGER		LONG_MAX
#define LUA_MININTEGER		LONG_MIN

#elif LUA_INT_TYPE == LUA_INT_LONGLONG	/* }{ long long */

/* use presence of macro LLONG_MAX as proxy for C99 compliance */
#if defined(LLONG_MAX)		/* { */
/* use ISO C99 stuff */

#define LUA_INTEGER		long long
#define LUA_INTEGER_FRMLEN	"ll"

#define LUA_MAXINTEGER		LLONG_MAX
#define LUA_MININTEGER		LLONG_MIN

#elif defined(LUA_USE_WINDOWS) /* }{ */
/* in Windows, can use specific Windows types */

#define LUA_INTEGER		__int64
#define LUA_INTEGER_FRMLEN	"I64"

#define LUA_MAXINTEGER		_I64_MAX
#define LUA_MININTEGER		_I64_MIN

#else				/* }{ */

#error "Compiler does not support 'long long'. Use option '-DLUA_32BITS' \
  or '-DLUA_C89_NUMBERS' (see file 'luaconf.h' for details)"

#endif				/* } */

#else				/* }{ */

#error "numeric integer type not defined"

#endif				/* } */

/* }================================================================== */


/*
** {==================================================================
** Dependencies with C99 and other C details
** ===================================================================
*/

/*
@@ l_sprintf is equivalent to 'snprintf' or 'sprintf' in C89.
** (All uses in Lua have only one format item.)
*/
#if !defined(LUA_USE_C89)
#define l_sprintf(s,sz,f,i)	snprintf(s,sz,f,i)
#else
#define l_sprintf(s,sz,f,i)	((void)(sz), sprintf(s,f,i))
#endif


/*
@@ lua_strx2number converts an hexadecimal numeric string to a number.
** In C99, 'strtod' does that conversion. Otherwise, you can
** leave 'lua_strx2number' undefined and Lua will provide its own
** implementation.
*/
#if !defined(LUA_USE_C89)
#define lua_strx2number(s,p)		lua_str2number(s,p)
#endif


/*
@@ lua_pointer2str converts a pointer to a readable string in a
** non-specified way.
*/
#define lua_pointer2str(buff,sz,p)	l_sprintf(buff,sz,"%p",p)


/*
@@ lua_number2strx converts a float to an hexadecimal numeric string.
** In C99, 'sprintf' (with format specifiers '%a'/'%A') does that.
** Otherwise, you can leave 'lua_number2strx' undefined and Lua will
** provide its own implementation.
*/
#if !defined(LUA_USE_C89)
#define lua_number2strx(L,b,sz,f,n)  \
	((void)L, l_sprintf(b,sz,f,(LUAI_UACNUMBER)(n)))
#endif


/*
** 'strtof' and 'opf' variants for math functions are not valid in
** C89. Otherwise, the macro 'HUGE_VALF' is a good proxy for testing the
** availability of these variants. ('math.h' is already included in
** all files that use these macros.)
*/
#if defined(LUA_USE_C89) || (defined(HUGE_VAL) && !defined(HUGE_VALF))
#undef l_mathop  /* variants not available */
#undef lua_str2number
#define l_mathop(op)		(lua_Number)op  /* no variant */
#define lua_str2number(s,p)	((lua_Number)strtod((s), (p)))
#endif

#undef lua_str2number
#define lua_str2number(s,p)	((lua_Number)strtod((s), (p)))

// For print
#include "debug.h"
#define lua_writestring(s,l)        TRACE_DEBUG_WP("%s", s);
#define lua_writeline()             TRACE_DEBUG_WP("\n");
#define lua_writestringerror(s,p)   TRACE_DEBUG_WP(s, p);

#endif
