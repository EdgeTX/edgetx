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

#include "button.h"
#include "menu.h"

class Choice;

class MenuToolbarButton : public Button
{
 public:
  MenuToolbarButton(Window* parent, const rect_t& rect, const char* picto);
  void paint(BitmapBuffer* dc) override;

 protected:
  const char* picto;
};

class MenuToolbar : public Window
{
  // friend Menu;

 public:
  MenuToolbar(Choice* choice, Menu* menu);
  ~MenuToolbar();

  void onEvent(event_t event) override;

 protected:
  Choice* choice;
  Menu* menu;

  lv_group_t* group;

  void addButton(const char* picto, int16_t filtermin, int16_t filtermax);
  bool filterMenu(MenuToolbarButton* btn, int16_t filtermin, int16_t filtermax);

  rect_t getButtonRect(size_t buttons);
};
