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

#pragma once

#include <string>
#include <cstring>
#include <cinttypes>
#include <memory>

constexpr uint8_t LEN_FILE_EXTENSION_MAX = 5;  // longest used, including the dot, excluding null term.

const char * getFileExtension(const char * filename, uint8_t size = 0, uint8_t extMaxLen = 0, uint8_t * fnlen = nullptr, uint8_t * extlen = nullptr);
bool isFileExtensionMatching(const char * extension, const char * pattern, char * match = nullptr);
//
//const char * OUiGetFileExtension(const char * filename, uint8_t size = 0,
//                                        uint8_t extMaxLen = 0, uint8_t * fnlen = nullptr, uint8_t * extlen = nullptr);
//bool OUiGisFileExtensionMatching(const char * extension, const char * pattern, char * match = nullptr);
