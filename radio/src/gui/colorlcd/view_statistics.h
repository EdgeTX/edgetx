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

#include "opentx.h"
#include "tabsgroup.h"
#include "static.h"
#include "window.h"
#include "font.h"

template <class T>
class DebugInfoNumber : public Window
{
  public:
    DebugInfoNumber(Window * parent, const rect_t & rect, std::function<T()> numberHandler, LcdFlags textFlags = 0, const char * prefix = nullptr, const char * suffix = nullptr):
    Window(parent, rect, 0, textFlags),
    prefix(prefix),
    suffix(suffix)
    {
      coord_t prefixSize = 0;
      if (prefix != nullptr) {
        prefixSize = getTextWidth(prefix, 0, HEADER_COLOR | FONT(XS));
        new StaticText(this, {0, 0,  prefixSize, rect.h}, prefix, 0, HEADER_COLOR | FONT(XS));
      }
      new DynamicNumber<uint32_t>(this, {prefixSize, 0, rect.w - prefixSize, rect.h},
                                  numberHandler);
    }

  protected:
    const char * prefix;
    const char * suffix;
};

class StatisticsViewPage : public PageTab
{
  public:
    StatisticsViewPage() :
      PageTab(STR_STATISTICS, ICON_STATS_THROTTLE_GRAPH)
    {
    }

  protected:
    void build(FormWindow * window) override;
};

class StatisticsViewPageGroup: public TabsGroup {
  public:
    StatisticsViewPageGroup();
};

class DebugViewPage : public PageTab
{
  public:
    DebugViewPage() :
      PageTab(STR_DEBUG, ICON_STATS_DEBUG)
    {
    }

  protected:
    void build(FormWindow * window) override;
};

class DebugViewMenu: public TabsGroup {
  public:
    DebugViewMenu();
};
