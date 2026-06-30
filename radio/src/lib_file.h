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
#include "ff.h"

constexpr uint8_t LEN_FILE_EXTENSION_MAX = 5;  // longest used, including the dot, excluding null term.

const char * getFileExtension(const char * filename, uint8_t size = 0, uint8_t extMaxLen = 0, uint8_t * fnlen = nullptr, uint8_t * extlen = nullptr);
bool isExtensionMatching(const char * extension, const char * pattern, char * match = nullptr);

// CWD tracker + path normalizer. FatFS can't resolve "."/".." or report the CWD
// on exFAT, so we track a normalized absolute CWD and resolve paths to absolute
// before they reach FatFS, matching FAT12/16/32 behavior.

// Resolve 'in' (absolute or relative to the tracked CWD) to a normalized
// absolute path in 'out' (bounds-checked against outLen).
void etxNormalizePath(const char * in, char * out, size_t outLen);
// f_chdir() to a normalized absolute path; updates the tracked CWD on success.
FRESULT etxChdir(const char * path);
// Tracked absolute CWD (replacement for the exFAT-broken f_getcwd()).
const char * etxGetcwd();

// comparison, not case sensitive.
static inline bool compare_nocase(const std::string& first, const std::string& second) {
    return std::lexicographical_compare(
        first.begin(), first.end(),
        second.begin(), second.end(),
        [](unsigned char a, unsigned char b) {
            return std::tolower(a) < std::tolower(b);
        }
    );
}
