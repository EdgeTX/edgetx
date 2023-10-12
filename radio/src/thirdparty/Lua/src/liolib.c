/*
** $Id: liolib.c,v 2.151.1.1 2017/04/19 17:29:57 roberto Exp $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

#define liolib_c
#define LUA_LIB

#include "lprefix.h"


#include <ctype.h>
#include <errno.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#include "FatFs/ff.h"

#define l_getc(f)               fatfs_getc(f)
#define l_lockfile(f)		((void)0)
#define l_unlockfile(f)		((void)0)

static int fatfs_getc(FIL* f) {
  char c; UINT result;
  if (f_read(f, &c, 1, &result) == FR_OK && result == 1) return c;
  return -1;
}

/*
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#if !defined(l_checkmode)

/* accepted extensions to 'mode' in 'fopen' */
#if !defined(L_MODEEXT)
#define L_MODEEXT	"b"
#endif

/* Check whether 'mode' matches '[rwa]%+?[L_MODEEXT]*' */
static int l_checkmode (const char *mode) {
  return (*mode != '\0' && strchr("rwa", *(mode++)) != NULL &&
         (*mode != '+' || (++mode, 1)) &&  /* skip if char is '+' */
         (strspn(mode, L_MODEEXT) == strlen(mode)));  /* check extensions */
}

#endif

/*
** {======================================================
** l_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#if !defined(l_popen)		/* { */

#if defined(LUA_USE_POSIX)	/* { */

#define l_popen(L,c,m)		(fflush(NULL), popen(c,m))
#define l_pclose(L,file)	(pclose(file))

#elif defined(LUA_USE_WINDOWS)	/* }{ */

#define l_popen(L,c,m)		(_popen(c,m))
#define l_pclose(L,file)	(_pclose(file))

#else				/* }{ */

/* ISO C definitions */
#define l_popen(L,c,m)  \
	  ((void)((void)c, m), \
	  luaL_error(L, "'popen' not supported"), \
	  (FILE*)0)
#define l_pclose(L,file)		((void)L, (void)file, -1)

#endif				/* } */

#endif				/* } */

/* }====================================================== */


#if !defined(l_getc)		/* { */

#if defined(LUA_USE_POSIX)
#define l_getc(f)		getc_unlocked(f)
#define l_lockfile(f)		flockfile(f)
#define l_unlockfile(f)		funlockfile(f)
#else
#define l_getc(f)		getc(f)
#define l_lockfile(f)		((void)0)
#define l_unlockfile(f)		((void)0)
#endif

#endif				/* } */


/*
** {======================================================
** l_fseek: configuration for longer offsets
** =======================================================
*/

#if !defined(l_fseek)		/* { */

#if defined(LUA_USE_POSIX)	/* { */

#include <sys/types.h>

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#elif defined(LUA_USE_WINDOWS) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)	/* }{ */

/* Windows (but not DDK) and Visual C++ 2005 or higher */
#define l_fseek(f,o,w)		_fseeki64(f,o,w)
#define l_ftell(f)		_ftelli64(f)
#define l_seeknum		__int64

#else				/* }{ */

/* ISO C definitions */
#define l_fseek(f,o,w)		fseek(f,o,w)
#define l_ftell(f)		ftell(f)
#define l_seeknum		long

#endif				/* } */

#endif				/* } */

/* }====================================================== */


#define IO_PREFIX	"_IO_"
#define IOPREF_LEN	(sizeof(IO_PREFIX)/sizeof(char) - 1)
#define IO_INPUT	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")

/*
** A file handle is a userdata with metatable 'LUA_FILEHANDLE' and
** initial structure 'LStream' (it may contain other fields
** after that initial structure).
*/

#define LUA_FILEHANDLE          "FILE*"

typedef struct LStream {
  FIL f;                 /* FatFs handle */
  lua_CFunction closef;  /* to close stream (NULL for closed streams) */
} LStream;

#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#define isclosed(p)	((p)->closef == NULL)


