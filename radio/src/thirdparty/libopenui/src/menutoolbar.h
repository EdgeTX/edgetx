/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#pragma once

#include "button.h"
#include "menu.h"

int getFirstAvailable(int min, int max, std::function<bool(int)>isValueAvailable);

class MenuToolbarButton: public Button {
  public:
    MenuToolbarButton(FormGroup * window, const rect_t & rect, char picto):
#if defined(HARDWARE_TOUCH)
      Button(window, rect, nullptr, BUTTON_CHECKED_ON_FOCUS),
#else
      Button(window, rect, nullptr),
#endif
      picto(picto)
    {
    }

    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override
    {
      if (hasFocus()) {
        check(false);
        clearFocus();
      }
      else {
        setFocus(SET_FOCUS_DEFAULT);
      }
      onPress();
      return true;
    }
#endif

    void check(bool checked=true)
    {
      Button::check(checked);
      onPress();
      Button::check(checked);
    }

  protected:
    char picto;
};

template <class T>
class MenuToolbar: public FormGroup {
  friend T;

  public:
    MenuToolbar(T * choice, Menu * menu):
      FormGroup(menu, MENUS_TOOLBAR_RECT, NO_SCROLLBAR),
      choice(choice),
      menu(menu)
    {
    }

    void paint(BitmapBuffer * dc) override
    {
      dc->clear(DISABLE_COLOR);
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override
    {
      if (event == EVT_KEY_BREAK(KEY_PGDN)) {
        if (current == children.end()) {
          current = children.begin();
        }
        else {
          static_cast<MenuToolbarButton *>(*current)->check(false);
          ++current;
        }
        if (current != children.end()) {
          auto button = static_cast<MenuToolbarButton *>(*current);
          button->check(true);
          scrollTo(button);
        }
        else {
          setScrollPositionY(0);
        }
      }
      else if (event == EVT_KEY_LONG(KEY_PGDN)) {
        killEvents(event);
        if (current == children.end()) {
          --current;
        }
        else {
          static_cast<MenuToolbarButton *>(*current)->check(false);
          if (current == children.begin())
            current = children.end();
          else
            --current;
        }
        if (current != children.end()) {
          auto button = static_cast<MenuToolbarButton *>(*current);
          button->check(true);
          scrollTo(button);
        }
        else {
          setScrollPositionY(0);
        }
      }
    }
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override
    {
      Window::onTouchEnd(x, y);
      return true; // = don't close the menu (inverted so that click outside the menu closes it)
    }
#endif

  protected:
    std::list<Window *>::iterator current = children.end();
    T * choice;
    Menu * menu;
    coord_t y = 0;

    void addButton(char picto, int16_t filtermin, int16_t filtermax)
    {
      int vmin = choice->vmin;
      int vmax = choice->vmax;

      if (vmin > filtermin || vmax < filtermin)
        return;

      if (choice->isValueAvailable && getFirstAvailable(filtermin, filtermax, choice->isValueAvailable) == 0)
        return;

      auto button = new MenuToolbarButton(this, {0, y, MENUS_TOOLBAR_BUTTON_WIDTH, MENUS_TOOLBAR_BUTTON_WIDTH}, picto);
      button->setPressHandler([=]() {
        if (button->checked()) {
          choice->fillMenu(menu, [=](int16_t index) {
            return index >= filtermin && index <= filtermax;
          });
        }
        else {
          choice->fillMenu(menu);
        }
        return 1;
      });

      y += MENUS_TOOLBAR_BUTTON_WIDTH;

      setInnerHeight(y);
    }
};


