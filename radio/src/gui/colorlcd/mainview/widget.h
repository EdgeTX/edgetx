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
#if !defined(BACKUP)
#include <vector>
#endif

#include "button.h"
#include "widgets_container.h"
#include "storage/yaml/yaml_defs.h"
#include "datastructs_screen.h"

class WidgetFactory;

//-----------------------------------------------------------------------------

struct WidgetOption
{
  // First two entries must match luaScriptInputType enum
  // TODO: should be cleaned up
  enum Type {
    Integer,
    Source,
    Bool,
    String,
    TextSize,
    Timer,
    Switch,
    Color,
    Align,
    Slider,
    Choice,
    File,
  };

  const char * name;
  Type type;
  WidgetOptionValue deflt;
  WidgetOptionValue min;
  WidgetOptionValue max;
  const char * displayName;
  std::string fileSelectPath;
#if !defined(BACKUP)
  std::vector<std::string> choiceValues;
#endif
};

//-----------------------------------------------------------------------------

class Widget : public ButtonBase
{
 public:

  Widget(const WidgetFactory* factory, Window* parent, const rect_t& rect,
         int screenNum, int zoneNum);

  ~Widget() override = default;

  const WidgetFactory* getFactory() const { return factory; }

  const WidgetOption* getOptionDefinitions() const;
  bool hasOptions() const { return getOptionDefinitions() && getOptionDefinitions()->name; }

  virtual const char* getErrorMessage() const { return nullptr; }

  WidgetPersistentData* getPersistentData();

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
  bool isFullscreen() const { return fullscreen; }

  // Should rotary encoder events be enabled when full screen
  virtual bool enableFullScreenRE() const { return true; }

  // Called when the widget options have changed
  virtual void updateWithoutRefresh() {}
  virtual void update() {}

  // Called at regular time interval if the widget is hidden or off screen
  virtual void background() {}
  // Called at regular time interval if the widget is visible and on screen
  virtual void foreground() {}

  // Update widget 'zone' data (for Lua widgets)
  virtual void updateZoneRect(rect_t rect, bool updateUI = true)
  {
    if (updateUI) update();
  }

  void enableFocus(bool enable);

 protected:
  const WidgetFactory* factory;
  int screenNum;
  int zoneNum;
  bool fullscreen = false;
  bool closeFS = false;
  lv_obj_t* focusBorder = nullptr;
  lv_style_t borderStyle;
  lv_point_t borderPts[5];

  void onCancel() override;
  bool onLongPress() override;

  virtual void onFullscreen(bool enable) {}
  void openMenu();
};

//-----------------------------------------------------------------------------

class WidgetFactory
{
 public:
  explicit WidgetFactory(const char* name, const WidgetOption* options = nullptr,
                         const char* displayName = nullptr) :
      name(name), displayName(displayName), options(options)
  {
    registerWidget(this);
  }

  virtual ~WidgetFactory() {}

  const char* getName() const { return name; }

  const WidgetOption* getDefaultOptions() const { return options; }
  virtual const void parseOptionDefaults() const {}
  virtual const void checkOptions(int screenNum, int zoneNum) const {}

  const char* getDisplayName() const
  {
    return displayName ? displayName : name;
  }

  Widget* create(Window* parent, const rect_t& rect,
                         int screenNum, int zoneNum,
                         bool init = true) const;

  virtual Widget* createNew(Window* parent, const rect_t& rect, int screenNum, int zoneNum) const = 0;

  virtual bool isLuaWidgetFactory() const { return false; }

  static std::list<const WidgetFactory*>& getRegisteredWidgets();
  static void registerWidget(const WidgetFactory* factory);
  static void unregisterWidget(const WidgetFactory* factory);
  static const WidgetFactory* getWidgetFactory(const char* name);
  static Widget* newWidget(const char* name, Window* parent, const rect_t& rect,
                           int screenNum, int zoneNum);

 protected:
  const char* name = nullptr;
  const char* displayName = nullptr;
  const WidgetOption* options = nullptr;
};

//-----------------------------------------------------------------------------

template <class T>
class BaseWidgetFactory : public WidgetFactory
{
 public:
  BaseWidgetFactory(const char* name, const WidgetOption* options,
                    const char* displayName = nullptr) :
      WidgetFactory(name, options, displayName)
  {
  }

  Widget* createNew(Window* parent, const rect_t& rect,
                 int screenNum, int zoneNum) const override
  {
    return new T(this, parent, rect, screenNum, zoneNum);
  }
};

//-----------------------------------------------------------------------------
