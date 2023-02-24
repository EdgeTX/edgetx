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

#include "font.h"
#include "opentx.h"
#include "static.h"
#include "tabsgroup.h"
#include "window.h"

class StatisticsViewPage : public PageTab
{
 public:
  StatisticsViewPage() : PageTab(STR_STATISTICS, ICON_STATS_THROTTLE_GRAPH) {}

 protected:
  void build(FormWindow* window) override;
};

class StatisticsViewPageGroup : public TabsGroup
{
 public:
  StatisticsViewPageGroup();
};

class DebugViewPage : public PageTab
{
 public:
  DebugViewPage() : PageTab(STR_DEBUG, ICON_STATS_DEBUG) {}

 protected:
  void build(FormWindow* window) override;
};

class DebugViewMenu : public TabsGroup
{
 public:
  DebugViewMenu();
};
