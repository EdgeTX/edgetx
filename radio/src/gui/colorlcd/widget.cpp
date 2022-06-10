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

#include "opentx.h"
#include "widget.h"
#include "menu.h"
#include "widget_settings.h"
#include "view_main.h"
#include "lcd.h"

#if defined(HARDWARE_TOUCH)
#include "touch.h"
#endif

static void openWidgetMenu(Widget * parent)
{
  Menu *menu = new Menu(parent);
  menu->addLine("Full screen", [=]() { parent->setFullscreen(true); });
  if (parent->getOptions() && parent->getOptions()->name)
    menu->addLine(TR_WIDGET_SETTINGS,
                  [=]() { new WidgetSettings(parent, parent); });
}

Widget::Widget(const WidgetFactory* factory, Window* parent,
               const rect_t &rect, WidgetPersistentData* persistentData) :
    Button(parent, rect, nullptr, 0, 0, window_create),
    factory(factory),
    persistentData(persistentData)
{
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

  setPressHandler([&]() -> uint8_t {
    // When ViewMain is in "widget select mode",
    // the widget is added to a focus group
    if (!fullscreen && lv_obj_get_group(lvobj))
      openWidgetMenu(this);
    return 0;
  });

  setLongPressHandler([&]() -> uint8_t {
    if (!fullscreen) openWidgetMenu(this);
    return 0;
  });
}

void Widget::paint(BitmapBuffer * dc)
{
  if (fullscreen) {
    // Draw background screen bellow
    OpenTxTheme::instance()->drawBackground(dc);
  }

  // refresh the widget
  refresh(dc);
  
  if (hasFocus() && !fullscreen) {
    dc->drawRect(0, 0, width(), height(), 2, STASHED, COLOR_THEME_FOCUS);
  }
}

#if defined(HARDWARE_KEYS)
void Widget::onEvent(event_t event)
{
  if (fullscreen && (EVT_KEY_LONG(KEY_EXIT) == event)) {
    setFullscreen(false);
  }
}
#endif

void Widget::update()
{
  auto container = dynamic_cast<WidgetsContainer*>(parent);
  if (container) {
    container->updateZones();
  }
}

void Widget::setFullscreen(bool enable)
{
  if (enable == fullscreen) return;

  // Leave Fullscreen Mode
  if (!enable) {

    // Reset all zones in container
    Widget::update();
    setWindowFlags(getWindowFlags() & ~OPAQUE);
    lv_obj_set_style_bg_opa(lvobj, LV_OPA_0, LV_PART_MAIN);

    // and give up focus
    ViewMain::instance()->enableTopbar();
    fullscreen = false;

    lv_group_remove_obj(lvobj);

    // re-enable scroll chaining (sliding main view)
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_SCROLL_CHAIN_VER);

    // exit editing mode
    lv_group_set_editing(lv_group_get_default(), false);

    onFullscreen(enable);
  }
  // Enter Fullscreen Mode
  else {

    // Set window opaque (inhibits redraw from windows bellow)
    setWindowFlags(getWindowFlags() | OPAQUE);
    lv_obj_set_style_bg_opa(lvobj, LV_OPA_MAX, LV_PART_MAIN);
    setRect(parent->getRect());
    fullscreen = true;
    ViewMain::instance()->disableTopbar();
    bringToTop();

    if (!lv_obj_get_group(lvobj)) {
      lv_group_add_obj(lv_group_get_default(), lvobj);
    }

    // disable scroll chaining (sliding main view)
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLL_CHAIN_VER);

    // set group in editing mode (keys LEFT / RIGHT)
    lv_group_set_editing(lv_group_get_default(), true);

    onFullscreen(enable);
  }
}

void Widget::onLongPress()
{
  if (!fullscreen) Button::onLongPress();
}

std::list<const WidgetFactory *> & getRegisteredWidgets()
{
  static std::list<const WidgetFactory *> widgets;
  return widgets;
}

void registerWidget(const WidgetFactory * factory)
{
  TRACE("register widget %s", factory->getName());
  getRegisteredWidgets().push_back(factory);
}

void unregisterWidget(const WidgetFactory * factory)
{
  TRACE("unregister widget %s", factory->getName());
  getRegisteredWidgets().remove(factory);
}

const WidgetFactory * getWidgetFactory(const char * name)
{
  auto it = getRegisteredWidgets().cbegin();
  for (; it != getRegisteredWidgets().cend();++it) {
    if (!strcmp(name, (*it)->getName())) {
      return (*it);
    }
  }
  return nullptr;
}

Widget* loadWidget(const char* name, Window* parent, const rect_t& rect,
                   WidgetPersistentData* persistentData)
{
  const WidgetFactory* factory = getWidgetFactory(name);
  if (factory) {
    return factory->create(parent, rect, persistentData, false);
  }
  return nullptr;
}
