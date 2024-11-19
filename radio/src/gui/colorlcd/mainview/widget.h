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

#pragma once

#include <string.h>

#include <list>

#include "button.h"
#include "widgets_container.h"

class WidgetFactory;

class Widget : public ButtonBase
{
 public:
  typedef WidgetPersistentData PersistentData;

  Widget(const WidgetFactory* factory, Window* parent, const rect_t& rect,
         WidgetPersistentData* persistentData);

  ~Widget() override = default;

  const WidgetFactory* getFactory() const { return factory; }

  const ZoneOption* getOptions() const;

  virtual const char* getErrorMessage() const { return nullptr; }

  ZoneOptionValue* getOptionValue(unsigned int index) const
  {
    return &persistentData->options[index].value;
  }

  PersistentData* getPersistentData() { return persistentData; }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "Widget"; }
#endif

  // Window interface
#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override;
#endif

  // Widget interface

  // Set/unset fullscreen mode
  void setFullscreen(bool enable);
  void closeFullscreen() { closeFS = true; }

  // Called when the widget options have changed
  virtual void update();

  // Called at regular time interval, even if the widget cannot be seen
  virtual void background() {}

  // Update widget 'zone' data (for Lua widgets)
  virtual void updateZoneRect(rect_t rect, bool updateUI = true)
  {
    if (updateUI) update();
  }

  void enableFocus(bool enable);

 protected:
  const WidgetFactory* factory;
  PersistentData* persistentData;
  bool fullscreen = false;
  bool fsAllowed = true;
  bool closeFS = false;
  lv_obj_t* focusBorder = nullptr;
  lv_style_t borderStyle;
  lv_point_t borderPts[5];

  void onCancel() override;
  bool onLongPress() override;

  virtual void onFullscreen(bool enable) {}
  void openMenu();
};

class WidgetFactory
{
 public:
  explicit WidgetFactory(const char* name, const ZoneOption* options = nullptr,
                         const char* displayName = nullptr) :
      name(name), displayName(displayName), options(options)
  {
    registerWidget(this);
  }

  virtual ~WidgetFactory() {}

  const char* getName() const { return name; }

  const ZoneOption* getOptions() const { return options; }

  const char* getDisplayName() const
  {
    return displayName ? displayName : name;
  }

  void initPersistentData(Widget::PersistentData* persistentData,
                          bool setDefault) const;

  virtual Widget* create(Window* parent, const rect_t& rect,
                         Widget::PersistentData* persistentData,
                         bool init = true) const = 0;

  virtual bool isLuaWidgetFactory() const { return false; }

  static std::list<const WidgetFactory*>& getRegisteredWidgets();
  static void registerWidget(const WidgetFactory* factory);
  static void unregisterWidget(const WidgetFactory* factory);
  static const WidgetFactory* getWidgetFactory(const char* name);
  static Widget* newWidget(const char* name, Window* parent, const rect_t& rect,
                           Widget::PersistentData* persistentData);

 protected:
  const char* name;
  const char* displayName;
  const ZoneOption* options;
};

template <class T>
class BaseWidgetFactory : public WidgetFactory
{
 public:
  BaseWidgetFactory(const char* name, const ZoneOption* options,
                    const char* displayName = nullptr) :
      WidgetFactory(name, options, displayName)
  {
  }

  Widget* create(Window* parent, const rect_t& rect,
                 Widget::PersistentData* persistentData,
                 bool init = true) const override
  {
    initPersistentData(persistentData, init);
    return new T(this, parent, rect, persistentData);
  }
};
