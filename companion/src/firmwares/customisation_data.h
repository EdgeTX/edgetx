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
constexpr int MAX_LAYOUT_ZONES        {10};
constexpr int MAX_LAYOUT_OPTIONS      {10};
constexpr int MAX_WIDGET_OPTIONS      {50};
constexpr int MAX_TOPBAR_ZONES        {7};    // 4 for portrait LCD, 6 for standard LCD, 7 for wide screen LCD
constexpr int MAX_TOPBAR_OPTIONS      {1};

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
  std::string stringValue;
  RawSource sourceValue;
  unsigned int colorValue;

  ZoneOptionValue();
  ZoneOptionValue(const ZoneOptionValue & src);
  ZoneOptionValue & operator=(const ZoneOptionValue & src);

  void clear();
  void copy(const ZoneOptionValue & src);
  bool isEmpty() const;
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
  void clear();
  bool isEmpty() const;
};

struct WidgetPersistentData {
  ZoneOptionValueTyped options[MAX_WIDGET_OPTIONS];

  WidgetPersistentData() {}
  void clear();
};

struct ZonePersistentData {
  std::string widgetName;
  WidgetPersistentData widgetData;

  ZonePersistentData() {}
  ZonePersistentData(const ZonePersistentData & src);
  ZonePersistentData & operator=(const ZonePersistentData & src);
  void clear();
  void copy(const ZonePersistentData & src);
  bool isEmpty() const;
};

template<int N, int O>
struct WidgetsContainerPersistentData {
  ZonePersistentData   zones[N];
  ZoneOptionValueTyped options[O];

  WidgetsContainerPersistentData() {}
  void clear() {
    for (int i = 0; i < N; i++) {
      zones[i].clear();
    }
    for (int i = 0; i < O; i++) {
      options[i].clear();
    }
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
      std::string layoutId;
      LayoutPersistentData layoutPersistentData;

      CustomScreenData() {}
      CustomScreenData(const CustomScreenData & src);
      CustomScreenData & operator=(const CustomScreenData & src);

      void clear();
      void copy(const CustomScreenData & src);
      bool isEmpty() const;
    };

    struct CustomScreens {
      CustomScreenData customScreenData[MAX_CUSTOM_SCREENS];

      CustomScreens() {}
      CustomScreens(const CustomScreens & src);
      CustomScreens & operator=(const CustomScreens & src);

      void clear();
      void copy(const CustomScreens & src);
    };

    static void init(const std::string layoutId, CustomScreens & customScreens);
};
