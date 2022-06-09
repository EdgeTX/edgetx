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

#ifndef _PAGE_H_
#define _PAGE_H_

#include "window.h"
#include "button.h"
#include "static.h"

class Page;

class PageHeader : public FormGroup
{
 public:
  PageHeader(Page* parent, uint8_t icon);

  uint8_t getIcon() const { return icon; }
  void setTitle(std::string txt) { title->setText(std::move(txt)); }

  void paint(BitmapBuffer* dc) override;

 protected:
  uint8_t icon;
  StaticText* title;
};

class Page : public Window
{
 public:
  explicit Page(unsigned icon);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "Page"; }
#endif

  void onCancel() override;
  void onClicked() override;

  void paint(BitmapBuffer* dc) override;
  void deleteLater(bool detach = true, bool trash = true) override;

 protected:
  PageHeader header;
  FormWindow body;
};

#endif // _PAGE_H_
