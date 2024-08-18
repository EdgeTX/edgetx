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

#include "tabsgroup.h"
#include "edgetx.h"

class ListBox;
class TabsGroup;
class ThemeColorPreview;
class ThemePersistance;
class ThemeFile;
class FileCarosell;

class ThemeSetupPage : public PageTab
{
 public:
  ThemeSetupPage(TabsGroup *tabsGroup);

  void build(Window *window) override;
  void checkEvents() override;

  bool isVisible() const override { return radioThemesEnabled(); }

  static LAYOUT_VAL(COLOR_PREVIEW_SIZE, 18, 18)
#if PORTRAIT_LCD
  static constexpr int LIST_HEIGHT = (LCD_H / 2 - 38);
#else
  static constexpr int LIST_WIDTH = ((LCD_W - PAD_MEDIUM * 2) / 2 - COLOR_PREVIEW_SIZE);
#endif

 protected:
  TabsGroup *tabsGroup = nullptr;
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
