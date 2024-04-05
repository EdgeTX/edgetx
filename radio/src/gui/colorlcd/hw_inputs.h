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

#pragma once

#include "button.h"
#include "dialog.h"
#include "form.h"

struct HWSticks : public Window {
  HWSticks(Window* parent);
};

struct HWPots : public Window {
  HWPots(Window* parent);
  bool potsChanged;
};

struct HWSwitches : public Window {
  HWSwitches(Window* parent);
};

template <class T>
struct HWInputDialog : public BaseDialog {
  HWInputDialog(const char* title = nullptr);
};

template <class T>
TextButton* makeHWInputButton(Window* parent, const char* title)
{
  return new TextButton(parent, rect_t{0, 0, 100, 0}, title, [=]() {
    new HWInputDialog<T>(title);
    return 0;
  });
}
