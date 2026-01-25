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

#include "widget.h"

#include "edgetx.h"
#include "etx_lv_theme.h"
#include "menu.h"
#include "view_main.h"
#include "widget_settings.h"

#if defined(HARDWARE_TOUCH)
#include "touch.h"
#endif

//-----------------------------------------------------------------------------

inline WidgetOptionValueEnum widgetValueEnumFromType(WidgetOption::Type type)
{
  switch(type) {
  case WidgetOption::String:
  case WidgetOption::File:
    return WOV_String;

  case WidgetOption::Integer:
    return WOV_Signed;

  case WidgetOption::Bool:
    return WOV_Bool;

  case WidgetOption::Source:
    return WOV_Source;

  case WidgetOption::Color:
    return WOV_Color;

  default:
    return WOV_Unsigned;
  }
}

void WidgetPersistentData::addEntry(int idx)
{
  if (idx >= (int)options.size()) {
    WidgetOptionValueTyped wov;
    wov.type = WOV_Unsigned;
    wov.value.unsignedValue = 0;
    while ((int)options.size() <= idx)
      options.push_back(wov);
  }
}

bool WidgetPersistentData::hasOption(int idx)
{
  return idx < (int)options.size();
}

void WidgetPersistentData::setDefault(int idx, const WidgetOption* opt, bool forced)
{
  addEntry(idx);
  auto optType = widgetValueEnumFromType(opt->type);
  if (forced || options[idx].type != optType) {
    // reset to default value
    options[idx].type = optType;
    options[idx].value.unsignedValue = opt->deflt.unsignedValue;
    options[idx].value.stringValue = opt->deflt.stringValue;
  }
}

void WidgetPersistentData::clear()
{
  options.clear();
}

WidgetOptionValueEnum WidgetPersistentData::getType(int idx)
{
  addEntry(idx);
  return options[idx].type;
}

void WidgetPersistentData::setType(int idx, WidgetOptionValueEnum typ)
{
  addEntry(idx);
  options[idx].type = typ;
}

int32_t WidgetPersistentData::getSignedValue(int idx)
{
  addEntry(idx);
  return options[idx].value.signedValue;
}

void WidgetPersistentData::setSignedValue(int idx, int32_t newValue)
{
  addEntry(idx);
  options[idx].value.signedValue = newValue;
}

uint32_t WidgetPersistentData::getUnsignedValue(int idx)
{
  addEntry(idx);
  return options[idx].value.unsignedValue;
}

void WidgetPersistentData::setUnsignedValue(int idx, uint32_t newValue)
{
  addEntry(idx);
  options[idx].value.unsignedValue = newValue;
}

bool WidgetPersistentData::getBoolValue(int idx)
{
  addEntry(idx);
  return options[idx].value.boolValue;
}

void WidgetPersistentData::setBoolValue(int idx, bool newValue)
{
  addEntry(idx);
  options[idx].value.boolValue = newValue;
}

std::string WidgetPersistentData::getString(int idx)
{
  addEntry(idx);
  return options[idx].value.stringValue;
}

void WidgetPersistentData::setString(int idx, const char* s)
{
  addEntry(idx);
  options[idx].value.stringValue = s;
}

//-----------------------------------------------------------------------------

Widget::Widget(const WidgetFactory* factory, Window* parent, const rect_t& rect,
               int screenNum, int zoneNum) :
    ButtonBase(parent, rect, nullptr, window_create),
    factory(factory),
    screenNum(screenNum), zoneNum(zoneNum)
{
  setWindowFlag(NO_FOCUS | NO_SCROLL);

  if (parent->isTopBar()) fsAllowed = false;

  setPressHandler([&]() -> uint8_t {
    // When ViewMain is in "widget select mode",
    // the widget is added to a focus group
    if (!fullscreen && lv_obj_get_group(lvobj)) openMenu();
    return 0;
  });
}

