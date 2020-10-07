/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef _LIBOPEUI_FILE_H_
#define _LIBOPEUI_FILE_H_

#include <string>
#include <string.h>
#include <inttypes.h>
#include "ff.h"
// #include "libopenui_defines.h"

constexpr uint8_t LEN_FILE_EXTENSION_MAX = 5;  // longest used, including the dot, excluding null term.

const char * getFileExtension(const char * filename, uint8_t size = 0, uint8_t extMaxLen = 0, uint8_t * fnlen = nullptr, uint8_t * extlen = nullptr);
bool isExtensionMatching(const char * extension, const char * pattern, char * match = nullptr);
FRESULT sdReadDir(DIR * dir, FILINFO * fno, bool & firstTime);

// comparison, not case sensitive.
inline bool compare_nocase(const std::string &first, const std::string &second)
{
  return strcasecmp(first.c_str(), second.c_str()) < 0;
}

#endif
