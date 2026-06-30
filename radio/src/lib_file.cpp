/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#include "lib_file.h"

#include <inttypes.h>
#include <cinttypes>
#include <stdarg.h>

#if defined(_WIN32) || defined(_WIN64)
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif

const char * getFileExtension(const char * filename, uint8_t size, uint8_t extMaxLen, uint8_t * fnlen, uint8_t * extlen)
{
  int len = size;
  if (!size) {
    len = strlen(filename);
  }
  if (!extMaxLen) {
    extMaxLen = LEN_FILE_EXTENSION_MAX;
  }
  if (fnlen != nullptr) {
    *fnlen = (uint8_t)len;
  }
  for (int i=len-1; i >= 0 && len-i <= extMaxLen; --i) {
    if (filename[i] == '.') {
      if (extlen) {
        *extlen = len-i;
      }
      return &filename[i];
    }
  }
  if (extlen != nullptr) {
    *extlen = 0;
  }
  return nullptr;
}

/**
  Check if given extension exists in a list of extensions.
  @param extension The extension to search for, including leading period.
  @param pattern One or more file extensions concatenated together, including the periods.
    The list is searched backwards and the first match, if any, is returned.
    eg: ".gif.jpg.jpeg.png"
  @param match Optional container to hold the matched file extension (wide enough to hold LEN_FILE_EXTENSION_MAX + 1).
  @retval true if a extension was found in the lost, false otherwise.
*/
bool isExtensionMatching(const char * extension, const char * pattern, char * match)
{
  const char *ext;
  uint8_t extlen, fnlen;
  int plen;

  ext = getFileExtension(pattern, 0, 0, &fnlen, &extlen);
  plen = (int)fnlen;
  while (plen > 0 && ext) {
    if (!strncasecmp(extension, ext, extlen)) {
      if (match != nullptr) strncat(&(match[0]='\0'), ext, extlen);
      return true;
    }
    plen -= extlen;
    if (plen > 0) {
      ext = getFileExtension(pattern, plen, 0, nullptr, &extlen);
    }
  }
  return false;
}

#if !defined(BOOT)

// CWD tracker + path normalizer (see lib_file.h).

static char s_currentDir[FF_MAX_LFN + 1] = "/";

void etxNormalizePath(const char* in, char* out, size_t outLen)
{
  if (outLen == 0) return;
  out[0] = '\0';
  if (outLen < 2) return;
  if (!in) in = "";

  // absolute input replaces the CWD; relative input is appended
  char work[FF_MAX_LFN + 1];
  if (in[0] == '/') {
    strncpy(work, in, sizeof(work) - 1);
    work[sizeof(work) - 1] = '\0';
  } else {
    snprintf(work, sizeof(work), "%s/%s", s_currentDir, in);
  }

  size_t len = 0;
  bool truncated = false;

  char* p = work;
  while (*p) {
    while (*p == '/') p++;
    if (!*p) break;
    char* seg = p;
    while (*p && *p != '/') p++;
    size_t seglen = (size_t)(p - seg);

    if (seglen == 1 && seg[0] == '.') continue;
    if (seglen == 2 && seg[0] == '.' && seg[1] == '.') {  // pop, clamp at root
      while (len > 0 && out[len - 1] != '/') len--;
      if (len > 0) len--;
      out[len] = '\0';
      continue;
    }
    if (len + 1 + seglen >= outLen) { truncated = true; break; }
    out[len++] = '/';
    memcpy(out + len, seg, seglen);
    len += seglen;
    out[len] = '\0';
  }

  // genuine collapse to root -> "/"; don't fabricate it on truncation
  if (len == 0 && !truncated) {
    out[0] = '/';
    out[1] = '\0';
  }
}

FRESULT etxChdir(const char* path)
{
  char abs[FF_MAX_LFN + 1];
  etxNormalizePath(path, abs, sizeof(abs));
  FRESULT res = f_chdir(abs);
  if (res == FR_OK) {
    strncpy(s_currentDir, abs, sizeof(s_currentDir) - 1);
    s_currentDir[sizeof(s_currentDir) - 1] = '\0';
  }
  return res;
}

const char* etxGetcwd() { return s_currentDir; }

// Replace FatFS implementation of f_puts and f_printf

int f_puts(const char* s, FIL* fp)
{
  unsigned int written;
  unsigned int len = strlen(s);
  FRESULT result = f_write(fp, s, len, &written);
  return (len == written && result == FR_OK) ? 0 : EOF;
}

#define MAX_FPRINTF_LEN 100

int f_printf (FIL* fp, const TCHAR* str, ...)
{
  char s[MAX_FPRINTF_LEN];
  va_list argptr;
  va_start(argptr, str);
  vsnprintf(s, MAX_FPRINTF_LEN, str, argptr);
  va_end(argptr);
  return f_puts(s, fp);
}

#endif
