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
#include "theme_manager.h"

ThemePersistance themePersistance;

#define COLOR_COUNT 12
static const char *conversionArray[COLOR_COUNT] = {
    "DEFAULT",    "PRIMARY1",   "PRIMARY2",   "PRIMARY3",
    "SECONDARY1", "SECONDARY2", "SECONDARY3", "FOCUS",
    "EDIT",       "ACTIVE",     "WARNING",    "DISABLED",
};

constexpr const char *RGBSTRING = "RGB(";
constexpr const char *THEMES = "THEMES";

char *getWorkingDirectory()
{
  static char path[FF_MAX_LFN + 1];  // TODO optimize that!
  f_getcwd((TCHAR *)path, FF_MAX_LFN);
  if (path[strlen(path) - 1] != '/') strncat(path, "/", FF_MAX_LFN);

  strncat(path, THEMES, FF_MAX_LFN);
  return path;
}

std::string ThemeFile::getThemeImageFileName()
{
  char fullPath[FF_MAX_LFN + 1];
  strncpy(fullPath, getWorkingDirectory(), FF_MAX_LFN);
  if (fullPath[strlen(fullPath) - 1] != '/') 
    strncat(fullPath, "/", FF_MAX_LFN);
  
  auto found = path.find('.');
  if (found != std::string::npos) {
    auto baseFileName(fullPath + path.substr(0, found) + ".png");
    return baseFileName;
  }

  return "";
}

void ThemeFile::scanFile()
{
  char line[256 + 1];
  char fullPath[FF_MAX_LFN + 1];
  ScanState scanState = none;

  strncpy(fullPath, getWorkingDirectory(), FF_MAX_LFN);
  if (fullPath[strlen(fullPath) - 1] != '/') strncat(fullPath, "/", FF_MAX_LFN);
  strncat(fullPath, path.c_str(), FF_MAX_LFN);
  FRESULT result = f_open(&file, fullPath, FA_OPEN_EXISTING | FA_READ);
  if (result != FR_OK) return;

  int lineNo = 1;
  while (readNextLine(line, 256)) {
    int len = strlen(line);
    if (len == 0) continue;

    if (lineNo == 1 && len != 3 && strcmp(line, "---") != 0) {
      TRACE("invalid yml file at line %d", lineNo);
      return;
    } else if (lineNo != 1) {
      if (line[0] != ' ' && line[0] != '\t') {
        char *pline = trim(line);
        if (line[strlen(line) - 1] != ':') {
          TRACE("invalid yml file at line %d", lineNo);
          return;
        }

        if (strcmp(pline, "colors:") == 0) {
          scanState = colors;
          continue;
        } else if (strcmp(pline, "summary:") == 0) {
          scanState = summary;
          continue;
        }
      }

      const char *ptr = strchr(line, ':');
      if (!ptr) continue;

      char lvalue[64];
      char rvalue[64];
      char *plvalue;
      char *prvalue;

      strncpy(lvalue, line, min((int)(ptr - line), 63));
      lvalue[ptr - line] = '\0';
      strncpy(rvalue, ptr + 1, 63);
      plvalue = trim(lvalue);
      prvalue = trim(rvalue);

      switch (scanState) {
        case colors: {
          int colorIndex = findColorIndex(plvalue);
          if (colorIndex >= 0) {
            uint32_t color;
            if (convertRGB(prvalue, color))
              colorList.emplace_back(ColorEntry{(LcdColorIndex)colorIndex, color});
            else
              TRACE("Theme: Could not convert color value");
          }
        } break;

        case summary: {
          if (strcmp(plvalue, "name") == 0)
            name = prvalue;
          else if (strcmp(plvalue, "author") == 0)
            author = prvalue;
          else if (strcmp(plvalue, "info") == 0)
            info = prvalue;
        } break;
      }
    }

    lineNo++;
  }

  f_close(&file);
}

#define HEX_COLOR_VALUE_LEN 8

