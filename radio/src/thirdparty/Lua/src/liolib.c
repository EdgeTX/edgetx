/*
** $Id: liolib.c,v 2.111 2013/03/21 13:57:27 roberto Exp $
** Standard I/O (and system) library
** See Copyright Notice in lua.h
*/

/*
** POSIX idiosyncrasy!
** This definition must come before the inclusion of 'stdio.h'; it
** should not affect non-POSIX systems
*/
#if !defined(_FILE_OFFSET_BITS)
#define _FILE_OFFSET_BITS 64
#endif

#define liolib_c
#define LUA_LIB

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"

#if defined(USE_FATFS)
  #define FILE FIL
#endif

#if !defined(lua_checkmode)

/*
** Check whether 'mode' matches '[rwa]%+?b?'.
** Change this macro to accept other modes for 'fopen' besides
** the standard ones.
*/
#define lua_checkmode(mode) \
	(*mode != '\0' && strchr("rwa", *(mode++)) != NULL &&	\
	(*mode != '+' || ++mode) &&  /* skip if char is '+' */	\
	(*mode != 'b' || ++mode) &&  /* skip if char is 'b' */	\
	(*mode == '\0'))

#endif

/*
** {======================================================
** lua_popen spawns a new process connected to the current
** one through the file streams.
** =======================================================
*/

#if !defined(lua_popen)	/* { */

#if defined(LUA_USE_POPEN)	/* { */

#define lua_popen(L,c,m)	((void)L, fflush(NULL), popen(c,m))
#define lua_pclose(L,file)	((void)L, pclose(file))

#elif defined(LUA_WIN)		/* }{ */

#define lua_popen(L,c,m)		((void)L, _popen(c,m))
#define lua_pclose(L,file)		((void)L, _pclose(file))


#else				/* }{ */

#define lua_popen(L,c,m)		((void)((void)c, m),  \
		luaL_error(L, LUA_QL("popen") " not supported"), (FILE*)0)
#define lua_pclose(L,file)		((void)((void)L, file), -1)


#endif				/* } */

#endif			/* } */

/* }====================================================== */


/*
** {======================================================
** lua_fseek/lua_ftell: configuration for longer offsets
** =======================================================
*/

#if !defined(lua_fseek)	/* { */

#if defined(LUA_USE_POSIX)

#define l_fseek(f,o,w)		fseeko(f,o,w)
#define l_ftell(f)		ftello(f)
#define l_seeknum		off_t

#elif defined(LUA_WIN) && !defined(_CRTIMP_TYPEINFO) \
   && defined(_MSC_VER) && (_MSC_VER >= 1400)
/* Windows (but not DDK) and Visual C++ 2005 or higher */

#define l_fseek(f,o,w)		_fseeki64(f,o,w)
#define l_ftell(f)		_ftelli64(f)
#define l_seeknum		__int64

#else

#define l_fseek(f,o,w)		fseek(f,o,w)
#define l_ftell(f)		ftell(f)
#define l_seeknum		long

#endif

#endif			/* } */

/* }====================================================== */


#define IO_PREFIX	"_IO_"
#define IO_INPUT	(IO_PREFIX "input")
#define IO_OUTPUT	(IO_PREFIX "output")


typedef luaL_Stream LStream;


#define tolstream(L)	((LStream *)luaL_checkudata(L, 1, LUA_FILEHANDLE))

#if !defined(USE_FATFS)

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


static int f_tostring (lua_State *L) {
  LStream *p = tolstream(L);
  if (isclosed(p))
    lua_pushliteral(L, "file (closed)");
  else
    lua_pushfstring(L, "file (%p)", p->f);
  return 1;
}

#endif

static FILE *tofile (lua_State *L) {
  LStream *p = tolstream(L);
#if defined(USE_FATFS)
  return &p->f;
#else
  if (isclosed(p))
    luaL_error(L, "attempt to use a closed file");
  lua_assert(p->f);
  return p->f;
#endif
}


/*
** When creating file handles, always creates a `closed' file handle
** before opening the actual file; so, if there is a memory error, the
** file is not left opened.
*/
static LStream *newprefile (lua_State *L) {
  LStream *p = (LStream *)lua_newuserdata(L, sizeof(LStream));
#if !defined(USE_FATFS)
  p->closef = NULL;  /* mark file handle as 'closed' */
#endif
  luaL_setmetatable(L, LUA_FILEHANDLE);
  return p;
}

