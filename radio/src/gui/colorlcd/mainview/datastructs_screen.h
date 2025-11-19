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

#if !defined(BACKUP)
#include <vector>
#include <string>
#endif

#include "etx_lv_theme.h"

struct WidgetOption;

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

#define WIDGET_OPTION_VALUE_SIGNED(x)   WidgetOptionValue{ .signedValue = (x) }
#define WIDGET_OPTION_VALUE_STRING(...) WidgetOptionValue{ .stringValue = { __VA_ARGS__ } }

struct WidgetOptionValue
{
#if defined(YAML_GENERATOR)
  CUST_ATTR(unsignedValue, r_wov_unsigned, w_wov_unsigned);
  CUST_ATTR(signedValue, r_wov_signed, w_wov_signed);
  CUST_ATTR(boolValue, r_wov_unsigned, w_wov_unsigned);
  CUST_ATTR(stringValue, r_wov_string, w_wov_string);
  CUST_ATTR(source, r_wov_source, w_wov_source);
  CUST_ATTR(color, r_wov_color, w_wov_color);
#else
  union {
    uint32_t unsignedValue;
    int32_t signedValue;
    uint32_t boolValue;
  };
#if !defined(BACKUP)
  std::string stringValue;
#endif
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

//-----------------------------------------------------------------------------

#define MAX_WIDGET_OPTIONS 50   // For YAML parser

struct WidgetPersistentData {
#if defined(YAML_GENERATOR)
  WidgetOptionValueTyped options[MAX_WIDGET_OPTIONS] FUNC(widget_option_is_active);
#else
#if !defined(BACKUP)
  std::vector<WidgetOptionValueTyped> options;
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
  std::string getString(int idx);
  void setString(int idx, const char* s);
#endif
#endif
};

//-----------------------------------------------------------------------------

enum LayoutOptionValueEnum {
  LOV_None=0,
  LOV_Bool,
  LOV_Color
};

union LayoutOptionValue
{
#if defined(YAML_GENERATOR)
  CUST_ATTR(unsignedValue, r_lov_unsigned, w_lov_unsigned);
  CUST_ATTR(boolValue, r_lov_unsigned, w_lov_unsigned);
  CUST_ATTR(color, r_lov_color, w_lov_color);
#else
  uint32_t unsignedValue;
  uint32_t boolValue;
#endif
};

struct LayoutOptionValueTyped
{
#if defined(YAML_GENERATOR)
  CUST_ATTR(type, r_lov_type, w_lov_type);
#else
  LayoutOptionValueEnum type;
#endif
  LayoutOptionValue value FUNC(select_lov);
};

#define MAX_LAYOUT_ZONES 10
#define MAX_LAYOUT_OPTIONS 10

struct ZonePersistentData {
#if defined(YAML_GENERATOR)
  CUST_ATTR(widgetName, r_widget_name, w_widget_name);
#else
#if !defined(BACKUP)
  std::string widgetName;
#endif
#endif
  WidgetPersistentData widgetData FUNC(isAlwaysActive);
#if !defined(YAML_GENERATOR)
  void clear();
#endif
};

struct LayoutPersistentData {
  ZonePersistentData zones[MAX_LAYOUT_ZONES] FUNC(widget_is_active);
  LayoutOptionValueTyped options[MAX_LAYOUT_OPTIONS] FUNC(layout_option_is_active);
#if !defined(YAML_GENERATOR)
  void clearZone(int idx);
  void clear();
  const char* getWidgetName(int idx);
  void setWidgetName(int idx, const char* s);
  WidgetPersistentData* getWidgetData(int idx);
  bool hasWidget(int idx);
#endif
};

//-----------------------------------------------------------------------------

struct CustomScreenData {
#if defined(YAML_GENERATOR)
  CUST_ATTR(LayoutId, r_screen_id, w_screen_id);
#else
#if !defined(BACKUP)
  std::string LayoutId;
#endif
#endif
  LayoutPersistentData layoutData FUNC(isAlwaysActive);
};

//-----------------------------------------------------------------------------

static LAYOUT_VAL_SCALED(MENU_HEADER_BUTTONS_LEFT, 47)

#if LCD_W == 800  // TODO: handle this better
static constexpr int TOPBAR_ZONE_WIDTH = 102;
#else
static LAYOUT_VAL_SCALED(TOPBAR_ZONE_WIDTH, 70)
#endif
static constexpr int MAX_TOPBAR_ZONES = (LCD_W - MENU_HEADER_BUTTONS_LEFT - 1 + TOPBAR_ZONE_WIDTH / 2) / TOPBAR_ZONE_WIDTH;

struct TopBarPersistentData {
  ZonePersistentData zones[MAX_TOPBAR_ZONES] FUNC(widget_is_active);
#if !defined(YAML_GENERATOR)
  void clearZone(int idx);
  void clear();
  const char* getWidgetName(int idx);
  void setWidgetName(int idx, const char* s);
  WidgetPersistentData* getWidgetData(int idx);
  bool hasWidget(int idx);
  bool isWidget(int idx, const char* s);
#endif
};

//-----------------------------------------------------------------------------
