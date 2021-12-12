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

#ifndef _LVGLWRAPPER_H_
#define _LVGLWRAPPER_H_

#include "lvgl/lvgl.h"

struct GuiWidget
{
  lv_obj_t obj;
};

class LvglWrapper {
public:
  LvglWrapper();
  ~LvglWrapper() {}

  static LvglWrapper* instance()
  {
    if (!_instance)
      _instance = new LvglWrapper();

    return _instance;
  }

private:
  static LvglWrapper* _instance;
};

#endif // _LVGLWRAPPER_H_
