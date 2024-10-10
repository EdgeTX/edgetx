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

#include "timeedit.h"

#include "strhelpers.h"

TimeEdit::TimeEdit(Window * parent, const rect_t & rect, int32_t vmin, int32_t vmax, std::function<int32_t()> getValue, std::function<void(int32_t)> setValue):
  NumberEdit(parent, rect, vmin, vmax, std::move(getValue), std::move(setValue))
{
  setDisplayHandler([=] (int value) {
    TimerOptions timerOptions;
    timerOptions.options = (textFlags & TIMEHOUR) != 0 ? SHOW_TIME : SHOW_TIMER;
    return std::string(getTimerString(this->getValue(), timerOptions));
  });
}
