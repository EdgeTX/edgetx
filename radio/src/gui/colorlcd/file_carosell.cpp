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

#include "translations.h"

extern inline tmr10ms_t getTicks()
{
  return g_tmr10ms;
}

FileCarosell::FileCarosell(Window *parent, const rect_t &rect,
                           std::vector<std::string> fileNames) :
    Window(parent, rect),
    fp(new FilePreview(this, {0, 0, rect.w, rect.h}))
{
  setWindowFlag(NO_FOCUS);

  message = new StaticText(this, {0, rect.h/2, rect.w, EdgeTxStyles::PAGE_LINE_HEIGHT * 2}, "", COLOR_THEME_PRIMARY1_INDEX, CENTERED | FONT(L));

  setFileNames(fileNames);
}

void FileCarosell::setFileNames(std::vector<std::string> fileNames)
{
  _fileNames = fileNames;
  setSelected(-1);
  timer = getTicks();
  pageInterval = SHORT_PAGE_INTERVAL;
}

void FileCarosell::setSelected(int n)
{
  if (n != selected) {
    selected = n;

    if (n >= 0 && n < (int)_fileNames.size()) {
      fp->setFile(_fileNames[selected].c_str());
    } else
      fp->setFile("");
  }
  
  message->show(selected == -1);
  if (selected == -1)
    message->setText(_fileNames.size() > 0 ? STR_LOADING : STR_NO_THEME_IMAGE);
}

void FileCarosell::checkEvents()
{
  Window::checkEvents();

  uint32_t newTicks = getTicks();

  // if we are paused then just update time.  we will begin the carosell after
  // timeout period once unpaused
  if (_paused) {
    timer = newTicks;
  } else if (newTicks - timer > pageInterval && _fileNames.size()) {
    int newSelected = (selected + 1) % _fileNames.size();
    setSelected(newSelected);
    timer = newTicks;
    pageInterval = PAGE_INTERVAL;
  }
}
