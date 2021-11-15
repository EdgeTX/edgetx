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
#include "file_carosell.h"

extern inline tmr10ms_t getTicks()
{
  return g_tmr10ms;
}

#define PAGE_INTERVAL ((1000 / 10) * 2)


FileCarosell::FileCarosell(Window *parent, const rect_t &rect,
                           std::vector<std::string> fileNames, FormField *nextCtrl) :
    FormGroup(parent, rect, NO_FOCUS | FORM_NO_BORDER),
    _fileNames(fileNames),
    fp(new FilePreview(this, {0, 0, rect.w, rect.h}, false))
{
  timer = getTicks();
  setSelected(0);
}

FileCarosell::~FileCarosell()
{
}

  void FileCarosell::setFileNames(std::vector<std::string> fileNames)
  {
    _fileNames = fileNames;
    selected = -1;  // so it passes the sameness comparison
    setSelected(0);
    timer = getTicks();
  }


void FileCarosell::checkEvents()
{
  FormGroup::checkEvents();

  uint32_t newTicks = getTicks();
  if (newTicks - timer > PAGE_INTERVAL && _fileNames.size()) {
    int newSelected = (selected + 1) % _fileNames.size();
    setSelected(newSelected);
    timer = newTicks;
  }
}
