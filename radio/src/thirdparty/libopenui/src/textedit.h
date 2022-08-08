/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#pragma once

#include "form.h"

class TextEdit : public FormField
{

 public:
  TextEdit(Window* parent, const rect_t& rect, char* value, uint8_t length,
           LcdFlags windowFlags = 0);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "TextEdit"; }
#endif

  uint8_t getMaxLength() const { return length; }
  char* getData() const { return value; }

  void update();

 protected:
  static void event_cb(lv_event_t* e);
  
  char* value;
  uint8_t length;

  void trim();

  void changeEnd(bool forceChanged = false) override;
  void onClicked() override;
  void onFocusLost() override;
};
