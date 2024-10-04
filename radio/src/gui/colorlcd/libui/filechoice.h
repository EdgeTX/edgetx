/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#include "choice.h"
#include <string>

class FileChoice : public Choice
{
public:
  FileChoice(Window* parent, const rect_t& rect, std::string folder,
             const char* extension, int maxlen,
             std::function<std::string()> getValue,
             std::function<void(std::string)> setValue,
             bool stripExtension = false,
             const char* title = "");

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "FileChoice"; }
#endif

protected:
  bool loaded = false;
  int fileCount = 0;
  int selectedIdx = -1;
  Menu* menu = nullptr;
  std::string getLabelText() override;
  std::string folder;
  const char* extension;
  int maxlen;
  std::function<std::string()> getValue;
  std::function<void(std::string)> setValue;
  bool stripExtension;

  void loadFiles();

  void openMenu() override;
};
