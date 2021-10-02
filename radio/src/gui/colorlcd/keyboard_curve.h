/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
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

#ifndef _KEYBOARD_CURVE_H_
#define _KEYBOARD_CURVE_H_

#include "keyboard_base.h"

class Button;

class CurveKeyboard : public Keyboard {
  public:
    CurveKeyboard();

    ~CurveKeyboard() override;

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "CurveKeyboard";
    }
#endif

    void enableRightLeft(bool enable);

    static void show(FormField * field, bool enableRightLeft)
    {
      if (!_instance)
        _instance = new CurveKeyboard();
      _instance->setField(field);
      _instance->enableRightLeft(enableRightLeft);
    }

    void paint(BitmapBuffer * dc) override;

  protected:
    static CurveKeyboard * _instance;
    Button * left = nullptr;
    Button * right = nullptr;
};

#endif // _KEYBOARD_CURVE_H_
