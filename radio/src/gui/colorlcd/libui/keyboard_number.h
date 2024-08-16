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
  ~NumberKeyboard() override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "NumberKeyboard"; }
#endif

  static void open(FormField* field);

  void handleEvent(const char* btn);

 protected:
  void decLarge();
  void decSmall();
  void incSmall();
  void incLarge();
  void setMIN();
  void setMAX();
  void setDEF();
  void changeSign();

#if defined(HARDWARE_KEYS)
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

  static NumberKeyboard* _instance;
};
