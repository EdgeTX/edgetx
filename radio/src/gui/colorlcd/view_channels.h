/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "edgetx.h"
#include "tabsgroup.h"

class ChannelsViewPage: public PageTab
{
  public:
    explicit ChannelsViewPage(uint8_t pageIndex = 0):
      PageTab(STR_MONITOR_CHANNELS[pageIndex], ICON_MONITOR_CHANNELS1 + pageIndex),
      pageIndex(pageIndex)
    {
    }

  protected:
    void build(FormWindow * window) override;
    uint8_t pageIndex = 0;
};

class ChannelsViewMenu: public TabsGroup {
  public:
    ChannelsViewMenu();
};
