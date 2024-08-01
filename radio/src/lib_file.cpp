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

#include "libopenui_file.h"

#include <inttypes.h>
#include <cinttypes>

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

// returns true if current working dir is at the root level
bool isCwdAtRoot()
{
  char path[10];
  if (f_getcwd(path, sizeof(path)-1) == FR_OK) {
    return (strcasecmp("/", path) == 0);
  }
  return false;
}

/*
  Wrapper around the f_readdir() function which
  also returns ".." entry for sub-dirs. (FatFS 0.12 does
  not return ".", ".." dirs anymore)
*/
FRESULT sdReadDir(DIR * dir, FILINFO * fno, bool & firstTime)
{
  FRESULT res;
  if (firstTime && !isCwdAtRoot()) {
    // fake parent directory entry
    strcpy(fno->fname, "..");
    fno->fattrib = AM_DIR;
    res = FR_OK;
  }
  else {
    res = f_readdir(dir, fno);                   /* Read a directory item */
  }
  firstTime = false;
  return res;
}
