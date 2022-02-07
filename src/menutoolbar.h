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
#include <algorithm>

int getFirstAvailable(int min, int max, std::function<bool(int)>isValueAvailable);

class MenuToolbarButton: public Button {
  public:
    MenuToolbarButton(FormGroup * window, const rect_t & rect, char picto):
#if defined(HARDWARE_TOUCH)
      Button(window, rect, nullptr),
#else
      Button(window, rect, nullptr),
#endif
      picto(picto)
    {
    }

    void paint(BitmapBuffer * dc) override;

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
      lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, LV_PART_MAIN);
    }

    void paint(BitmapBuffer * dc) override
    {
//      dc->clear(COLOR_THEME_SECONDARY3);
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override
    {
      if (event == EVT_KEY_BREAK(KEY_PGDN)  || event == EVT_KEY_LONG(KEY_PGDN) || event == EVT_KEY_BREAK(KEY_PGUP)) {
#if defined(HARDWARE_TOUCH)
        if (current != children.end()) {
          static_cast<MenuToolbarButton *>(*current)->onTouchEnd(0,0);
        }
#endif
        current = IS_KEY_LONG(event) || event == EVT_KEY_BREAK(KEY_PGUP) ?
          current == children.end() ?
            --current:
            current == children.begin() ? 
              children.end() :
              --current : 
          current == children.end() ?
            children.begin() :
            ++current;

        if (IS_KEY_LONG(event))
          killEvents(event);

        if (current != children.end()) {
          auto button = static_cast<MenuToolbarButton *>(*current);
#if defined(HARDWARE_TOUCH)
          button->onTouchEnd(0,0);
#endif
          scrollTo(button);
        }
        else {
          lv_obj_scroll_to_y(lvobj, 0, LV_ANIM_OFF);
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
    std::vector<MenuToolbarButton *> menuButtons;

    void addButton(char picto, int16_t filtermin, int16_t filtermax)
    {
      int vmin = choice->vmin;
      int vmax = choice->vmax;

      if (vmin > filtermin || vmax < filtermin)
        return;

      if (choice->isValueAvailable &&
          getFirstAvailable(filtermin, filtermax, choice->isValueAvailable) ==
              0)
        return;

      auto button = new MenuToolbarButton(
          this, {0, y, MENUS_TOOLBAR_BUTTON_WIDTH, MENUS_TOOLBAR_BUTTON_WIDTH},
          picto);
      button->setPressHandler([=]() {
        auto val = choice->getValue();
        button->check(!button->checked());

        if (button->checked()) {
          choice->fillMenu(menu, val, [=](int16_t index) {
            return index >= filtermin && index <= filtermax;
          });
        } else {
          choice->fillMenu(menu, val);
        }
        menu->setFocusBody();

        // set current for page key procesing
        current = std::find_if(children.begin(), children.end(), [button](Window *o) { return o == button; });
        
        // clear all checked but the current button
        for (auto b : menuButtons) {
          if (b != button)
            b->check(false);
        }

        return button->checked();
      });

      menuButtons.emplace_back(button);


      y += MENUS_TOOLBAR_BUTTON_WIDTH;

      setInnerHeight(y);
    }
};


