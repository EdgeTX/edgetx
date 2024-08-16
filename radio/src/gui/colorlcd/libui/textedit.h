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

#include "form.h"
#include "button.h"

class TextArea;

class TextEdit : public TextButton
{
 public:
  TextEdit(Window* parent, const rect_t& rect, char* text, uint8_t length,
           std::function<void(void)> updateHandler = nullptr);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "TextEdit \"" + text + "\""; }
#endif

  void preview(bool edited, char* text, uint8_t length);
  void update();

  static LAYOUT_VAL(DEF_W, 100, 100)

 protected:
  std::function<void(void)> updateHandler = nullptr;
  TextArea* edit = nullptr;
  char* text;
  uint8_t length;

  void openEdit();
};

class ModelTextEdit : public TextEdit
{
 public:
  ModelTextEdit(Window* parent, const rect_t& rect, char* value,
                uint8_t length, std::function<void(void)> updateHandler = nullptr);
};

class RadioTextEdit : public TextEdit
{
 public:
  RadioTextEdit(Window* parent, const rect_t& rect, char* value,
                uint8_t length);
};
