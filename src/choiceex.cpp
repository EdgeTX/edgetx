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
#include "choiceex.h"

static void localLongPressHandler(lv_event_t* e)
{
  lv_eventData_t* ld = (lv_eventData_t*)lv_event_get_user_data(e);
  ld->isLongPressed = true;
  ld->lv_LongPressHandler(ld->userData);
}

void ChoiceEx::set_lv_LongPressHandler(lvHandler_t longPressHandler, void* data)
{
  TRACE("longPressHandler=%p", longPressHandler);

  if (longPressHandler) {
    longPressData.userData = data;
    longPressData.lv_LongPressHandler = longPressHandler;
    lv_obj_add_event_cb(lvobj, localLongPressHandler, LV_EVENT_LONG_PRESSED,
                        &longPressData);
    lv_obj_add_event_cb(lvobj, ClickHandler, LV_EVENT_CLICKED, this);
  }
}

ChoiceEx::ChoiceEx(Window* parent, const rect_t& rect, int16_t vmin,
                   int16_t vmax, std::function<int16_t()> getValue,
                   std::function<void(int16_t)> setValue,
                   WindowFlags windowFlags) :
    Choice(parent, rect, vmin, vmax, getValue, setValue, windowFlags)
{
  longPressData.isLongPressed = false;
}
