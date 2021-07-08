/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
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

#include "page.h"
#include "static.h"
#include "lcd.h"
#include "menus.h"

constexpr uint16_t TEXT_FILE_MAXSIZE = 2048;

class ViewTextWindow : public Page
{
 public:
  ViewTextWindow(const std::string iPath, const std::string iName, unsigned int icon = ICON_RADIO_SD_MANAGER) :
      Page(icon),
      path(std::move(iPath)),
      name(std::move(iName)),
      icon(icon)
  {
    fullPath = path + std::string("/") + name;
    extractNameSansExt();

    //body.setWindowFlags(FORWARD_SCROLL);
    header.setWindowFlags(NO_SCROLLBAR);
    

    buildHeader(&header);
    buildBody(&body);
  };

  bool sdReadTextLine(const char *filename, char lines[],
                      const uint8_t lineLength = LCD_COLS); 
                                        
  virtual void checkEvents();

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ViewTextWindow"; };
#endif

 protected:
  std::string path;
  std::string name;
  std::string fullPath;
  std::string extension;
  unsigned int icon;

  bool lastLine;
  uint16_t readCount;
  int longestLine;

  void extractNameSansExt(void);
  void buildBody(Window *window);
  void buildHeader(Window *window)
  {
    new StaticText(window,
                   {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + 10,
                    LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                   name, 0, MENU_HIGHLIGHT_COLOR);
  };
};

void readModelNotes();