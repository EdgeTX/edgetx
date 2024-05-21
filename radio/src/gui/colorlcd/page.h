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

#include "bitmaps.h"
#include "button.h"
#include "static.h"

class Page;

class PageHeader : public Window
{
 public:
  PageHeader(Page* parent, EdgeTxIcon icon);

  void setTitle(std::string txt) { title->setText(std::move(txt)); }
  StaticText* setTitle2(std::string txt);

  static LAYOUT_VAL(PAGE_TITLE_LEFT, 50, 50)
  static constexpr coord_t PAGE_TITLE_TOP = 2;

 protected:
  EdgeTxIcon icon;
  StaticText* title;
  StaticText* title2 = nullptr;
};

class Page : public NavWindow
{
 public:
  explicit Page(EdgeTxIcon icon, PaddingSize padding = PAD_MEDIUM);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "Page"; }
#endif

  void onCancel() override;
  void onClicked() override;

  void deleteLater(bool detach = true, bool trash = true) override;

 protected:
  PageHeader* header = nullptr;
  Window* body = nullptr;

  void checkEvents() override;
  bool bubbleEvents() override { return false; }
};

class SubPage : public Page
{
 public:
  SubPage(EdgeTxIcon icon, const char* title, const char* subtitle);
  SubPage(EdgeTxIcon icon, const char* title, const char* subtitle, SetupLineDef* setupLines, int lineCount);

  static LAYOUT_VAL(EDT_X, 220, 144)
};
