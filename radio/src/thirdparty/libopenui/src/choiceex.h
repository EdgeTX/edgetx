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
#ifndef _CHOICEEX_H_
#define _CHOICEEX_H_
#include "choice.h"

typedef void (*lvHandler_t)(void*);
typedef struct {
  void* userData;
  bool isLongPressed;
  lvHandler_t lv_LongPressHandler;
} lv_eventData_t;

class ChoiceEx : public Choice
{
 public:
  ChoiceEx(Window* parent, const rect_t& rect, int16_t vmin, int16_t vmax,
           std::function<int16_t()> getValue,
           std::function<void(int16_t)> setValue = nullptr,
           WindowFlags windowFlags = 0);

  void set_lv_LongPressHandler(lvHandler_t longPressHandler, void* data);

  lv_eventData_t longPressData;

  void onClicked() override
  {
    if (!longPressData.isLongPressed) Choice::onClicked();
  }

 protected:
  std::function<void(event_t)> longPressHandler = nullptr;

  static void ClickHandler(lv_event_t* e)
  {
    ChoiceEx* ch = (ChoiceEx*)lv_event_get_user_data(e);
    if (ch) ch->longPressData.isLongPressed = false;
  }
};

#endif