#if !defined(USE_FATFS)
static int aux_close (lua_State *L) {
  LStream *p = tolstream(L);
  lua_CFunction cf = p->closef;
  p->closef = NULL;  /* mark stream as closed */
  return (*cf)(L);  /* close it */
}
#endif

static int io_close (lua_State *L) {
#if defined(USE_FATFS)
  f_close(tofile(L));
  return 0;
#else
  if (lua_isnone(L, 1))  /* no argument? */
    lua_getfield(L, LUA_REGISTRYINDEX, IO_OUTPUT);  /* use standard output */
  tofile(L);  /* make sure argument is an open stream */
  return aux_close(L);
#endif
}

static int f_gc (lua_State *L) {
#if !defined(USE_FATFS)
  LStream *p = tolstream(L);
  if (!isclosed(p) && p->f != NULL)
    aux_close(L);  /* ignore closed and incompletely open files */
#endif
  f_close(tofile(L));  // no need to check if file was already closed (fatfs will not close it if p->f->fs is 0)
  return 0;
}

#if !defined(USE_FATFS)
/*
** function to close regular files
*/
static int io_fclose (lua_State *L) {
  LStream *p = tolstream(L);
  int res = fclose(p->f);
  return luaL_fileresult(L, (res == 0), NULL);
}
#endif

static LStream *newfile (lua_State *L) {
  LStream *p = newprefile(L);
#if !defined(USE_FATFS)
  p->f = NULL;
  p->closef = &io_fclose;
#endif
  return p;
}

#if !defined(USE_FATFS)
static void opencheck (lua_State *L, const char *fname, const char *mode) {
  LStream *p = newfile(L);
  p->f = fopen(fname, mode);
  if (p->f == NULL)
    luaL_error(L, "cannot open file " LUA_QS " (%s)", fname, strerror(errno));
}
#endif

static int io_open (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  const char *md = luaL_optstring(L, 2, "r");
  LStream *p = newfile(L);
#if defined(USE_FATFS)
  BYTE mode = FA_READ;
  if (*md == 'w')
    mode = FA_WRITE | FA_CREATE_ALWAYS;     // always create file and truncate it
  else if (*md == 'a')
    mode = FA_WRITE | FA_OPEN_ALWAYS;       // always open file (create it if necessary) 
  FRESULT result = f_open(&p->f, filename, mode);
  if (result == FR_OK) {
    if (*md == 'a')
      f_lseek(&p->f, f_size(&p->f));   // seek to the end of the file
    return 1;
  }
  else {
    return luaL_fileresult(L, 0, filename);
  }
#else
  const char *mode = md;  /* to traverse/check mode */
  luaL_argcheck(L, lua_checkmode(mode), 2, "invalid mode");
  p->f = fopen(filename, md);
  return (p->f == NULL) ? luaL_fileresult(L, 0, filename) : 1;
#endif
}

#if !defined(USE_FATFS)

/*
** function to close 'popen' files
*/
static int io_pclose (lua_State *L) {
  LStream *p = tolstream(L);
  return luaL_execresult(L, lua_pclose(L, p->f));
}

static int io_popen (lua_State *L) {
  const char *filename = luaL_checkstring(L, 1);
  const char *mode = luaL_optstring(L, 2, "r");
  LStream *p = newprefile(L);
  p->f = lua_popen(L, filename, mode);
  p->closef = &io_pclose;
  return (p->f == NULL) ? luaL_fileresult(L, 0, filename) : 1;
}


static int io_tmpfile (lua_State *L) {
  LStream *p = newfile(L);
  p->f = tmpfile();
  return (p->f == NULL) ? luaL_fileresult(L, 0, NULL) : 1;
}


