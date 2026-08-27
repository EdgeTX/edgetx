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

#include "table.h"

class FileBrowser : public TableField
{
 public:
  // path, name, fullpath
  typedef std::function<void(const char*, const char*, const char*, bool isDir)> FileAction;

  FileBrowser(Window* parent, const rect_t& rect, const char* dir);

  void setFileAction(FileAction fct);
  void setFileSelected(FileAction fct);
  void refresh();

  void adjustWidth();

  void onDrawBegin(uint16_t row, uint16_t col,
                   lv_obj_draw_part_dsc_t* dsc) override;
  void onDrawEnd(uint16_t row, uint16_t col,
                 lv_obj_draw_part_dsc_t* dsc) override;

  // TableField methods
  void onSelected(uint16_t row, uint16_t col) override;
  void onPress(uint16_t row, uint16_t col) override;

 protected:
  void onSelected(const char* name, bool is_dir);
  void onPress(const char* name, bool is_dir);
  void onPressLong(const char* name, bool is_dir);

 private:
  const char* selected = nullptr;
  FileAction fileAction;
  FileAction fileSelected;
};
