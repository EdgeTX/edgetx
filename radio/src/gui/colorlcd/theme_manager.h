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
#include "sdcard.h"
#include "str_functions.h"

class ThemePersistance;
extern ThemePersistance themePersistance;

#define COLOR_COUNT 13

struct ColorEntry
{
    LcdColorIndex colorNumber;
    uint32_t colorValue;

    bool operator== (const ColorEntry &a) { return this->colorNumber == a.colorNumber; }
};


#define AUTHOR_LENGTH 50
#define INFO_LENGTH 255
#define NAME_LENGTH 26
class ThemeFile
{
 public:
    ThemeFile() { 
      name[0] = '\0';
      author[0] = '\0';
      info[0] = '\0';
    }
    
    ThemeFile(std::string themePath);
    ThemeFile(const ThemeFile &theme)
    {
        path = theme.path;
        strncpy(name, theme.name, NAME_LENGTH);
        name[NAME_LENGTH] = '\0';
        strncpy(author, theme.author, AUTHOR_LENGTH);
        author[AUTHOR_LENGTH] = '\0';
        strncpy(info, theme.info, INFO_LENGTH);
        info[INFO_LENGTH] = '\0';
        colorList.assign(theme.colorList.begin(), theme.colorList.end());
    }
    virtual ~ThemeFile()
    {
    }

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

    uint32_t getColorByName(std::string colorName) {

        auto colorIndex = findColorIndex(colorName.c_str());
        ColorEntry a = {colorIndex, 0};
        auto colorEntry = std::find(colorList.begin(), colorList.end(), a);
        if (colorEntry != colorList.end())
            return colorEntry->colorValue;
        
        return 0;
    }
    
    void setName(std::string name) { strncpy(this->name, name.c_str(), NAME_LENGTH); }
    void setAuthor(std::string author) { strncpy(this->author, author.c_str(), AUTHOR_LENGTH); }
    void setInfo(std::string info) { strncpy(this->info, info.c_str(), INFO_LENGTH); }
    void setPath(std::string path) { this->path = path; }

    std::vector<ColorEntry>& getColorList() { return colorList; }
    void setColor(LcdColorIndex colorIndex, uint32_t color);
    void setColorByIndex(int index, uint32_t color);

    virtual std::vector<std::string> getThemeImageFileNames();
    void applyTheme();

  protected:
    std::string path;
    char name[NAME_LENGTH + 1];
    char author[AUTHOR_LENGTH + 1];
    char info[INFO_LENGTH + 1];
    std::vector<ColorEntry> colorList;
    std::vector<std::string> _imageFileNames;
    void applyColors();
    virtual void applyBackground();

    enum ScanState
    {
        none,
        summary,
        colors
    };

    void deSerialize();
    bool convertRGB(char *pColorRGB, uint32_t &color);
    LcdColorIndex findColorIndex(const char *name);
    bool readNextLine(VfsFile &file, char *line, int maxlen);
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
      if (index >= 0 && index < (int) themes.size()) {
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
