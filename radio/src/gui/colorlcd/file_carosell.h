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

#include "file_preview.h"
#include "file_carosell.h"

#define PAGE_INTERVAL ((1000 / 10) * 2)
#define SHORT_PAGE_INTERVAL ((1000 / 20))  // 500 MS

class FileCarosell : public Window
{
 public:
  FileCarosell(Window *parent, const rect_t &rect,
               std::vector<std::string> fileNames);

  inline void setSelected(int n);
  void setFileNames(std::vector<std::string> fileNames);
  void checkEvents() override;
  inline void pause(bool pause)
  {
    _paused = pause;
  }

 protected:
  bool _paused = false;
  int selected = -1;
  std::vector<std::string> _fileNames;
  FilePreview *fp;
  uint32_t timer;
  uint32_t pageInterval = PAGE_INTERVAL;
  StaticText* message;
};
