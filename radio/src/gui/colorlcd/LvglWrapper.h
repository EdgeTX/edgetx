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

#include <lvgl/lvgl.h>
#include "opentx_types.h"

lv_color_t makeLvColor(uint32_t colorFlags);

class LvglWidgetFactory;
typedef std::function<lv_obj_t *(lv_obj_t *parent)> LvObjConstructor;
typedef std::function<void (LvglWidgetFactory *factory)> LvStyleInitializer;

class LvglWidgetFactory
{
  public:
    LvglWidgetFactory(LvObjConstructor construct, LvStyleInitializer styleInitializer = nullptr) :
      _construct(std::move(construct)),
      _styleInitializer(std::move(styleInitializer))
    {
      lv_style_init(&style);
      styleInitialized = false;
    };

    lv_style_t style;

    inline lv_obj_t *construct(lv_obj_t *parent) { 
      if (!styleInitialized && _styleInitializer != nullptr) {
        (_styleInitializer(this));
        styleInitialized = true;
      }
      return _construct(parent); 
    }

  private:
    LvObjConstructor _construct;
    LvStyleInitializer _styleInitializer;
    bool styleInitialized;
};

class LvglWrapper
{
 public:
  static LvglWrapper* instance();

  void run();
  void pollInputs();

 private:
  LvglWrapper();
  ~LvglWrapper() {}

  static LvglWrapper *_instance;
  tmr10ms_t lastTick = 0;
};

#endif // _LVGLWRAPPER_H_