static int io_type (lua_State *L) {
  LStream *p;
  luaL_checkany(L, 1);
  p = (LStream *)luaL_testudata(L, 1, LUA_FILEHANDLE);
  if (p == NULL)
    lua_pushnil(L);  /* not a file */
  else if (isclosed(p))
    lua_pushliteral(L, "closed file");
  else
    lua_pushliteral(L, "file");
  return 1;
}


static FIL *tofile (lua_State *L) {
  LStream *p = tolstream(L);
  if (isclosed(p))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  return &p->f;
}


/*
** When creating file handles, always creates a 'closed' file handle
** before opening the actual file; so, if there is a memory error, the
** handle is in a consistent state.
*/
static LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdata(L, sizeof(LStream));
  p->closef = NULL;  /* mark file handle as 'closed' */
  luaL_setmetatable(L, LUA_FILEHANDLE);
  return p;
}


/*
** Calls the 'close' function from a file handle. The 'volatile' avoids
** a bug in some versions of the Clang compiler (e.g., clang 3.0 for
** 32 bits).
*/
static int aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  volatile lua_CFunction cf = p->closef;
  p->closef = NULL;  /* mark stream as closed */
  return (*cf)(L);  /* close it */
}


static int file_close (lua_State *L) {
  tofile(L);  /* make sure argument is an open stream */
  return aux_close(L);
}


static int io_close (lua_State *L) {
  if (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use standard output */
  return file_close(L);
}


static int file_gc (lua_State *L) {
  LStream *p = tolstream(L);
  if (!isclosed(p))
    aux_close(L);  /* ignore closed and incompletely open files */
  return 0;
}


/*
** function to close regular files
*/
static int io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  int res = f_close(&p->f);
  return luaL_fileresult(L, (res == 0), NULL);
}


static LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
  p->closef = &io_fclose;
  return p;
}

static inline BYTE fatfs_open_mode(const char *mode)
{
  if (mode[0] == 'w') {
    /* always create file and truncate it */
    return FA_WRITE | FA_CREATE_ALWAYS;
  } else if (mode[0] == 'a') {
    /* always open file (create it if necessary) */
    return FA_WRITE | FA_OPEN_ALWAYS;
  } else {
    return FA_READ;
  }
}

static void opencheck (lua_State *L, const char *fname, const char *mode) {
  LStream *p = newfile(L);
  if (f_open(&p->f, fname, fatfs_open_mode(mode)) != FR_OK)
    luaL_error(L, "cannot open file '%s' (%s)", fname, strerror(errno));
}


static int io_open (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  const char *mode = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
  const char *md = mode;  /* to traverse/check mode */
  luaL_argcheck(L, l_checkmode(md), 2, "invalid mode");
  return (f_open(&p->f, filename, fatfs_open_mode(mode)) != FR_OK)
             ? luaL_fileresult(L, 0, filename) : 1;
}

static FIL *getiofile (lua_State *L, const char *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  if (isclosed(p))
    luaL_error(L, "standard %s file is closed", findex + IOPREF_LEN);
  return &p->f;
}


static int g_iofile (lua_State *L, const char *f, const char *mode) {
  if (!lua_isnoneornil(L, 1)) {
    const char *filename = lua_tostring(L, 1);
    if (filename)
      opencheck(L, filename, mode);
    else {
      tofile(L);  /* check that it's a valid file handle */
      lua_pushvalue(L, 1);
    }
    lua_setfield(L, LUA_REGISTRYINDEX, f);
  }
  /* return current value */
  lua_getfield(L, LUA_REGISTRYINDEX, f);
  return 1;
}


static int io_input (lua_State *L) {
  return g_iofile(L, IO_INPUT, "r");
}


static int io_output (lua_State *L) {
  return g_iofile(L, IO_OUTPUT, "w");
}


static int io_readline (lua_State *L);


/*
** maximum number of arguments to 'f:lines'/'io.lines' (it + 3 must fit
** in the limit for upvalues of a closure)
*/
#define MAXARGLINE	250

static void aux_lines (lua_State *L, int toclose) {
  int n = lua_gettop(L) - 1;  /* number of arguments to read */
  luaL_argcheck(L, n <= MAXARGLINE, MAXARGLINE + 2, "too many arguments");
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  lua_rotate(L, 2, 2);  /* move 'n' and 'toclose' to their positions */
  lua_pushcclosure(L, io_readline, 3 + n);
}


