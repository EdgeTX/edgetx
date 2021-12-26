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

#include <algorithm>

#include "colors.h"
#include "debug.h"
#include "opentx.h"
#include "str_functions.h"

#define COLOR_COUNT 13

#define AUTHOR_LENGTH 50
#define INFO_LENGTH 255

class ThemePersistance;
extern ThemePersistance themePersistance;

struct ColorEntry
{
    LcdColorIndex colorNumber;
    uint32_t colorValue;

    bool operator== (const ColorEntry &a) { return this->colorNumber == a.colorNumber; }
};

class ThemeFile
{
 public:
    ThemeFile() { 
      name[0] = '\0';
      author[0] = '\0';
      info[0] = '\0';
    }
    
    ThemeFile(std::string themePath, bool loadYAML = true);

    virtual ~ThemeFile() {}

    ThemeFile& operator= (const ThemeFile& theme);

    void serialize();

    std::string getPath() { return path; }
    char *getName() { return name; }
    char *getAuthor() { return author; }
    char *getInfo() { return info; }

    ColorEntry *getColorEntryByIndex(LcdColorIndex colorNumber) {
        int n = 0;
        for (auto colorEntry : colorList) {
            if (colorEntry.colorNumber == colorNumber)
                return &colorList[n];
            n++;
        }

        return nullptr;
    }
    
    void setName(std::string name) { strAppend(this->name, name.c_str(), SELECTED_THEME_NAME_LEN); }
    void setAuthor(std::string author) { strAppend(this->author, author.c_str(), AUTHOR_LENGTH); }
    void setInfo(std::string info) { strAppend(this->info, info.c_str(), INFO_LENGTH); }
    void setPath(std::string path) { this->path = path; }

    std::vector<ColorEntry>& getColorList() { return colorList; }
    void setColor(LcdColorIndex colorIndex, uint32_t color);

    virtual std::vector<std::string> getThemeImageFileNames();
    void applyTheme();

  protected:
    std::string path;
    char name[SELECTED_THEME_NAME_LEN + 1];
    char author[AUTHOR_LENGTH + 1];
    char info[INFO_LENGTH + 1];
    std::vector<ColorEntry> colorList;
    std::vector<std::string> _imageFileNames;
    void applyColors();
    virtual void applyBackground();

    void deSerialize();
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
    static char **getColorNames();
    bool deleteThemeByIndex(int index);
    bool createNewTheme(std::string name, ThemeFile &theme);

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
      if (index >= 0 && index < (int)themes.size()) {
        auto theme = themes[index];
        theme->applyTheme();
      }
    }

    inline int getThemeIndex() {return currentTheme;}
    inline void setThemeIndex(int index) { currentTheme = index;}
    inline bool isDefaultTheme() { if (currentTheme == 0) return true; else return false; }

    inline ThemeFile* getCurrentTheme() 
    { 
      if (currentTheme < (int)themes.size()) 
        return themes[currentTheme];
      return nullptr;
    }

    inline ThemeFile* getThemeByIndex(int index) 
    { 
      if (index < (int) themes.size())
        return themes[index]; 
      return nullptr;
    }

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
    void clearThemes();
    void scanThemeFolder(char *fullPath);
};