static FILE *getiofile (lua_State *L, const char *findex) {
  LStream *p;
  lua_getfield(L, LUA_REGISTRYINDEX, findex);
  p = (LStream *)lua_touserdata(L, -1);
  if (isclosed(p))
    luaL_error(L, "standard %s file is closed", findex + strlen(IO_PREFIX));
  return p->f;
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


static void aux_lines (lua_State *L, int toclose) {
  int i;
  int n = lua_gettop(L) - 1;  /* number of arguments to read */
  /* ensure that arguments will fit here and into 'io_readline' stack */
  luaL_argcheck(L, n <= LUA_MINSTACK - 3, LUA_MINSTACK - 3, "too many options");
  lua_pushvalue(L, 1);  /* file handle */
  lua_pushinteger(L, n);  /* number of arguments to read */
  lua_pushboolean(L, toclose);  /* close/not close file when finished */
  for (i = 1; i <= n; i++) lua_pushvalue(L, i + 1);  /* copy arguments */
  lua_pushcclosure(L, io_readline, 3 + n);
}


static int f_lines (lua_State *L) {
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


static int read_number (lua_State *L, FILE *f) {
  lua_Number d;
  if (fscanf(f, LUA_NUMBER_SCAN, &d) == 1) {
    lua_pushnumber(L, d);
    return 1;
  }
  else {
   lua_pushnil(L);  /* "result" to be removed */
   return 0;  /* read fails */
  }
}

static int test_eof (lua_State *L, FILE *f) {
  int c = lua_getc(f);
  ungetc(c, f);
  lua_pushlstring(L, NULL, 0);
  return (c != EOF);
}


static int read_line (lua_State *L, FILE *f, int chop) {
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  for (;;) {
    size_t l;
    char *p = luaL_prepbuffer(&b);
    if (fgets(p, LUAL_BUFFERSIZE, f) == NULL) {  /* eof? */
      luaL_pushresult(&b);  /* close buffer */
      return (lua_rawlen(L, -1) > 0);  /* check whether read something */
    }
    l = strlen(p);
    if (l == 0 || p[l-1] != '\n')
      luaL_addsize(&b, l);
    else {
      luaL_addsize(&b, l - chop);  /* chop 'eol' if needed */
      luaL_pushresult(&b);  /* close buffer */
      return 1;  /* read at least an `eol' */
    }
  }
}

#define MAX_SIZE_T	(~(size_t)0)

static void read_all (lua_State *L, FILE *f) {
  size_t rlen = LUAL_BUFFERSIZE;  /* how much to read in each cycle */
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  for (;;) {
    char *p = luaL_prepbuffsize(&b, rlen);
    size_t nr = fread(p, sizeof(char), rlen, f);
    luaL_addsize(&b, nr);
    if (nr < rlen) break;  /* eof? */
    else if (rlen <= (MAX_SIZE_T / 4))  /* avoid buffers too large */
      rlen *= 2;  /* double buffer size at each iteration */
  }
  luaL_pushresult(&b);  /* close buffer */
}
#endif

static int read_chars (lua_State *L, FILE *f, size_t n) {
  unsigned int nr;  /* number of chars actually read */
  char *p;
  luaL_Buffer b;
  luaL_buffinit(L, &b);
  p = luaL_prepbuffsize(&b, n);  /* prepare buffer to read whole block */
  FRESULT result = f_read(f, p, n, &nr);  /* try to read 'n' chars */
  luaL_addsize(&b, nr);
  luaL_pushresult(&b);  /* close buffer */
  return (result == FR_OK && nr > 0);  /* true iff read something */
}

#if !defined(USE_FATFS)
static int g_read (lua_State *L, FILE *f, int first) {
  int nargs = lua_gettop(L) - 1;
  int success;
  int n;
  clearerr(f);
  if (nargs == 0) {  /* no arguments? */
    success = read_line(L, f, 1);
    n = first+1;  /* to return 1 result */
  }
  else {  /* ensure stack space for all results and for auxlib's buffer */
    luaL_checkstack(L, nargs+LUA_MINSTACK, "too many arguments");
    success = 1;
    for (n = first; nargs-- && success; n++) {
      if (lua_type(L, n) == LUA_TNUMBER) {
        size_t l = (size_t)lua_tointeger(L, n);
        success = (l == 0) ? test_eof(L, f) : read_chars(L, f, l);
      }
      else {
        const char *p = lua_tostring(L, n);
        luaL_argcheck(L, p && p[0] == '*', n, "invalid option");
        switch (p[1]) {
          case 'n':  /* number */
            success = read_number(L, f);
            break;
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
  if (ferror(f))
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


static int f_read (lua_State *L) {
  return g_read(L, tofile(L), 2);
}

#else

static int io_read (lua_State *L) {
  LStream *p = tolstream(L);
  size_t l = (size_t)lua_tointeger(L, 2);
  read_chars(L, &p->f, l);
  return 1;
}

#endif

#if !defined(USE_FATFS)
static int io_readline (lua_State *L) {
  LStream *p = (LStream *)lua_touserdata(L, lua_upvalueindex(1));
  int i;
  int n = (int)lua_tointeger(L, lua_upvalueindex(2));
  if (isclosed(p))  /* file is already closed? */
    return luaL_error(L, "file is already closed");
  lua_settop(L , 1);
  for (i = 1; i <= n; i++)  /* push arguments to 'g_read' */
    lua_pushvalue(L, lua_upvalueindex(3 + i));
  n = g_read(L, p->f, 2);  /* 'n' is number of results */
  lua_assert(n > 0);  /* should return at least a nil */
  if (!lua_isnil(L, -n))  /* read at least one value? */
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

#endif

static int g_write (lua_State *L, FILE *f, int arg) {
  int nargs = lua_gettop(L) - arg;
  int status = 1;
  for (; nargs--; arg++) {
    if (lua_type(L, arg) == LUA_TNUMBER) {
      /* optimization: could be done exactly as for strings */
      char s[LUAI_MAXNUMBER2STR];
      sprintf(s, LUA_NUMBER_FMT, lua_tonumber(L, arg));
      status = status && f_puts(s, f) > 0;
    }
    else {
      size_t l;
      UINT count;
      const char *s = luaL_checklstring(L, arg, &l);
      status = status && (f_write(f, s, l, &count) == FR_OK && count == l);
    }
  }
  if (status) return 1;  /* file handle already on stack top */
  else return luaL_fileresult(L, status, NULL);
}

#if !defined(USE_FATFS)
static int io_write (lua_State *L) {
  return g_write(L, getiofile(L, IO_OUTPUT), 1);
}

static int f_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  return g_write(L, f, 2);
}
#else
static int io_write (lua_State *L) {
  FILE *f = tofile(L);
  lua_pushvalue(L, 1);  /* push file at the stack top (to be returned) */
  return g_write(L, f, 2);
}
#endif

#if !defined(USE_FATFS)
static int f_seek (lua_State *L) {
  static const int mode[] = {SEEK_SET, SEEK_CUR, SEEK_END};
  static const char *const modenames[] = {"set", "cur", "end", NULL};
  FILE *f = tofile(L);
  int op = luaL_checkoption(L, 2, "cur", modenames);
  lua_Number p3 = luaL_optnumber(L, 3, 0);
  l_seeknum offset = (l_seeknum)p3;
  luaL_argcheck(L, (lua_Number)offset == p3, 3,
                  "not an integer in proper range");
  op = l_fseek(f, offset, mode[op]);
  if (op)
    return luaL_fileresult(L, 0, NULL);  /* error */
  else {
    lua_pushnumber(L, (lua_Number)l_ftell(f));
    return 1;
  }
}


static int f_setvbuf (lua_State *L) {
  static const int mode[] = {_IONBF, _IOFBF, _IOLBF};
  static const char *const modenames[] = {"no", "full", "line", NULL};
  FILE *f = tofile(L);
  int op = luaL_checkoption(L, 2, NULL, modenames);
  lua_Integer sz = luaL_optinteger(L, 3, LUAL_BUFFERSIZE);
  int res = setvbuf(f, NULL, mode[op], sz);
  return luaL_fileresult(L, res == 0, NULL);
}


static int io_flush (lua_State *L) {
  return luaL_fileresult(L, fflush(getiofile(L, IO_OUTPUT)) == 0, NULL);
}


static int f_flush (lua_State *L) {
  return luaL_fileresult(L, fflush(tofile(L)) == 0, NULL);
}

#else

static int io_seek (lua_State *L) {
  FILE *f = tofile(L);
  lua_Unsigned offset = luaL_checkunsigned(L, 2);
  lua_pushinteger(L, f_lseek(f, offset));
  return 1;
}

#endif

/*
** functions for 'io' library
*/
LROT_BEGIN(iolib, NULL, 0)
  LROT_FUNCENTRY( close, io_close )
  LROT_FUNCENTRY( seek, io_seek )
  LROT_FUNCENTRY( open, io_open )
  LROT_FUNCENTRY( read, io_read )
  LROT_FUNCENTRY( write, io_write )
LROT_END(iolib, NULL, 0)

/*
** methods for file handles
**
** we define only garbage collector to close any leftover open files
*/
LROT_BEGIN(file_handle, NULL, LROT_MASK_GC)
  LROT_FUNCENTRY( __gc, f_gc )
LROT_END(file_handle, NULL, LROT_MASK_GC)


LUAMOD_API int luaopen_io (lua_State *L) {
  luaL_rometatable( L, LUA_FILEHANDLE,  LROT_TABLEREF(file_handle));
  return 0;
}
