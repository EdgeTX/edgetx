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

#include "menutoolbar.h"
#include "choice.h"
#include "font.h"

MenuToolbarButton::MenuToolbarButton(Window* parent, const rect_t& rect,
                                     const char* picto) :
    Button(parent, rect, nullptr, 0, 0, window_create), picto(picto)
{
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
}

void MenuToolbarButton::paint(BitmapBuffer * dc)
{
  if (checked()) {
    dc->drawSolidFilledRect(
        MENUS_TOOLBAR_BUTTON_PADDING, MENUS_TOOLBAR_BUTTON_PADDING,
        MENUS_TOOLBAR_BUTTON_WIDTH - 2 * MENUS_TOOLBAR_BUTTON_PADDING,
        MENUS_TOOLBAR_BUTTON_WIDTH - 2 * MENUS_TOOLBAR_BUTTON_PADDING,
        COLOR_THEME_FOCUS);
    dc->drawText(rect.w / 2, (rect.h - getFontHeight(FONT(STD))) / 2 + 1,
                 picto, CENTERED | COLOR_THEME_PRIMARY2);
  } else {
    dc->drawText(rect.w / 2, (rect.h - getFontHeight(FONT(STD))) / 2 + 1,
                 picto, CENTERED | COLOR_THEME_PRIMARY1);
  }
}

MenuToolbar::MenuToolbar(Choice* choice, Menu* menu) :
    Window(menu, MENUS_TOOLBAR_RECT, NO_SCROLLBAR),
    choice(choice),
    menu(menu),
    group(lv_group_create())
{
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, LV_PART_MAIN);
}

MenuToolbar::~MenuToolbar()
{
  lv_group_del(group);
}

void MenuToolbar::onEvent(event_t event)
{
  lv_obj_t* obj = lv_group_get_focused(group);
  if (obj) {
    auto btn = (MenuToolbarButton*)lv_obj_get_user_data(obj);
    if (btn) { btn->check(false); }
  }
  
  if (event == EVT_KEY_BREAK(KEY_PGDN)) {
    lv_group_focus_next(group);
  } else if (event == EVT_KEY_LONG(KEY_PGDN) ||
             event == EVT_KEY_BREAK(KEY_PGUP)) {
    lv_group_focus_prev(group);
  }
    
  obj = lv_group_get_focused(group);
  if (!obj) {
    choice->fillMenu(menu);
  } else {
    auto btn = (MenuToolbarButton*)lv_obj_get_user_data(obj);
    if (btn) btn->onClicked();
  }
}

static rect_t _get_button_rect(size_t buttons)
{
  coord_t y = buttons * MENUS_TOOLBAR_BUTTON_WIDTH;
  return {0, y, MENUS_TOOLBAR_BUTTON_WIDTH, MENUS_TOOLBAR_BUTTON_WIDTH};
}

bool MenuToolbar::filterMenu(MenuToolbarButton* btn, int16_t filtermin,
                             int16_t filtermax)
{
  btn->check(!btn->checked());

  Choice::FilterFct filter = nullptr;
  if (btn->checked()) {
    filter = [=](int16_t index) {
      return index >= filtermin && index <= filtermax;
    };
  }

  choice->fillMenu(menu, filter);
  return btn->checked();
}

typedef std::function<bool(int)> IsValueAvailable;
static int getFirstAvailable(int min, int max, IsValueAvailable isValueAvailable)
{
  int retval = 0;
  for (int i = min; i <= max; i++) {
    if (isValueAvailable(i)) {
      retval = i;
      break;
    }
  }
  return retval;
}


void MenuToolbar::addButton(const char* picto, int16_t filtermin,
                            int16_t filtermax)
{
  int vmin = choice->vmin;
  int vmax = choice->vmax;

  if (vmin > filtermin || vmax < filtermin) return;

  if (choice->isValueAvailable &&
      getFirstAvailable(filtermin, filtermax, choice->isValueAvailable) == 0)
    return;

  rect_t r = _get_button_rect(children.size());
  auto button = new MenuToolbarButton(this, r, picto);

  button->setPressHandler(
      std::bind(&MenuToolbar::filterMenu, this, button, filtermin, filtermax));

  lv_group_add_obj(group, button->getLvObj());
}