void Widget::openMenu()
{
  if (fsAllowed && ViewMain::instance()->isAppMode())
  {
    setFullscreen(true);
    return;
  }

  if (hasOptions() || fsAllowed) {
    Menu* menu = new Menu();
    menu->setTitle(getFactory()->getDisplayName());
    if (fsAllowed) {
      menu->addLine(STR_WIDGET_FULLSCREEN, [&]() { setFullscreen(true); });
    }
    if (hasOptions()) {
      menu->addLine(STR_WIDGET_SETTINGS,
                    [=]() { new WidgetSettings(this); });
    }
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

void Widget::onCancel()
{
  if (!fullscreen) ButtonBase::onCancel();
}

void Widget::update() {}

void Widget::setFullscreen(bool enable)
{
  if (!fsAllowed || (enable == fullscreen)) return;

  fullscreen = enable;

  // Show or hide ViewMain widgets and decorations
  ViewMain::instance()->show(!enable);

  // Leave Fullscreen Mode
  if (!enable) {
    clearWindowFlag(OPAQUE);

    lv_group_remove_obj(lvobj);

    // re-enable scroll chaining (sliding main view)
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_add_flag(lvobj, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
  }
  // Enter Fullscreen Mode
  else {
    ViewMain::instance()->enableWidgetSelect(false);

    // ViewMain hidden - re-show this widget
    show();

    // Set window opaque (inhibits redraw from windows below)
    setWindowFlag(OPAQUE);

    updateZoneRect(parent->getRect(), false);
    setRect(parent->getRect());

    if (!lv_obj_get_group(lvobj))
      lv_group_add_obj(lv_group_get_default(), lvobj);

    // disable scroll chaining (sliding main view)
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLL_CHAIN_HOR);
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLL_CHAIN_VER);
  }

  // set group in editing mode (keys LEFT / RIGHT)
  if (enableFullScreenRE())
    lv_group_set_editing(lv_group_get_default(), enable);

  onFullscreen(enable);

  update();
}

bool Widget::onLongPress()
{
  if (!fullscreen) {
    openMenu();
    return false;
  }
  return true;
}

const WidgetOption* Widget::getOptionDefinitions() const
{
  return getFactory()->getDefaultOptions();
}

void Widget::enableFocus(bool enable)
{
  if (enable) {
    if (!focusBorder) {
      lv_style_init(&borderStyle);
      lv_style_set_line_width(&borderStyle, PAD_BORDER);
      lv_style_set_line_opa(&borderStyle, LV_OPA_COVER);
      lv_style_set_line_color(&borderStyle, makeLvColor(COLOR_THEME_FOCUS));

      borderPts[0] = {1, 1};
      borderPts[1] = {(lv_coord_t)(width() - 1), 1};
      borderPts[2] = {(lv_coord_t)(width() - 1), (lv_coord_t)(height() - 1)};
      borderPts[3] = {1, (lv_coord_t)(height() - 1)};
      borderPts[4] = {1, 1};

      focusBorder = lv_line_create(lvobj);
      lv_obj_add_style(focusBorder, &borderStyle, LV_PART_MAIN);
      lv_line_set_points(focusBorder, borderPts, 5);

      if (!hasFocus()) {
        lv_obj_add_flag(focusBorder, LV_OBJ_FLAG_HIDDEN);
      }

      setFocusHandler([=](bool hasFocus) {
        if (hasFocus) {
          lv_obj_clear_flag(focusBorder, LV_OBJ_FLAG_HIDDEN);
        } else {
          lv_obj_add_flag(focusBorder, LV_OBJ_FLAG_HIDDEN);
        }
        ViewMain::instance()->refreshWidgetSelectTimer();
      });

      lv_group_add_obj(lv_group_get_default(), lvobj);
    }
  } else {
    if (focusBorder) {
      lv_obj_del(focusBorder);
      setFocusHandler(nullptr);
      lv_group_remove_obj(lvobj);
    }
    focusBorder = nullptr;
  }
}

WidgetPersistentData* Widget::getPersistentData() { return g_model.getWidgetData(screenNum, zoneNum); }

//-----------------------------------------------------------------------------

std::list<const WidgetFactory*>& WidgetFactory::getRegisteredWidgets()
{
  static std::list<const WidgetFactory*> widgets;
  return widgets;
}

void WidgetFactory::unregisterWidget(const WidgetFactory* factory)
{
  getRegisteredWidgets().remove(factory);
}

const WidgetFactory* WidgetFactory::getWidgetFactory(const char* name)
{
  auto it = getRegisteredWidgets().cbegin();
  for (; it != getRegisteredWidgets().cend(); ++it) {
    if (!strcmp(name, (*it)->getName())) {
      return (*it);
    }
  }
  return nullptr;
}

void WidgetFactory::registerWidget(const WidgetFactory* factory)
{
  auto name = factory->getName();
  auto oldWidget = getWidgetFactory(name);
  if (oldWidget) {
    unregisterWidget(oldWidget);
  }
  for (auto it = getRegisteredWidgets().cbegin();
       it != getRegisteredWidgets().cend(); ++it) {
    if (strcasecmp((*it)->getDisplayName(), factory->getDisplayName()) > 0) {
      getRegisteredWidgets().insert(it, factory);
      return;
    }
  }
  getRegisteredWidgets().push_back(factory);
}

Widget* WidgetFactory::newWidget(const char* name, Window* parent,
                                 const rect_t& rect, int screenNum, int zoneNum)
{
  const WidgetFactory* factory = getWidgetFactory(name);
  if (factory) {
    return factory->create(parent, rect, screenNum, zoneNum, false);
  }
  return nullptr;
}

Widget* WidgetFactory::create(Window* parent, const rect_t& rect,
                int screenNum, int zoneNum,
                bool init) const
{
  auto widgetData = g_model.getWidgetData(screenNum, zoneNum);

  if (init) {
    widgetData->clear();
    parseOptionDefaults();
  }
  if (options) {
    int i = 0;
    for (const WidgetOption* option = options; option->name; option++, i++) {
      TRACE("WidgetFactory::create() setting option '%s'", option->name);
      widgetData->setDefault(i, option, init);
    }
  }

  return createNew(parent, rect, screenNum, zoneNum);
}