static int file_lines (lua_State *L) {
  tofile(L);  /* check that it's a valid file handle */
  aux_lines(L, 0);
  return 1;
}


static int io_lines (lua_State *L) {
  int toclose;
  if (lua_isnone(L, 1)) lua_pushnil(L);  /* at least one argument */
  if (lua_isnil(L, 1)) {  /* no file name? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_INPUT);  /* get default input */
    lua_replace(L, 1);  /* put it at index 1 */
    tofile(L);  /* check that it's a valid file handle */
    toclose = 0;  /* do not close it after iteration */
  }
  else {  /* open a new file */
    const char *filename = luaL_checkstring(L, 1);
    opencheck(L, filename, "r");
    lua_replace(L, 1);  /* put file at index 1 */
    toclose = 1;  /* close it after iteration */
  }
  aux_lines(L, toclose);
  return 1;
}


/*
** {======================================================
** READ
** =======================================================
*/


static int test_eof (lua_State *L, FIL *f) {
  lua_pushliteral(L, "");
  return !f_eof(f);
}


static int read_line (lua_State *L, FIL *f, int chop) {
  luaL_Buffer b;
  int c = '\0';
  luaL_buffinit(L, &b);
  while (c != EOF && c != '\n') {  /* repeat until end of line */
    char *buff = luaL_prepbuffer(&b);  /* preallocate buffer */
    int i = 0;
    l_lockfile(f);  /* no memory errors can happen inside the lock */
    while (i < LUAL_BUFFERSIZE && (c = l_getc(f)) != EOF && c != '\n')
      buff[i++] = c;
    l_unlockfile(f);
    luaL_addsize(&b, i);
  }
  if (!chop && c == '\n')  /* want a newline and have one? */
    luaL_addchar(&b, c);  /* add ending newline to result */
  luaL_pushresult(&b);  /* close buffer */
  /* return ok if read something (either a newline or something else) */
  return (c == '\n' || lua_rawlen(L, -1) > 0);
}


static void read_all (lua_State *L, FIL *f) {
  UINT nr;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  do {  /* read file in chunks of LUAL_BUFFERSIZE bytes */
    char *p = luaL_prepbuffer(&b);
    if (f_read(f, p, LUAL_BUFFERSIZE, &nr) != FR_OK) nr = 0;
    luaL_addsize(&b, nr);
  } while (nr == LUAL_BUFFERSIZE);
  luaL_pushresult(&b);  /* close buffer */
}


static int read_chars (lua_State *L, FIL *f, size_t n) {
  UINT nr;  /* number of chars actually read */
  char *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  if (f_read(f, p, LUAL_BUFFERSIZE, &nr) != FR_OK) nr = 0; /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  return (nr > 0);  /* true iff read something */
}


static int g_read (lua_State *L, FIL *f, int first) {
  int nargs = lua_gettop(L) - 1;
  int success;
  int n;
  f_error(f) = 0;
  if (nargs == 0) {  /* no arguments? */
    success = read_line(L, f, 1);
    n = first+1;  /* to return 1 result */
  }
  else {  /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    for (n = first; nargs-- && success; n++) {
      if (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)luaL_checkinteger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      else {
        const char *p = luaL_checkstring(L, n);
        if (*p == '*') p++;  /* skip optional '*' (for compatibility) */
        switch (*p) {
          case 'l':  /* line */
            success = read_line(L, f, 1);
            break;
          case 'L':  /* line with end-of-line */
            success = read_line(L, f, 0);
            break;
          case 'a':  /* file */
            read_all(L, f);  /* read entire file */
            success = 1; /* always success */
            break;
          default:
            return luaL_argerror(L, n, "invalid format");
        }
      }
    }
  }
  if (f_error(f))
    return luaL_fileresult(L, 0, NULL);
  if (!success) {
    lua_pop(L, 1);  /* remove last result */
    lua_pushnil(L);  /* push nil instead */
  }
  return n - first;
}


static int io_read (lua_State *L) {
  return g_read(L, getiofile(L, IO_INPUT), 1);
}


