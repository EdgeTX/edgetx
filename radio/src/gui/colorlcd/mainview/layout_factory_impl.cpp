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

#include "layout_factory_impl.h"

#include "sliders.h"
#include "trims.h"
#include "view_main.h"

Layout::Layout(Window* parent, const LayoutFactory* factory,
               PersistentData* persistentData, uint8_t zoneCount,
               uint8_t* zoneMap) :
    LayoutBase(parent, {0, 0, LCD_W, LCD_H}, persistentData),
    factory(factory),
    decoration(new ViewMainDecoration(this)),
    zoneCount(zoneCount),
    zoneMap(zoneMap)
{
  show(true);
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

void Layout::show(bool visible)
{
  // Set visible decoration
  setSlidersVisible(visible && hasSliders());
  setTrimsVisible(visible && hasTrims());
  setFlightModeVisible(visible && hasFlightMode());

  if (visible) {
    // and update relevant windows
    updateZones();
  }
}

rect_t Layout::getMainZone() const
{
  rect_t zone = decoration->getMainZone();
  if (hasSliders() || hasTrims() || hasFlightMode()) {
    // some decoration activated
    zone.x += PAD_LARGE;
    zone.y += PAD_LARGE;
    zone.w -= 2 * PAD_LARGE;
    zone.h -= 2 * PAD_LARGE;
  }
  return ViewMain::instance()->getMainZone(zone, hasTopbar());
}

rect_t Layout::getZone(unsigned int index) const
{
  rect_t z = getMainZone();

  unsigned int i = index * 4;

  coord_t xo = z.w * zoneMap[i] / LAYOUT_MAP_DIV;
  coord_t yo = z.h * zoneMap[i + 1] / LAYOUT_MAP_DIV;
  coord_t w = z.w * zoneMap[i + 2] / LAYOUT_MAP_DIV;
  coord_t h = z.h * zoneMap[i + 3] / LAYOUT_MAP_DIV;

  if (isMirrored()) xo = z.w - xo - w;

  return {z.x + xo, z.y + yo, w, h};
}

const ZoneOption defaultZoneOptions[] = {LAYOUT_COMMON_OPTIONS,
                                         LAYOUT_OPTIONS_END};
