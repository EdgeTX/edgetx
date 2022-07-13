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

#include "ff.h"
#include "lcd.h"
#include "menus.h"
#include "page.h"
#include "static.h"

#include "LvglWrapper.h"

constexpr int maxTxtBuffSize = 64 * 1024;

class ViewTextWindow : public Page
{
 public:
  ViewTextWindow(const std::string path, const std::string name,
                 unsigned int icon = ICON_RADIO_SD_MANAGER) :
      Page(icon), path(std::move(path)), name(std::move(name))
  {
    fullPath = this->path + std::string("/") + this->name;
    extractNameSansExt();

    header.setTitle(this->name);
    buildBody(&body);
  };

  FRESULT sdReadTextFileBlock(const char* filename, const uint32_t bufSize,
                              const uint32_t offset);

  ~ViewTextWindow()
  {
    if (buffer) {
      free(buffer);
      buffer = nullptr;
    }
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ViewTextWindow"; };
#endif

 protected:
  std::string path;
  std::string name;
  std::string fullPath;
  std::string extension;

  lv_obj_t* lb;

  int offset = 0;
  char* buffer = nullptr;
  size_t bufSize = 0;
  int fileLength = 0;
  bool openFromEnd;

  void extractNameSansExt(void);
  void buildBody(Window* window);

  void onEvent(event_t event) override;
};

void readModelNotes();
