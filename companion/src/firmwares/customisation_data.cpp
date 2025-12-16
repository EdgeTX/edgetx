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

#include "customisation_data.h"

// cannot use QColor so use formula from libopenui_defines.h
#define RADIO_RGB(r, g, b) \
  (uint16_t)((((r)&0xF8) << 8) + (((g)&0xFC) << 3) + (((b)&0xF8) >> 3))
#define WHITE RADIO_RGB(0xFF, 0xFF, 0xFF)
#define RED RADIO_RGB(229, 32, 30)

ZoneOptionValue::ZoneOptionValue()
{
  clear();
}

ZoneOptionValue::ZoneOptionValue(const ZoneOptionValue & src)
{
  copy(src);
}

ZoneOptionValue & ZoneOptionValue::operator=(const ZoneOptionValue & src)
{
  copy(src);
  return *this;
}

void ZoneOptionValue::copy(const ZoneOptionValue & src)
{
  unsignedValue = src.unsignedValue;
  signedValue = src.signedValue;
  boolValue = src.boolValue;
  stringValue = src.stringValue;
  sourceValue = src.sourceValue;
  colorValue = src.colorValue;
}

bool ZoneOptionValue::isEmpty() const
{
  return unsignedValue == 0 && signedValue == 0 && boolValue == 0 && colorValue == 0 &&
         stringValue.empty() && sourceValue.toValue() == 0;
}

void ZoneOptionValue::clear()
{
  unsignedValue = 0;
  signedValue = 0;
  boolValue = 0;
  stringValue.clear();
  sourceValue.clear();
  colorValue = 0;
}

inline void setZoneOptionValue(ZoneOptionValue& zov, bool value)
{
  zov.boolValue = value;
}

inline void setZoneOptionValue(ZoneOptionValue& zov, int value)
{
  zov.signedValue = value;
}

inline void setZoneOptionValue(ZoneOptionValue& zov, const char* value)
{
  zov.stringValue = value;
}

inline void setZoneOptionValue(ZoneOptionValue& zov, unsigned int value)
{
  zov.unsignedValue = value;
}

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

  case ZoneOption::Color:
    return ZOV_Color;

  case ZoneOption::Source:
    return ZOV_Source;

  default:
    return ZOV_Unsigned;
  }
}

inline const char * zoneOptionValueEnumToString(ZoneOptionValueEnum zovenum) {
  switch (zovenum) {
    case ZOV_Unsigned:
      return "unsigned";
    case ZOV_Signed:
      return "signed";
    case ZOV_Bool:
      return "bool";
    case ZOV_String:
      return "string";
    default:
      return "unknown";
  }
}

ZoneOptionValueTyped::ZoneOptionValueTyped()
{
  clear();
}

bool ZoneOptionValueTyped::isEmpty() const
{
  return type == ZOV_Unsigned && value.isEmpty();
}

void ZoneOptionValueTyped::clear()
{
  type = ZOV_Unsigned;
  value.clear();
}

void WidgetPersistentData::clear()
{
  for (int i = 0; i < MAX_WIDGET_OPTIONS; i += 1)
    options[i].clear();
}

ZonePersistentData::ZonePersistentData(const ZonePersistentData & src)
{
  copy(src);
}

ZonePersistentData & ZonePersistentData::operator=(const ZonePersistentData & src)
{
  copy(src);
  return *this;
}

void ZonePersistentData::copy(const ZonePersistentData & src)
{
  widgetName = src.widgetName;
  widgetData = src.widgetData;
}

bool ZonePersistentData::isEmpty() const
{
  return widgetName.empty();
}

void ZonePersistentData::clear()
{
  widgetName.clear();
  widgetData.clear();
}

RadioLayout::CustomScreenData::CustomScreenData(const RadioLayout::CustomScreenData & src)
{
  copy(src);
}

RadioLayout::CustomScreenData & RadioLayout::CustomScreenData::operator=(const RadioLayout::CustomScreenData & src)
{
  copy(src);
  return *this;
}

void RadioLayout::CustomScreenData::copy(const RadioLayout::CustomScreenData & src)
{
  layoutId = src.layoutId;
  layoutPersistentData = src.layoutPersistentData;
}

bool RadioLayout::CustomScreenData::isEmpty() const
{
  return layoutId.empty();
}

void RadioLayout::CustomScreenData::clear()
{
  layoutId.clear();
  layoutPersistentData.clear();
}

RadioLayout::CustomScreens::CustomScreens(const RadioLayout::CustomScreens & src)
{
  copy(src);
}

RadioLayout::CustomScreens & RadioLayout::CustomScreens::operator=(const RadioLayout::CustomScreens & src)
{
  copy(src);
  return *this;
}

void RadioLayout::CustomScreens::copy(const RadioLayout::CustomScreens & src)
{
  for (int i = 0; i < MAX_CUSTOM_SCREENS; i++)
    customScreenData[i] = src.customScreenData[i];
}

void RadioLayout::CustomScreens::clear()
{
  for (int i = 0; i < MAX_CUSTOM_SCREENS; i++)
    customScreenData[i].clear();
}

void RadioLayout::init(const std::string layoutId, CustomScreens& customScreens)
{
  customScreens.clear();

  for (int i = 0; i < MAX_CUSTOM_SCREENS; i++) {
    if (i == 0)
      customScreens.customScreenData[i].layoutId = layoutId;

    LayoutPersistentData& persistentData =
        customScreens.customScreenData[i].layoutPersistentData;

    int j = 0;
    persistentData.options[j].type =
        zoneValueEnumFromType(ZoneOption::Type::Bool);
    setZoneOptionValue(persistentData.options[j++].value, (bool)true);

    persistentData.options[j].type =
        zoneValueEnumFromType(ZoneOption::Type::Bool);
    setZoneOptionValue(persistentData.options[j++].value, (bool)true);

    persistentData.options[j].type =
        zoneValueEnumFromType(ZoneOption::Type::Bool);
    setZoneOptionValue(persistentData.options[j++].value, (bool)true);

    persistentData.options[j].type =
        zoneValueEnumFromType(ZoneOption::Type::Bool);
    setZoneOptionValue(persistentData.options[j++].value, (bool)true);

    persistentData.options[j].type =
        zoneValueEnumFromType(ZoneOption::Type::Bool);
    setZoneOptionValue(persistentData.options[j++].value, (bool)false);
  }
}
