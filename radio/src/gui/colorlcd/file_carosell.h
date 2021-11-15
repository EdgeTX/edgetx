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
#include "color_editor.h"
#include "file_preview.h"
#include "file_carosell.h"
#include "tabsgroup.h"

class FileCarosell : public FormGroup
{
 public:
  FileCarosell(Window *parent, const rect_t &rect,
               std::vector<std::string> fileNames, FormField *nextCtrl = nullptr);
  ~FileCarosell();

  inline void setSelected(int n)
  {
    if (n == selected) return;
    if (n >= 0 && n < (int)_fileNames.size()) {
      selected = n;
      fp->setFile(_fileNames[selected].c_str());
    } else
      fp->setFile("");
  }

  void setFileNames(std::vector<std::string> fileNames);
  void checkEvents() override;

 protected:
  int selected = -1;
  std::vector<std::string> _fileNames;
  FilePreview *fp;
  uint32_t timer;
};
