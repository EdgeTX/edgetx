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
#include "page.h"

class ViewTextWindow : public Page
{
 public:
  ViewTextWindow(const std::string path, const std::string name,
                 EdgeTxIcon icon);

  FRESULT sdReadTextFileBlock(const uint32_t bufSize,
                              const uint32_t offset);

  ~ViewTextWindow()
  {
    if (buffer) {
      free(buffer);
      buffer = nullptr;
    }
  }

  void onCancel() override;

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
  virtual void buildBody(Window* window);

  bool openFile();

  void onEvent(event_t event) override;

  static void on_draw(lv_event_t * e);
};

void readModelNotes(bool fromMenu = false);
void readChecklist();
