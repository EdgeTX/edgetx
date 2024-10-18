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

#include <inttypes.h>
#include "storage/yaml/yaml_defs.h"
#include <vector>

#define LEN_ZONE_OPTION_STRING 12

#if defined(_MSC_VER)
  #define OPTION_VALUE_UNSIGNED(x)    { uint32_t(x) }
  #define OPTION_VALUE_SIGNED(x)      { int32_t(x) }
  #define OPTION_VALUE_BOOL(x)        {  bool(x) }
  #define OPTION_VALUE_STRING(...)    { *(ZoneOptionValue *)(const char *) #__VA_ARGS__ }
#else
  #define OPTION_VALUE_UNSIGNED(x) ZoneOptionValue{ .unsignedValue = (x) }
  #define OPTION_VALUE_SIGNED(x)   ZoneOptionValue{ .signedValue = (x) }
  #define OPTION_VALUE_BOOL(x)     ZoneOptionValue{ .boolValue = (x) }
  #define OPTION_VALUE_STRING(...) ZoneOptionValue{ .stringValue = { __VA_ARGS__ } }
#endif

union ZoneOptionValue
{
  uint32_t unsignedValue;
  int32_t signedValue;
  uint32_t boolValue;
  char stringValue[LEN_ZONE_OPTION_STRING];
  CUST_ATTR(source, r_zov_source, w_zov_source);
  CUST_ATTR(color, r_zov_color, w_zov_color);
};

enum ZoneOptionValueEnum {
  ZOV_Unsigned=0,
  ZOV_Signed,
  ZOV_Bool,
  ZOV_String,
  ZOV_Source,
  ZOV_Color
};

struct ZoneOption
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
  ZoneOptionValue deflt;
  ZoneOptionValue min;
  ZoneOptionValue max;
  const char * displayName;
  std::string fileSelectPath;
  std::vector<std::string> choiceValues;
};

enum ZoneOptionAlign
{
  ALIGN_LEFT,
  ALIGN_CENTER,
  ALIGN_RIGHT,

  // this one MUST be last
  ALIGN_COUNT
};

struct ZoneOptionValueTyped
{
  ZoneOptionValueEnum type;
  ZoneOptionValue     value FUNC(select_zov);
};

inline ZoneOptionValueEnum zoneValueEnumFromType(ZoneOption::Type type)
{
  switch(type) {
  case ZoneOption::String:
  case ZoneOption::File:
    return ZOV_String;

  case ZoneOption::Integer:
    return ZOV_Signed;

  case ZoneOption::Bool:
    return ZOV_Bool;

  case ZoneOption::Source:
    return ZOV_Source;

  case ZoneOption::Color:
    return ZOV_Color;
    
  default:
    return ZOV_Unsigned;
  }
}
