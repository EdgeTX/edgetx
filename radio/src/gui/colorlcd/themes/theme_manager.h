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
#include "edgetx.h"
#include "sdcard.h"

struct ColorEntry
{
    LcdColorIndex colorNumber;
    uint32_t colorValue;

    bool operator== (const ColorEntry &a) { return this->colorNumber == a.colorNumber; }
};

class ThemeFile
{
 public:
    ThemeFile() {}
    ThemeFile(std::string themePath, bool loadYAML = true);

    virtual ~ThemeFile() {}

    ThemeFile& operator= (const ThemeFile& theme);

    void serialize();

    std::string getPath() { return path; }
    std::string getName() { return name; }
    std::string getAuthor() { return author; }
    std::string getInfo() { return info; }

    ColorEntry *getColorEntryByIndex(LcdColorIndex colorNumber) {
        int n = 0;
        for (auto colorEntry : colorList) {
            if (colorEntry.colorNumber == colorNumber)
                return &colorList[n];
            n++;
        }

        return nullptr;
    }
    
    void setName(std::string name) { this->name = name; }
    void setAuthor(std::string author) { this->author = author; }
    void setInfo(std::string info) { this->info = info; }
    void setPath(std::string path) { this->path = path; }

    std::vector<ColorEntry>& getColorList() { return colorList; }
    void setColor(LcdColorIndex colorIndex, uint32_t color);

    virtual std::vector<std::string> getThemeImageFileNames();
    void applyTheme();

    static constexpr int AUTHOR_LENGTH = 50;
    static constexpr int INFO_LENGTH = 255;

  protected:
    std::string path;
    std::string name;
    std::string author;
    std::string info;
    std::vector<ColorEntry> colorList;
    std::vector<std::string> _imageFileNames;

    void applyColors();
    void applyBackground();

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

    void refresh();

  protected:
    std::vector<ThemeFile *> themes;
    int currentTheme = 0;
    void scanForThemes();
    void insertDefaultTheme();
    void clearThemes();
    void scanThemeFolder(char *fullPath);

    static ThemePersistance themePersistance;
};

class HeaderDateTime : public Window
{
 public:
  HeaderDateTime(Window* parent, int x, int y);

  void setColor(LcdFlags color);

  static LAYOUT_VAL(HDR_DATE_WIDTH, 45, 45)
  static LAYOUT_VAL(HDR_DATE_HEIGHT, 12, 12)
  static LAYOUT_VAL(HDR_DATE_LINE2, 15, 15)

 protected:
  lv_obj_t *date = nullptr;
  lv_obj_t *time = nullptr;
  struct gtm lastTime = { 0 };

  void checkEvents() override;
};

class HeaderIcon : public StaticIcon
{
 public:
  HeaderIcon(Window *parent, EdgeTxIcon icon);
  HeaderIcon(Window *parent, const char* iconFile);
};

class UsbSDConnected : public Window
{
 public:
  UsbSDConnected();
};
