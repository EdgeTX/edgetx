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

#include "edgetx.h"
#include "pagegroup.h"

class FileCarosell;
class ListBox;
class PageGroup;
class StaticText;
class ThemeColorPreview;
class ThemeFile;
class ThemePersistance;

class ThemeSetupPage : public PageGroupItem
{
 public:
  ThemeSetupPage(const PageDef& pageDef);

  void build(Window *window) override;
  void checkEvents() override;

  static LAYOUT_VAL_SCALED(COLOR_PREVIEW_SIZE, 18)
  static LAYOUT_ORIENTATION(LIST_SIZE, (LCD_W - PAD_MEDIUM * 2) / 2 - COLOR_PREVIEW_SIZE, LCD_H / 2 - LAYOUT_SCALE(38));

 protected:
  Window *pageWindow = nullptr;
  Window *previewWindow = nullptr;
  FileCarosell *fileCarosell = nullptr;
  ThemeColorPreview *themeColorPreview = nullptr;
  ListBox *listBox = nullptr;
  StaticText *authorText = nullptr;
  StaticText *nameText = nullptr;
  int currentTheme = 0;
  bool started = false;

  void setupListbox(Window *window, rect_t r, ThemePersistance *tp);
  void displayThemeMenu(Window *window, ThemePersistance *tp);
  void setAuthor(ThemeFile *theme);
  void setName(ThemeFile *theme);
  void setSelected(ThemePersistance *tp);
};
