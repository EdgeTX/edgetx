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

class TextEdit : public FormField
{
 public:
  TextEdit(Window* parent, const rect_t& rect, char* value, uint8_t length);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "TextEdit"; }
#endif

  uint8_t getMaxLength() const { return length; }
  char* getData() const { return value; }

  void update();

  static LAYOUT_VAL(DEF_W, 100, 100)

 protected:
  static void event_cb(lv_event_t* e);

  char* value;
  uint8_t length;

  void trim();

  void changeEnd(bool forceChanged = false) override;
  void onClicked() override;
};

class ModelTextEdit : public TextEdit
{
 public:
  ModelTextEdit(Window* parent, const rect_t& rect, char* value,
                uint8_t length);
};

class RadioTextEdit : public TextEdit
{
 public:
  RadioTextEdit(Window* parent, const rect_t& rect, char* value,
                uint8_t length);
};