bool ThemeFile::convertRGB(char *pColorRGB, uint32_t &color)
{
  if (strlen(pColorRGB) < strlen(RGBSTRING)) return false;

  if (strncmp(pColorRGB, RGBSTRING, strlen(RGBSTRING)) == 0) {
    if (pColorRGB[strlen(pColorRGB) - 1] != ')') return false;

    pColorRGB[strlen(pColorRGB) - 1] = '\0';
    pColorRGB += strlen(RGBSTRING);
    if (strlen(pColorRGB)) {
      char *token = nullptr;
      int numTokens = 0;
      uint32_t tokens[3];

      token = strtok(pColorRGB, ",");
      while (token != nullptr) {
        if (numTokens < 3) {
          tokens[numTokens] = strtol(token, nullptr, 0);
          numTokens++;
        } else
          break;

        token = strtok(nullptr, ",");
      }

      if (numTokens != 3) return false;

      color = RGB(tokens[0], tokens[1], tokens[2]);
      return true;
    }
  } else if (pColorRGB[0] == '0' && (pColorRGB[1] == 'x' || pColorRGB[1] == 'X')) {
    if (strlen(pColorRGB) != HEX_COLOR_VALUE_LEN) return false;
    pColorRGB += 2;
    uint32_t tokens[3];
    char hexVal[3];
    for (int i = 0; i < 3; i++) {
      strncpy(hexVal, pColorRGB, 2);
      hexVal[2] = '\0';
      tokens[i] = strtol(hexVal, nullptr, 16);
      pColorRGB += 2;
    }

    color = RGB(tokens[0], tokens[1], tokens[2]);
    return true;
  }

  TRACE("Theme: Invalid color value");
  return false;
}

LcdColorIndex ThemeFile::findColorIndex(const char *name)
{
  int i;
  for (i = 0; i < COLOR_COUNT; i++) {
    if (strcmp(name, conversionArray[i]) == 0) break;
  }

  if (i >= COLOR_COUNT) return DEFAULT_COLOR_INDEX;

  return (LcdColorIndex)i;
}

bool ThemeFile::readNextLine(char *line, int maxlen)
{
  if (f_gets(line, maxlen, &file) != NULL) {
    int curlen = strlen(line) - 1;
    if (line[curlen] ==
        '\n') {  // remove unwanted chars if file was edited using windows
      if (line[curlen - 1] == '\r') {
        line[curlen - 1] = 0;
      } else {
        line[curlen] = 0;
      }
    }

    return true;
  }

  return false;
}

void ThemeFile::setColor(LcdColorIndex colorIndex, uint32_t color)
{
  if (colorIndex >= DEFAULT_COLOR_INDEX && colorIndex < LCD_COLOR_COUNT) {
    colorList.emplace_back(ColorEntry {colorIndex, color});
  }
}

void ThemeFile::applyTheme()
{
  for (auto color: colorList) {
      lcdColorTable[color.colorNumber] = color.colorValue;
  }
  OpenTxTheme::instance()->update(false);
}

void ThemePersistance::scanForThemes()
{
  TRACE("in scanForThemes");
  themes.clear();

  DIR dir;
  FILINFO fno;

  char fullPath[FF_MAX_LFN + 1];
  strcpy(fullPath, "./");
  strcat(fullPath, THEMES);

  TRACE("opening directory: %s", fullPath);
  FRESULT res = f_opendir(&dir, fullPath);  // Open the directory
  if (res == FR_OK) {
    TRACE("scanForThemes: open successful");
    // read all entries
    bool firstTime = true;
    for (;;) {
      res = sdReadDir(&dir, &fno, firstTime);

      if (res != FR_OK || fno.fname[0] == 0)
        break;  // Break on error or end of dir

      if (strlen((const char *)fno.fname) > SD_SCREEN_FILE_LENGTH) continue;
      if (fno.fattrib & AM_DIR) continue;

      TRACE("scanForThemes: found file %s", fno.fname);
      std::string fname(fno.fname);
      auto found = fname.find('.');
      if (found != std::string::npos) {
        if (fname.substr(found) != ".yml") continue;
      }

      themes.emplace_back(new ThemeFile(fno.fname));
    }

    f_closedir(&dir);
  }
}

