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

#include "menu_statistics.h"

#include "edgetx.h"
#include "view_statistics.h"

PageDef statsMenuItems[] = {
  { ICON_STATS_THROTTLE_GRAPH, STR_STATISTICS, QuickMenu::STATS_STATS, [](PageDef& pageDef) { return new StatisticsViewPage(pageDef); }},
  { ICON_STATS_DEBUG, STR_DEBUG, QuickMenu::STATS_DEBUG, [](PageDef& pageDef) { return new DebugViewPage(pageDef); }},
  { EDGETX_ICONS_COUNT }
};

StatisticsViewPageGroup::StatisticsViewPageGroup() : PageGroup(ICON_STATS, statsMenuItems)
{
}
