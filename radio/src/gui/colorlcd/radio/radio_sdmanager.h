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

#include "dataconstants.h"
#include "tabsgroup.h"

enum MultiModuleType : short;

class FileBrowser;
class FilePreview;

class RadioSdManagerPage : public PageTab
{
  FileBrowser* browser = nullptr;
  FilePreview* preview = nullptr;
  
 public:
  RadioSdManagerPage();
  void build(Window* window) override;

  static LAYOUT_VAL(PREVIEW_W, LCD_W * 2 / 5 - PAD_SMALL * 2, LCD_W - PAD_MEDIUM * 2)
  static LAYOUT_VAL(PREVIEW_H, LCD_H - 68, (LCD_H - 68) / 3)

 protected:
  int loadPreview = 0;
  Window* loading = nullptr;
  const char* previewFilename = nullptr;

  void fileAction(const char* path, const char* name, const char* fullpath);
  void dirAction(const char* path, const char* name, const char* fullpath);
  
  void BootloaderUpdate(const char* fn);
#if defined(BLUETOOTH)
  void BluetoothFirmwareUpdate(const char* fn);
#endif
  void FrSkyFirmwareUpdate(const char* fn, ModuleIndex module);
  void MultiFirmwareUpdate(const char* fn, ModuleIndex module,
                           MultiModuleType type);

  void checkEvents() override; 
};