void ThemePersistance::loadDefaultTheme()
{
  FIL file;
  char fullPath[128];
  strcpy(fullPath, getWorkingDirectory());
  if (fullPath[strlen(fullPath) - 1] != '/')
    strcat(fullPath, "/");
  strcat(fullPath, "selectedtheme.txt");

  FRESULT status = f_open(&file, fullPath, FA_READ);
  if (status != FR_OK) return;

  char line[256];
  uint len;
  status = f_read(&file, line, 256, &len);
  if (status == FR_OK) {
    refresh();

    line[len] = '\0';

    int index = 0;
    for (auto theme : themes) {
      if (theme->getPath() == std::string(line)) {
        applyTheme(index);
        setThemeIndex(index);
      }

      index++;
    }
  }
  f_close(&file);
}

void ThemePersistance::deleteDefaultTheme()
{
  char fullPath[128];
  strcpy(fullPath, getWorkingDirectory());
  if (fullPath[strlen(fullPath) - 1] != '/')
    strcat(fullPath, "/");
  strcat(fullPath, "selectedtheme.txt");
  FIL file;

  FRESULT status = f_open(&file, fullPath, FA_CREATE_ALWAYS | FA_WRITE);
  if (status == FR_OK) f_close(&file);
}

void ThemePersistance::setDefaultTheme(int index)
{
  char fullPath[128];
  strcpy(fullPath, getWorkingDirectory());
  if (fullPath[strlen(fullPath) - 1] != '/')
    strcat(fullPath, "/");
  strcat(fullPath, "selectedtheme.txt");
  FIL file;

  auto theme = themes[index];
  FRESULT status = f_open(&file, fullPath, FA_CREATE_ALWAYS | FA_WRITE);
  if (status != FR_OK) return;

  f_printf(&file, theme->getPath().c_str());
  f_close(&file);
}

class DefaultEdgeTxTheme : public ThemeFile
{
  public:
    DefaultEdgeTxTheme() : ThemeFile("")
    {
      setName("EdgeTX Default");
      setAuthor("EdgeTX Team");
      setInfo("Default EdgeTX Color Scheme");
    }

    void applyTheme() override
    {
      lcdColorTable[DEFAULT_COLOR_INDEX] = RGB(18, 94, 153);

      lcdColorTable[COLOR_THEME_PRIMARY1_INDEX] = RGB(0, 0, 0);
      lcdColorTable[COLOR_THEME_PRIMARY2_INDEX] = RGB(255, 255, 255);
      lcdColorTable[COLOR_THEME_PRIMARY3_INDEX] = RGB(12, 63, 102);
      lcdColorTable[COLOR_THEME_SECONDARY1_INDEX] = RGB(18, 94, 153);
      lcdColorTable[COLOR_THEME_SECONDARY2_INDEX] = RGB(182, 224, 242);
      lcdColorTable[COLOR_THEME_SECONDARY3_INDEX] = RGB(228, 238, 242);
      lcdColorTable[COLOR_THEME_FOCUS_INDEX] = RGB(20, 161, 229);
      lcdColorTable[COLOR_THEME_EDIT_INDEX] = RGB(0, 153, 9);
      lcdColorTable[COLOR_THEME_ACTIVE_INDEX] = RGB(255, 222, 0);
      lcdColorTable[COLOR_THEME_WARNING_INDEX] = RGB(224, 0, 0);
      lcdColorTable[COLOR_THEME_DISABLED_INDEX] = RGB(140, 140, 140);
      lcdColorTable[CUSTOM_COLOR_INDEX] = RGB(170, 85, 0);
      
      OpenTxTheme::instance()->update(false);
    }

    std::string getThemeImageFileName() override
    {
      return "/THEMES/EdgeTX.png";
    }
};

void ThemePersistance::insertDefaultTheme()
{
  auto themeFile = new DefaultEdgeTxTheme();
  themes.insert(themes.begin(), themeFile);
}
