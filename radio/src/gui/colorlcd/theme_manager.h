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
#pragma once
#include <stdlib.h>
#include <list>
#include <string>
#include <locale>
#include "sdcard.h"
#include "colors.h"
#include "str_functions.h"

class ThemePersistance;
extern ThemePersistance themePersistance;

struct ColorEntry
{
    LcdColorIndex colorNumber;
    uint32_t colorValue;
};

class ThemeFile
{
 public:
    ThemeFile(std::string themePath) :
      path(themePath)
    {
        if (themePath.size()) {
            scanFile();
        }
    }

    std::string getPath() { return path; }
    std::string getName() { return name; }
    std::string getAuthor() { return author; }
    std::string getInfo() { return info; }
    
    void setName(std::string name) { this->name = name; }
    void setAuthor(std::string author) { this->author = author; }
    void setInfo(std::string info) { this->info = info; }

    std::vector<ColorEntry> getColorList() { return colorList; }
    void setColor(LcdColorIndex colorIndex, uint32_t color);

    virtual std::string getThemeImageFileName();
    virtual void applyTheme();

  protected:
    FIL file;
    std::string path;
    std::string name;
    std::string author;
    std::string info;
    std::vector<ColorEntry> colorList;


    enum ScanState
    {
        none,
        summary,
        colors
    };

    void scanFile();
    bool convertRGB(char *pColorRGB, uint32_t &color);
    LcdColorIndex findColorIndex(const char *name);
    bool readNextLine(char * line, int maxlen);
};


class ThemePersistance
{
  public:
    ThemePersistance()
    {
    }

    static ThemePersistance *instance() {
        return &themePersistance;
    }

    void loadDefaultTheme();
    void setDefaultTheme(int index);
    void deleteDefaultTheme();

    std::vector<std::string> getNames()
    {
        std::vector<std::string> names;
        for (auto theme:themes) {
            names.emplace_back(theme->getName());
        }

        return names;
    }

    void applyTheme(int index)
    {
        auto theme = themes[index];
        theme->applyTheme();
    }

    inline int getThemeIndex() {return currentTheme;}
    inline void setThemeIndex(int index) { currentTheme = index;}

    inline ThemeFile* getCurrentTheme() { return themes[currentTheme]; }

    void refresh()
    {
        scanForThemes();
        insertDefaultTheme();
    }

  protected:
    std::vector<ThemeFile *> themes;
    int currentTheme = 0;
    void scanForThemes();
    void insertDefaultTheme();
};
