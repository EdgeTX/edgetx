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

#include "keyboard_base.h"

class NumberEdit;

class NumberKeyboard : public Keyboard
{
 public:
  NumberKeyboard();
  ~NumberKeyboard() override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "NumberKeyboard"; }
#endif

  static void show(NumberEdit* field);

 protected:
  static NumberKeyboard* _instance;
};
