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

#ifndef _GVAR_NUMBEREDIT_H_
#define _GVAR_NUMBEREDIT_H_

#include "form.h"
#include "choice.h"
#include "numberedit.h"
#include "gvars.h"

constexpr coord_t GVAR_BUTTON_WIDTH = 30;

class GVarNumberEdit : public Window
{
 public:
  GVarNumberEdit(Window* parent, const rect_t& rect, int32_t vmin, int32_t vmax,
                 std::function<int32_t()> getValue,
                 std::function<void(int32_t)> setValue,
                 LcdFlags textFlags = 0, int32_t voffset = 0);

  void switchGVarMode();
  void setSuffix(std::string value);

 protected:
  Choice* gvar_field = nullptr;
  NumberEdit* num_field = nullptr;
  FormField* act_field = nullptr;

  int32_t vmin;
  int32_t vmax;
  std::function<int32_t()> getValue;
  std::function<void(int32_t)> setValue;
  LcdFlags textFlags;
  int32_t voffset;

  void onEvent(event_t event) override;
  void update();

  static void value_changed(lv_event_t* e);
};

#endif // _GVAR_NUMBEREDIT_H_
