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

#include "window.h"

class ModalWindow : public Window
{
 public:
  explicit ModalWindow(Window* parent, bool closeWhenClickOutside = false);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ModalWindow"; }
#endif

  void setCloseWhenClickOutside(bool value = true)
  {
    closeWhenClickOutside = value;
  }

  void onClicked() override;
  void deleteLater(bool detach = true, bool trash = true) override;

 protected:
  bool closeWhenClickOutside;
};

class ModalWindowContent : public Window
{
 public:
  explicit ModalWindowContent(ModalWindow* parent, const rect_t& rect);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ModalWindowContent"; }
#endif

  void setTitle(const std::string& text);
  std::string getTitle() const;

  void onClicked() override;

 protected:
  lv_obj_t* title = nullptr;
};
