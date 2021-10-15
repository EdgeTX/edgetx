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

#define MAX_LAYOUT_ZONES    10
#define MAX_LAYOUT_OPTIONS  10

#if defined(PCBNV14)
#define MAX_TOPBAR_ZONES     2
#else
#define MAX_TOPBAR_ZONES     4
#endif

#define WIDGET_NAME_LEN     10
#define MAX_WIDGET_OPTIONS   5
#define MAX_TOPBAR_OPTIONS   1
#define LAYOUT_ID_LEN       10

#include "zone_220.h"

struct WidgetPersistentData {
  ZoneOptionValueTyped options[MAX_WIDGET_OPTIONS];
};

struct ZonePersistentData {
  char widgetName[WIDGET_NAME_LEN];
  WidgetPersistentData widgetData;
};

struct LayoutPersistentData {
  ZonePersistentData   zones[MAX_LAYOUT_ZONES];
  ZoneOptionValueTyped options[MAX_LAYOUT_OPTIONS];
};

struct TopBarPersistentData {
  ZonePersistentData   zones[MAX_TOPBAR_ZONES];
  ZoneOptionValueTyped options[MAX_TOPBAR_OPTIONS];
};
