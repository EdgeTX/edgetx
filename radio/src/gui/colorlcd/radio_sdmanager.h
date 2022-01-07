/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
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

#include "opentx.h"
#include "dataconstants.h"
#include "tabsgroup.h"
#include "sdcard.h"

#include "strhelpers.h"

// todo: later move to strhelpers.h (#1317 may use that also)
template<size_t L>
struct Stringbuffer {
    explicit Stringbuffer(char (&b)[L]) : buffer{b}, end{buffer} {
        clear();
    }
    constexpr operator const char*() const {
        return &buffer[0];
    }
    char* raw() {
        end = &buffer[L - 1];
        return &buffer[0];
    }
    template<typename F>
    char* fillRaw(F f) {
        f(&buffer[0]);
        revalidate();
        return &buffer[0];
    }
    void revalidate() {
        for(end = &buffer[0]; end < &buffer[L]; ++end) {
            if (*end == '\0') {
                return;
            }
        }
        *--end = '\0';
    }
    Stringbuffer& clear() {
        buffer[0] = '\0';
        end = &buffer[0];
        return *this;
    }
    Stringbuffer& add(const char* const s, size_t maxChars = L - 1) {
        if (s) {
            for(size_t i = 0; (i < maxChars) && (end < &buffer[L - 1]); ++i, ++end) {
                if ((*end = s[i]) == '\0') {
                    return *this;
                }
            }
            *end = '\0';
        }
        return *this;
    }
    Stringbuffer& add(const std::string& s) {
        return add(s.c_str());
    }
    Stringbuffer& add(const char* const a, const char* const e) {
        if ((a) && (e) && (e > a)) {
            const size_t length = (e - a) - 1;
            add(a, length);            
        }
        return *this;
    }
    template<size_t Size>
    Stringbuffer& add(const char (&s)[Size]) {
        return add(s, Size);
    }
    constexpr size_t size() const {
        return end - &buffer[0];
    }
    constexpr size_t capacity() const {
        return L - 1;
    }
    constexpr size_t free() const {
        return capacity() - size();
    }
private:
    char (&buffer)[L];
    char* end{}; // assert (end == '\0') && (end < &buffer[L])
};

// in C++17 and onwards use CTAD
using path_t = Stringbuffer<sizeof(reusableBuffer.sdManager.pathConstructBuffer)>;
using name_t = Stringbuffer<sizeof(reusableBuffer.sdManager.nameBuffer)>;

enum MultiModuleType : short;

class RadioSdManagerPage : public PageTab
{
public:
    RadioSdManagerPage();
    
    void build(FormWindow* window) override;
    
protected:
    void bootloaderUpdate(const char* name);
    void frSkyFirmwareUpdate(const char* name, ModuleIndex module);
    void multiFirmwareUpdate(const char* name, ModuleIndex module,
                             MultiModuleType type);
    void rebuild(FormWindow* window);
    
private:
    void updateCurrentDir();
    bool fileExists(const char* filename);
     
    inline const char* setPathBufferToFullPath(const char* filename) {
        return pathBuffer.clear().add(currentDir).add(PATH_SEPARATOR).add(filename);        
    }
    path_t pathBuffer{reusableBuffer.sdManager.pathConstructBuffer};
    path_t currentDir{reusableBuffer.sdManager.currentDirBuffer};
};
