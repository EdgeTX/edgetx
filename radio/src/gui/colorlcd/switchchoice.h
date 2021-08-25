/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _SWITCHCHOICE_H_
#define _SWITCHCHOICE_H_

#include "form.h"

constexpr int LONG_PRESS_10MS = 100;

class Menu;
bool isSwitchAvailableInMixes(int swtch);

class SwitchChoice : public FormField
{
  template <class T>
  friend class MenuToolbar;

 public:
  SwitchChoice(Window* parent, const rect_t& rect, int vmin, int vmax,
               std::function<int16_t()> getValue,
               std::function<void(int16_t)> setValue) :
      FormField(parent, rect),
      vmin(vmin),
      vmax(vmax),
      getValue(std::move(getValue)),
      setValue(std::move(setValue))
  {
#if defined(HARDWARE_TOUCH)
    duration10ms = 0;
#endif    
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "SwitchChoice"; }
#endif

  void paint(BitmapBuffer* dc) override;

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
  void checkEvents(void) override;
  bool onTouchEnd(coord_t x, coord_t y) override;
  bool onTouchStart(coord_t x, coord_t y) override;
  inline bool isLongPress(tmr10ms_t longPressDuration = LONG_PRESS_10MS);
#endif

  void setAvailableHandler(std::function<bool(int)> handler)
  {
    isValueAvailable = std::move(handler);
  }

 protected:
  int16_t vmin;
  int16_t vmax;
#if defined(HARDWARE_TOUCH)
    tmr10ms_t duration10ms;
#endif
  std::function<int16_t()> getValue;
  std::function<void(int16_t)> setValue;
  std::function<bool(int)> isValueAvailable = isSwitchAvailableInMixes;
  void fillMenu(Menu* menu, std::function<bool(int16_t)> condition = nullptr);
  void openMenu();
};

#endif // _SWITCHCHOICE_H_
