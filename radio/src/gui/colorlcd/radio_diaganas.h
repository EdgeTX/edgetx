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

class RadioAnalogsDiagsViewPageGroup : public TabsGroup
{
 public:
  RadioAnalogsDiagsViewPageGroup();
};

class AnaCalibratedViewPage : public PageTab
{
 public:
  AnaCalibratedViewPage() : PageTab(STR_ANADIAGS_CALIB, ICON_STATS_ANALOGS) {}

 protected:
  void build(FormWindow* window) override;
};

class AnaFilteredDevViewPage : public PageTab
{
 public:
  AnaFilteredDevViewPage() : PageTab(STR_ANADIAGS_FILTRAWDEV, ICON_STATS_THROTTLE_GRAPH) {}

 protected:
  void build(FormWindow* window) override;
};

class AnaUnfilteredRawViewPage : public PageTab
{
 public:
  AnaUnfilteredRawViewPage() : PageTab(STR_ANADIAGS_UNFILTRAW, ICON_RADIO_HARDWARE) {}

 protected:
  void build(FormWindow* window) override;
};

class AnaMinMaxViewPage : public PageTab
{
 public:
  AnaMinMaxViewPage() : PageTab(STR_ANADIAGS_MINMAX, ICON_RADIO_CALIBRATION) {}

 protected:
  void build(FormWindow* window) override;
};
