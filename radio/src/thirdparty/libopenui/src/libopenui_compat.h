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

#pragma once

#if defined(__MINGW32__) || !defined(__GNUC__)
  #include <windows.h>
  #include <tchar.h>
  #define sleep(x) Sleep(x)
  #define strcasecmp  _stricmp
  #define strncasecmp _strnicmp
  #define chdir  _chdir
  #define getcwd _getcwd
  // remove windows default definitions
  #undef OPAQUE
  #undef RGB
  #undef EXTERN_C
#endif
