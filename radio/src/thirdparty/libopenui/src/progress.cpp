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

#include "progress.h"
#include "theme.h"

Progress::Progress(Window * parent, const rect_t & rect):
    Window(parent, rect)
{
  bar = etx_bar_create(lvobj);
  lv_bar_set_range(bar, 0, 100);
  setValue(0);
}

void Progress::setValue(int newValue)
{
  if (newValue != value) {
    value = newValue;
    lv_bar_set_value(bar, value, LV_ANIM_OFF);
  }
}
