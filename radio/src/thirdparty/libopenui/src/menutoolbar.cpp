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

#include "menutoolbar.h"

#include "font.h"
#include "translations.h"

static void toolbar_btn_defocus(lv_event_t* event)
{
  auto obj = lv_event_get_target(event);
  auto btn = (MenuToolbarButton*)lv_obj_get_user_data(obj);
  btn->check(false);
}

MenuToolbarButton::MenuToolbarButton(Window* parent, const rect_t& rect,
                                     const char* picto) :
    Button(parent, rect, nullptr, 0, 0, etx_menu_button_create)
{
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);
  lv_obj_add_flag(lvobj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

  lv_obj_add_event_cb(lvobj, toolbar_btn_defocus, LV_EVENT_DEFOCUSED, nullptr);

  auto label = lv_label_create(lvobj);
  lv_label_set_text(label, picto);
  lv_obj_center(label);
}

MenuToolbar::MenuToolbar(Choice* choice, Menu* menu, const int columns) :
    Window(menu, {0, (LCD_H - MENUS_MAX_HEIGHT) / 2, 76, MENUS_MAX_HEIGHT},
           OPAQUE),
    choice(choice),
    menu(menu),
    filterColumns(columns),
    group(lv_group_create())
{
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_SECONDARY3), 0);

  setWidth((MENUS_TOOLBAR_BUTTON_WIDTH + MENUS_TOOLBAR_BUTTON_PADDING) *
               columns +
           MENUS_TOOLBAR_BUTTON_PADDING);

  addButton(STR_SELECT_MENU_ALL, choice->getMin(), choice->getMax(), nullptr,
            nullptr, true);
}

MenuToolbar::~MenuToolbar() { lv_group_del(group); }

void MenuToolbar::resetFilter()
{
  if (lv_group_get_focused(group) != lvobj) {
    lv_group_focus_obj(lvobj);
    choice->fillMenu(menu);
    menu->setTitle(choice->menuTitle);
  }
}

void MenuToolbar::onEvent(event_t event)
{
  if (event == EVT_KEY_BREAK(KEY_PAGEDN)) {
    lv_group_focus_next(group);
  }
#if defined(KEYS_GPIO_REG_PAGEUP)
  else if (event == EVT_KEY_BREAK(KEY_PAGEUP)) {
#else
  else if (event == EVT_KEY_LONG(KEY_PAGEDN)) {
    killEvents(event);
#endif
    lv_group_focus_prev(group);
  }

  auto obj = lv_group_get_focused(group);
  lv_event_send(obj, LV_EVENT_CLICKED, nullptr);
}

rect_t MenuToolbar::getButtonRect(bool wideButton)
{
  if (wideButton && (nxtBtnPos % filterColumns))
    nxtBtnPos = nxtBtnPos - (nxtBtnPos % filterColumns) + filterColumns;
  coord_t x = (nxtBtnPos % filterColumns) *
                  (MENUS_TOOLBAR_BUTTON_WIDTH + MENUS_TOOLBAR_BUTTON_PADDING) +
              MENUS_TOOLBAR_BUTTON_PADDING;
  coord_t y = (nxtBtnPos / filterColumns) *
                  (MENUS_TOOLBAR_BUTTON_WIDTH + MENUS_TOOLBAR_BUTTON_PADDING) +
              MENUS_TOOLBAR_BUTTON_PADDING;
  coord_t w =
      wideButton ? (MENUS_TOOLBAR_BUTTON_WIDTH + MENUS_TOOLBAR_BUTTON_PADDING) *
                           (filterColumns - 1) +
                       MENUS_TOOLBAR_BUTTON_WIDTH
                 : MENUS_TOOLBAR_BUTTON_WIDTH;
  nxtBtnPos += wideButton ? filterColumns : 1;
  return {x, y, w, MENUS_TOOLBAR_BUTTON_WIDTH};
}

bool MenuToolbar::filterMenu(MenuToolbarButton* btn, int16_t filtermin,
                             int16_t filtermax,
                             const Choice::FilterFct& filterFunc,
                             const char* title)
{
  btn->check(!btn->checked());

  filter = nullptr;
  if (btn->checked()) {
    if (title)
      menu->setTitle(title);
    else
      menu->setTitle(choice->menuTitle);
    filter = [=](int16_t index) {
      if (filterFunc) return filterFunc(index);
      return index == 0 || (abs(index) >= filtermin && abs(index) <= filtermax);
    };
    lv_group_focus_obj(btn->getLvObj());
    choice->fillMenu(menu, filter);
  } else {
    lv_event_send(allBtn->getLvObj(), LV_EVENT_CLICKED, nullptr);
  }

  return btn->checked();
}

void MenuToolbar::addButton(const char* picto, int16_t filtermin,
                            int16_t filtermax,
                            const Choice::FilterFct& filterFunc,
                            const char* title, bool wideButton)
{
  int vmin = choice->vmin;
  int vmax = choice->vmax;

  if (vmin > filtermin || vmax < filtermin) return;

  if (choice->isValueAvailable) {
    bool found = false;
    for (int i = filtermin; i <= filtermax; i += 1) {
      if (choice->isValueAvailable(i)) {
        if (filterFunc && !filterFunc(i))
          continue;
        found = true;
        break;
      }
    }
    if (!found) return;
  }

  rect_t r = getButtonRect(wideButton);
  auto button = new MenuToolbarButton(this, r, picto);

  button->setPressHandler(std::bind(&MenuToolbar::filterMenu, this, button,
                                    filtermin, filtermax, filterFunc, title));

  lv_group_add_obj(group, button->getLvObj());

  if (children.size() == 1) {
    allBtn = button;
    lv_event_send(allBtn->getLvObj(), LV_EVENT_CLICKED, nullptr);
  }
}
