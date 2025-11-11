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
#include <vector>

#include "button.h"
#include "widgets_container.h"
#include "storage/yaml/yaml_defs.h"

class WidgetFactory;

//-----------------------------------------------------------------------------

enum WidgetOptionAlign
{
  ALIGN_LEFT,
  ALIGN_CENTER,
  ALIGN_RIGHT,

  // this one MUST be last
  ALIGN_COUNT
};

enum WidgetOptionValueEnum {
  WOV_Unsigned=0,
  WOV_Signed,
  WOV_Bool,
  WOV_String,
  WOV_Source,
  WOV_Color
};

#define LEN_ZONE_OPTION_STRING 12

#define WIDGET_OPTION_VALUE_SIGNED(x)   WidgetOptionValue{ .signedValue = (x) }
#define WIDGET_OPTION_VALUE_STRING(...) WidgetOptionValue{ .stringValue = { __VA_ARGS__ } }

union WidgetOptionValue
{
#if defined(YAML_GENERATOR)
  CUST_ATTR(unsignedValue, r_wov_unsigned, w_wov_unsigned);
  CUST_ATTR(signedValue, r_wov_signed, w_wov_signed);
  CUST_ATTR(boolValue, r_wov_unsigned, w_wov_unsigned);
  CUST_ATTR(stringValue, r_wov_string, w_wov_string);
  CUST_ATTR(source, r_wov_source, w_wov_source);
  CUST_ATTR(color, r_wov_color, w_wov_color);
#else
  uint32_t unsignedValue;
  int32_t signedValue;
  uint32_t boolValue;
  char stringValue[LEN_ZONE_OPTION_STRING];
#endif
};

struct WidgetOptionValueTyped
{
#if defined(YAML_GENERATOR)
  CUST_ATTR(type, r_wov_type, w_wov_type);
#else
  WidgetOptionValueEnum type;
#endif
  WidgetOptionValue value FUNC(select_wov);
};

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
  std::vector<std::string> choiceValues;
};

//-----------------------------------------------------------------------------

#define MAX_WIDGET_OPTIONS 50   // For YAML parser

struct WidgetPersistentData {
#if defined(YAML_GENERATOR)
  WidgetOptionValueTyped options[MAX_WIDGET_OPTIONS] FUNC(widget_option_is_active);
#else
#if !defined(BACKUP)
  std::vector<WidgetOptionValueTyped> options;
#endif
  void addEntry(int idx);
  bool hasOption(int idx);
  void setDefault(int idx, const WidgetOption* opt, bool forced);
  void clear();
  WidgetOptionValueEnum getType(int idx);
  void setType(int idx, WidgetOptionValueEnum typ);
  int32_t getSignedValue(int idx);
  void setSignedValue(int idx, int32_t newValue);
  uint32_t getUnsignedValue(int idx);
  void setUnsignedValue(int idx, uint32_t newValue);
  bool getBoolValue(int idx);
  void setBoolValue(int idx, bool newValue);
  const char* getString(int idx);
  void setString(int idx, const char* s);
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
  int screenNum;
  int zoneNum;
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
