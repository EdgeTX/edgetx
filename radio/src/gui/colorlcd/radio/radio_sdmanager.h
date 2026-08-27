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
#include "pagegroup.h"

enum MultiModuleType : short;

class FileBrowser;
class FilePreview;

class RadioSdManagerPage : public PageGroupItem
{
  FileBrowser* browser = nullptr;
  FilePreview* preview = nullptr;

 public:
  RadioSdManagerPage(const PageDef& pageDef);
  void build(Window* window) override;

 protected:
  int loadPreview = 0;
  Window* loading = nullptr;
  const char* previewFilename = nullptr;

  void fileAction(const char* path, const char* name, const char* fullpath);
  void dirAction(const char* path, const char* name, const char* fullpath);

#if defined(FIRMWARE_FORMAT_UF2)
  void FirmwareUpdate(const char* fn);
#else
  void BootloaderUpdate(const char* fn);
#endif
#if defined(BLUETOOTH)
  void BluetoothFirmwareUpdate(const char* fn);
#endif
#if defined(HARDWARE_INTERNAL_MODULE) || defined(HARDWARE_EXTERNAL_MODULE)
  void FrSkyFirmwareUpdate(const char* fn, ModuleIndex module);
  void MultiFirmwareUpdate(const char* fn, ModuleIndex module,
                           MultiModuleType type);
#endif

  void checkEvents() override;
};
