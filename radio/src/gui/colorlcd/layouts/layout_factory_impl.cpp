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

#include "layout_factory_impl.h"
#include "layouts/trims.h"
#include "layouts/sliders.h"
#include "view_main_decoration.h"
#include "view_main.h"

Layout::Layout(const LayoutFactory * factory, PersistentData * persistentData):
  LayoutBase({0, 0, LCD_W, LCD_H}, persistentData),
  factory(factory),
  decoration(new ViewMainDecoration(this, getRect()))
{
  adjustLayout();
}

void Layout::create()
{
  memset(persistentData, 0, sizeof(PersistentData));

  getOptionValue(LAYOUT_OPTION_TOPBAR)->boolValue   = true;
  getOptionValue(LAYOUT_OPTION_FM)->boolValue       = true;
  getOptionValue(LAYOUT_OPTION_SLIDERS)->boolValue  = true;
  getOptionValue(LAYOUT_OPTION_TRIMS)->boolValue    = true;
  getOptionValue(LAYOUT_OPTION_MIRRORED)->boolValue = false;
}    

#if defined(DEBUG_WINDOWS)
void Layout::paint(BitmapBuffer * dc)
{
  TRACE_WINDOWS("# painting -> %s", getWindowDebugString().c_str());
  LayoutBase::paint(dc);
}
#endif

void Layout::checkEvents()
{
  LayoutBase::checkEvents();
  adjustLayout();

  uint32_t now = RTOS_GET_MS();
  if (now - lastRefresh >= LAYOUT_REFRESH) {
    lastRefresh = now;
    invalidate();

#if defined(DEBUG_WINDOWS)
    TRACE_WINDOWS("# %s refresh: %s", factory->getId(), getWindowDebugString().c_str());
#endif
  }
}

void Layout::setTrimsVisible(bool visible)
{
  decoration->setTrimsVisible(visible);
}

void Layout::setSlidersVisible(bool visible)
{
  decoration->setSlidersVisible(visible);
}

void Layout::setFlightModeVisible(bool visible)
{
  decoration->setFlightModeVisible(visible);
}

void Layout::adjustLayout()
{
  // Check if deco setting are still up-to-date
  uint8_t checkSettings =
    (hasTopbar() ? 1 << 0 : 0) |
    (hasSliders() ? 1 << 1 : 0) |
    (hasTrims() ? 1 << 2 : 0) |
    (hasFlightMode() ? 1 << 3 : 0) |
    (isMirrored() ? 1 << 4 : 0);

  if (checkSettings == decorationSettings) {
    // everything ok, exit!
    return;
  }

  // Save settings
  decorationSettings = checkSettings;

  // Set visible decoration
  setSlidersVisible(hasSliders());
  setTrimsVisible(hasTrims());
  setFlightModeVisible(hasFlightMode());

  // Re-compute positions
  decoration->adjustDecoration();

  // and update relevant windows
  updateZones();

  // probably not needed
  //invalidate();
}

rect_t Layout::getMainZone() const
{
  rect_t zone = decoration->getMainZone();
  return ViewMain::instance()->getMainZone(zone, hasTopbar());
}    