static int file_read (lua_State *L) {
  return g_read(L, tofile(L), 2);
}


static int io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  int i;
  int n = (int)lua_tointeger(L, lua_upvalueindex(2));
  if (isclosed(p))  /* file is already closed? */
    return luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  luaL_checkstack(L, n, "too many arguments");
  for (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, &p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  if (lua_toboolean(L, -n))  /* read at least one value? */
    return n;  /* return them */
  else {  /* first result is nil: EOF or error */
    if (n > 1) {  /* is there error information? */
      /* 2nd result is error message */
      return luaL_error(L, "%s", lua_tostring(L, -n + 1));
    }
    if (lua_toboolean(L, lua_upvalueindex(3))) {  /* generator created file? */
      lua_settop(L, 0);
      lua_pushvalue(L, lua_upvalueindex(1));
      aux_close(L);  /* close it */
    }
    return 0;
  }
}

/* }====================================================== */

#define MAX_FORMAT 32

static int g_write (lua_State *L, FIL *f, int arg) {
  int nargs = lua_gettop(L) - arg;
  int status = 1;
  for (; nargs--; arg++) {
    if (lua_type(L, arg) == LUA_TNUMBER) {
      /* optimization: could be done exactly as for strings */
      char buff[MAX_FORMAT];
      int len = lua_isinteger(L, arg)
        ? lua_integer2str(buff, MAX_FORMAT, (LUAI_UACINT)lua_tointeger(L, arg))
        : lua_number2str(buff, MAX_FORMAT, (LUAI_UACINT)lua_tonumber(L, arg));
      UINT w;
      status = status && (f_write(f, buff, len, &w) == FR_OK && w == len);
    }
    else {
      size_t l; UINT w;
      const char *s = luaL_checklstring(L, arg, &l);
      status = status && (f_write(f, s, l, &w) == FR_OK && w == l);
    }
  }
  if (status) return 1;  /* file handle already on stack top */
  else return luaL_fileresult(L, status, NULL);
}


static int io_write (lua_State *L) {
  return g_write(L, getiofile(L, IO_OUTPUT), 1);
}


static int file_write (lua_State *L) {
  FIL *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  return g_write(L, f, 2);
}


static int file_seek (lua_State *L) {
  static const char *const modenames[] = {"set", "cur", "end", NULL};
  FIL *f = tofile(L);
  int op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Integer p3 = luaL_optinteger(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Integer)offset == p3, 3,
                  "not an integer in proper range");
  if (op == 1) {
    offset += (l_seeknum)f_tell(f);
  } else if (op == 2) {
    offset += (l_seeknum)f_size(f);
  }
  if (f_lseek(f, offset) != FR_OK)
    return luaL_fileresult(L, 0, NULL);  /* error */
  else {
    lua_pushinteger(L, (lua_Integer)f_tell(f));
    return 1;
  }
}


/*
** functions for 'io' library
*/
LROT_BEGIN(iolib, NULL, 0)
  LROT_FUNCENTRY( close, io_close )
  LROT_FUNCENTRY( input, io_input )
  LROT_FUNCENTRY( lines, io_lines )
  LROT_FUNCENTRY( open, io_open )
  LROT_FUNCENTRY( output, io_output )
  LROT_FUNCENTRY( read, io_read )
  LROT_FUNCENTRY( type, io_type )
  LROT_FUNCENTRY( write, io_write )
LROT_END(iolib, NULL, 0)

/*
** methods for file handles
**
** we define only garbage collector to close any leftover open files
*/
LROT_BEGIN(file_handle, NULL, LROT_MASK_GC)
  LROT_FUNCENTRY( close, file_close )
  LROT_FUNCENTRY( lines, file_lines )
  LROT_FUNCENTRY( read,  file_read )
  LROT_FUNCENTRY( seek,  file_seek )
  LROT_FUNCENTRY( write,  file_write )
  LROT_FUNCENTRY( __gc, file_gc )
LROT_END(file_handle, NULL, LROT_MASK_GC)

LUAMOD_API int luaopen_io (lua_State *L) {
  luaL_rometatable( L, LUA_FILEHANDLE,  LROT_TABLEREF(file_handle));
  return 0;
}
