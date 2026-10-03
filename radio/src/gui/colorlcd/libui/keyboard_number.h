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

class NumberKeyboard : public Keyboard
{
 public:
  NumberKeyboard();

  enum NumKBEvents
  {
    // Match button order in number_kb_map
    DEC_FASTSTEP = 0,
    DEC_STEP,
    INC_STEP,
    INC_FASTSTEP,
    SET_MIN,
    SET_DEFAULT,
    CHANGE_SIGN,
    SET_MAX,
  };

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "NumberKeyboard"; }
#endif

  static void open(FormField* field, int stepSmall, int stepLarge, LcdFlags textFlags,
                    bool hasChangeSign, std::function<void(uint16_t)> onEvent);

  void handleEvent(uint16_t btnId);

  static LAYOUT_VAL_SCALED(KEYBOARD_HEIGHT, 90)

 protected:

  std::function<void(uint16_t)> eventHandler = nullptr;

  void openKeyboard(FormField* field, int stepSmall, int stepLarge, LcdFlags textFlags,
                    bool hasChangeSign, std::function<void(uint16_t)> onEvent);

#if defined(HARDWARE_KEYS)
  void onKey(NumKBEvents e1, NumKBEvents e2);
  void onPressSYS() override;
  void onLongPressSYS() override;
  void onPressMDL() override;
  void onLongPressMDL() override;
  void onPressTELE() override;
  void onLongPressTELE() override;
  void onPressPGUP() override;
  void onPressPGDN() override;
#endif

  static NumberKeyboard* _instance;
};
