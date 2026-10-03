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

#include "keyboard_base.h"

class TextKeyboard : public Keyboard
{
 public:
  TextKeyboard();

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "TextKeyboard"; }
#endif

  static void open(FormField* field);

 protected:
#if defined(HARDWARE_KEYS)
  void changeMode();
  void cursorLeft();
  void cursorRight();
  void cursorStart();
  void cursorEnd();
  void toggleCase();
  void deleteChar();
  void backspace();

  void onPressSYS() override;
  void onLongPressSYS() override;
  void onPressMDL() override;
  void onLongPressMDL() override;
  void onPressTELE() override;
  void onLongPressTELE() override;
  void onPressPGUP() override;
  void onPressPGDN() override;
  void onLongPressPGUP() override;
  void onLongPressPGDN() override;
#endif

  static TextKeyboard* _instance;
};
