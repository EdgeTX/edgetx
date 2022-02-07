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

#include "menu.h"
#include "font.h"
#include "theme.h"

LvglWidgetFactory menuBodyWidgetFactory = { [] (lv_obj_t *parent) {
  return lv_list_create(parent);
}, nullptr};

void menuBodyEventCallback(lv_event_t *e)
{
  auto code = lv_event_get_code(e);
  if (code == LV_EVENT_CLICKED) {
    lv_obj_t *target = lv_event_get_target(e);
    if (target != nullptr) {
      long index = (long)lv_event_get_user_data(e);
      MenuBody *mb = (MenuBody *) lv_obj_get_user_data(lv_obj_get_parent(target));
      Menu *menu = mb->getParentMenu();
      if (index < (int)mb->lines.size()) {
        onKeyPress();
        if (menu->multiple) {
          if (mb->selectedIndex == index)
            mb->lines[index].onPress();
          else
            mb->setIndex(index);
        }
        else {
          mb->setIndex(index);
          menu->deleteLater();
          mb->lines[index].onPress();
        }
      }
    }
  }
}

MenuBody::MenuBody(Window * parent, const rect_t & rect):
  Window(parent, rect, OPAQUE, 0, &menuBodyWidgetFactory)
{
  setPageHeight(MENUS_LINE_HEIGHT);
}

void MenuBody::addLine(const std::string & text, std::function<void()> onPress, std::function<bool()> isChecked)
{
  lines.emplace_back(text, std::move(onPress), std::move(isChecked));
  lv_obj_t *btn = lv_list_add_btn(lvobj, nullptr, text.c_str());
  lv_obj_set_style_bg_color(btn, makeLvColor(COLOR_THEME_SECONDARY2), LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_style_bg_color(btn, makeLvColor(COLOR_THEME_SECONDARY2), LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_FOCUSED);
  lv_obj_set_style_bg_opa(btn, LV_OPA_100, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(btn, LV_OPA_100, LV_PART_MAIN | LV_STATE_CHECKED);
  lv_obj_set_style_bg_opa(btn, LV_OPA_100, LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_FOCUSED);
  lv_obj_add_flag(btn, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_event_cb(btn, menuBodyEventCallback, LV_EVENT_CLICKED, (void *) (lines.size() - 1));
  lv_obj_set_style_text_color(lvobj, makeLvColor(COLOR_THEME_PRIMARY2), LV_PART_MAIN | LV_STATE_FOCUSED);
}

void MenuBody::removeLines()
{
  lines.clear();
  lv_obj_clean(lvobj);
}

// ensure index is in range and also handle wrapping index
int MenuBody::rangeCheck(int index)
{
    if (index < 0) 
      index = lines.size() - 1;
    else if (index > (signed)lines.size() - 1)
      index = 0;

    return index;
}

void MenuBody::setIndex(int index)
{
  if (index < (int)lines.size()) {
    lv_obj_t *btn = lv_obj_get_child(lvobj, selectedIndex);
    if (btn != nullptr) {
      lv_obj_clear_state(btn, LV_STATE_CHECKED);
    }

    selectedIndex = index;

    btn = lv_obj_get_child(lvobj, selectedIndex);
    if (btn != nullptr) {
      lv_obj_add_state(btn, LV_STATE_CHECKED);
      lv_obj_scroll_to_view(btn, LV_ANIM_OFF);
    }
  }
}

void MenuBody::select(int index)
{
  // adjust the selection based on separators
  for (int i = 0; i <= index; i++) {
    if (lines[i].isSeparator)
      index++;
    index = rangeCheck(index);
  }

  setIndex(index);
}

void MenuBody::selectNext(MENU_DIRECTION direction)
{
  // look for the next non separator line
  int index = selectedIndex + direction;
  index = rangeCheck(index);
  while (lines[index].isSeparator) {
    index += direction;
    index = rangeCheck(index);
  }

  setIndex(index);
}


#if defined(HARDWARE_KEYS)
void MenuBody::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_ROTARY_RIGHT) {
    if (!lines.empty()) {
      selectNext(DIRECTION_UP);
      onKeyPress();
    }
  }
  else if (event == EVT_ROTARY_LEFT) {
    if (!lines.empty()) {
      selectNext(DIRECTION_DOWN);
      onKeyPress();
    }
  }
  else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    if (!lines.empty()) {
      onKeyPress();
      if (selectedIndex < 0) {
        select(0);
      }
      else {
        Menu * menu = getParentMenu();
        if (menu->multiple) {
          lines[selectedIndex].onPress();
          menu->invalidate();
        }
        else {
          menu->deleteLater();
          lines[selectedIndex].onPress();
        }
      }
    }
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    onKeyPress();
    if (onCancel) {
      onCancel();
    }
    Window::onEvent(event);
  }
  else {
    Window::onEvent(event);
  }
}
#endif

MenuWindowContent::MenuWindowContent(Menu* parent) :
    ModalWindowContent(parent, {(LCD_W - MENUS_WIDTH) / 2,
                                (LCD_H - MENUS_WIDTH) / 2, MENUS_WIDTH, 0}),
    body(this, {0, 0, width(), height()})
{
  body.setFocus(SET_FOCUS_DEFAULT);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, LV_PART_MAIN);
}

Menu::Menu(Window * parent, bool multiple):
  ModalWindow(parent, true),
  content(createMenuWindow(this)),
  multiple(multiple)
{
}

void Menu::updatePosition()
{
  // calcualte the correct menu height given that the line heights are variable
  coord_t height = 0;
  for (int i = 0; i < (signed)content->body.lines.size(); i++)
    height += content->body.lines[i].lineHeight();

  if (!toolbar) {
    // there is no navigation bar at the left, we may center the window on screen
    auto headerHeight = content->title.empty() ? 0 : POPUP_HEADER_HEIGHT;
    auto bodyHeight = limit<coord_t>(MENUS_MIN_HEIGHT, height, MENUS_MAX_HEIGHT);
    content->setTop((LCD_H - headerHeight - bodyHeight) / 2 + MENUS_OFFSET_TOP);
    content->setHeight(headerHeight + bodyHeight);
    content->body.setTop(headerHeight);
    content->body.setHeight(bodyHeight);
  }

  content->body.setInnerHeight(height);
}

void Menu::setTitle(std::string text)
{
  content->setTitle(std::move(text));
  updatePosition();
}

void Menu::addLine(const std::string & text, std::function<void()> onPress, std::function<bool()> isChecked)
{
  content->body.addLine(text, std::move(onPress), std::move(isChecked));
  updatePosition();
}

void Menu::addSeparator()
{
  content->body.addSeparator();
  updatePosition();
}

void Menu::addCustomLine(std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine, std::function<void()> onPress, std::function<bool()> isChecked)
{
  content->body.addCustomLine(std::move(drawLine), std::move(onPress), std::move(isChecked));
  updatePosition();
}

void Menu::removeLines()
{
  content->body.removeLines();
  updatePosition();
}

#if defined(HARDWARE_KEYS)
void Menu::onEvent(event_t event)
{
  if (toolbar && (event == EVT_KEY_BREAK(KEY_PGDN) || event == EVT_KEY_LONG(KEY_PGDN) || 
      event == EVT_KEY_BREAK(KEY_PGUP))) {
    toolbar->onEvent(event);
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    deleteLater();
  }
  else if (event == EVT_KEY_BREAK(KEY_ENTER) && !multiple) {
    deleteLater();
  }
}
#endif

