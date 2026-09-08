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

#include "window.h"

class FormField;

class Keyboard : public NavWindow
{
 public:
  explicit Keyboard(coord_t height);
  ~Keyboard();

  void clearField(bool wasCancelled);
  static void hide(bool wasCancelled);

  // Commit the field and close the keyboard because the "Enter" key was
  // pressed (distinct from the "OK" checkmark, so fields can send/submit).
  void enterField();

  static Keyboard* keyboardWindow() { return activeKeyboard; }

 protected:
  static Keyboard *activeKeyboard;

  bool hasTwoPageKeys;
  bool enterPressed = false;
  lv_group_t* group = nullptr;
  lv_obj_t* keyboard = nullptr;

  FormField* field = nullptr;
  Window* fieldContainer = nullptr;
  lv_group_t* fieldGroup = nullptr;
  lv_coord_t scroll_pos = 0;

  void setField(FormField* newField);
  bool attachKeyboard();
  void deleteLater() override;
};
