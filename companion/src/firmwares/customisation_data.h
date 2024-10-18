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
#include "constants.h"
#include "rawsource.h"

#include <QtCore>

/*

  Note: this code is consolidated from various radio/src files and modified if necessary to run on desktop

*/

constexpr int MAX_CUSTOM_SCREENS      {10};
constexpr int MAX_THEME_OPTIONS       {5};
constexpr int LEN_ZONE_OPTION_STRING  {8};
constexpr int MAX_LAYOUT_ZONES        {10};
constexpr int MAX_LAYOUT_OPTIONS      {10};
constexpr int WIDGET_NAME_LEN         {12};
constexpr int MAX_WIDGET_OPTIONS      {5};
constexpr int MAX_TOPBAR_ZONES        {6};  //  max 4 used for portrait
constexpr int MAX_TOPBAR_OPTIONS      {1};
constexpr int LAYOUT_ID_LEN           {12};

// Common 'ZoneOptionValue's among all layouts
enum {
  LAYOUT_OPTION_TOPBAR = 0,
  LAYOUT_OPTION_FM,
  LAYOUT_OPTION_SLIDERS,
  LAYOUT_OPTION_TRIMS,
  LAYOUT_OPTION_MIRRORED,

  LAYOUT_OPTION_LAST_DEFAULT=LAYOUT_OPTION_MIRRORED
};

struct ZoneOptionValue  // union in radio/src/datastructs.h
{
  unsigned int unsignedValue;
  int signedValue;
  unsigned int boolValue;
  char stringValue[LEN_ZONE_OPTION_STRING + 1];
  RawSource sourceValue;
  unsigned int colorValue;

  ZoneOptionValue();
};

enum ZoneOptionValueEnum {
  ZOV_Unsigned,
  ZOV_Signed,
  ZOV_Bool,
  ZOV_String,
  ZOV_Source,
  ZOV_Color,
  ZOV_LAST = ZOV_Color
};

enum ZoneOptionAlign
{
  ALIGN_LEFT,
  ALIGN_CENTER,
  ALIGN_RIGHT,
  ALIGN_COUNT
};

struct ZoneOption
{
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
};

struct ZoneOptionValueTyped
{
  ZoneOptionValueEnum type;
  ZoneOptionValue     value;

  ZoneOptionValueTyped();
  bool isEmpty() const;
};

struct WidgetPersistentData {
  ZoneOptionValueTyped options[MAX_WIDGET_OPTIONS];

  WidgetPersistentData();
};

struct ZonePersistentData {
  char widgetName[WIDGET_NAME_LEN + 1];
  WidgetPersistentData widgetData;

  ZonePersistentData();
  bool isEmpty() const;
};

template<int N, int O>
struct WidgetsContainerPersistentData {
  ZonePersistentData   zones[N];
  ZoneOptionValueTyped options[O];

  WidgetsContainerPersistentData() {
    memset((void*)this, 0, sizeof(WidgetsContainerPersistentData));
  }
};

typedef WidgetsContainerPersistentData<MAX_LAYOUT_ZONES, MAX_LAYOUT_OPTIONS>
    LayoutPersistentData;

typedef WidgetsContainerPersistentData<MAX_TOPBAR_ZONES, MAX_TOPBAR_OPTIONS>
    TopBarPersistentData;

class RadioLayout
{
  Q_DECLARE_TR_FUNCTIONS(RadioLayout)

  public:
    struct CustomScreenData {
      char layoutId[LAYOUT_ID_LEN + 1];
      LayoutPersistentData layoutPersistentData;

      CustomScreenData();
      bool isEmpty() const;
    };

    struct CustomScreens {
      CustomScreenData customScreenData[MAX_CUSTOM_SCREENS];

      void clear();
    };

    static void init(const char * layoutId, CustomScreens & customScreens);
};
