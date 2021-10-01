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
  if (path[strlen(path) - 1] != '/')
    strncat(path, "/", FF_MAX_LFN);

  strncat(path, THEMES, FF_MAX_LFN);
  return path;
}

void ThemeFile::scanFile()
{
  char line[256 + 1];
  char fullPath[FF_MAX_LFN + 1];
  ScanState scanState = none;

  strncpy(fullPath, getWorkingDirectory(), FF_MAX_LFN);
  if (fullPath[strlen(fullPath) - 1] != '/')
    strncat(fullPath, "/", FF_MAX_LFN);
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
    } 
    else if (lineNo != 1) {
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

      strncpy(lvalue, line, ptr - line);
      lvalue[ptr - line] = '\0';
      strcpy(rvalue, ptr + 1);
      plvalue = trim(lvalue);
      prvalue = trim(rvalue);

      switch (scanState) {
        case colors: {
          int colorIndex = findColorIndex(plvalue);
          if (colorIndex >= 0) {
            uint32_t color;
            if (convertRGB(prvalue, color))
              colorList.emplace_back(
                  ColorEntry{(LcdColorIndex)colorIndex, color});
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

bool ThemeFile::convertRGB(char *pColorRGB, uint32_t &color)
{
  if (strlen(pColorRGB) < strlen(RGBSTRING)) return false;
  if (strncmp(pColorRGB, RGBSTRING, strlen(RGBSTRING)) != 0) return false;
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

  return false;
}

LcdColorIndex ThemeFile::findColorIndex(char *name)
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

void ThemePersistance::scanForThemes()
{
  themes.clear();
  TRACE("in scanForThemes");

  DIR dir;
  FILINFO fno;

  char fullPath[FF_MAX_LFN+1];
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
      strcat(fullPath, "defaulttheme.txt");

      FRESULT status = f_open(&file, fullPath, FA_READ);
      if (status != FR_OK)
        return;

      char line[256];
      uint len;
      status = f_read(&file, line, 256, &len);
      if (status == FR_OK) {
        refresh();

        line[len] = '\0';

        int index = 0;
        for (auto theme:themes) {
          if (theme->getPath() == std::string(line)) {
            applyTheme(index);
            setCurrentTheme(index+1);
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
      strcat(fullPath, "defaulttheme.txt");
      FIL file;

      FRESULT status = f_open(&file, fullPath, FA_CREATE_ALWAYS | FA_WRITE);
      if (status == FR_OK)
        f_close(&file);
    }

    void ThemePersistance::setDefaultTheme(int index)
    {
        char fullPath[128];
        strcpy(fullPath, getWorkingDirectory());
        strcat(fullPath, "defaulttheme.txt");
        FIL file;

        auto theme = themes[index];
        FRESULT status = f_open(&file, fullPath, FA_CREATE_ALWAYS | FA_WRITE);
        if (status != FR_OK)
            return;

        f_printf(&file, theme->getPath().c_str());
        f_close(&file);
    }
