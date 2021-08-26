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

#ifndef _SOURCECHOICE_H_
#define _SOURCECHOICE_H_

#include "form.h"

class Menu;

bool isSourceAvailable(int source);

class SourceChoice : public FormField {
  template <class T> friend class MenuToolbar;

  public:
    SourceChoice(Window * parent, const rect_t & rect, int16_t vmin, int16_t vmax, std::function<int16_t()> getValue, 
                std::function<void(int16_t)> setValue, WindowFlags windowFlags = 0, LcdFlags textFlags = 0):
      FormField(parent, rect, windowFlags, textFlags),
      vmin(vmin),
      vmax(vmax),
      getValue(std::move(getValue)),
      setValue(std::move(setValue))
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "SourceChoice";
    }
#endif

    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

    void setAvailableHandler(std::function<bool(int)> handler)
    {
      isValueAvailable = std::move(handler);
    }

  protected:
    int16_t vmin;
    int16_t vmax;
    std::function<int16_t()> getValue;
    std::function<void(int16_t)> setValue;
    std::function<bool(int)> isValueAvailable = isSourceAvailable;
    void fillMenu(Menu * menu, const std::function<bool(int16_t)> & condition=nullptr);
    void openMenu();
};

#endif // _SOURCECHOICE_H_
