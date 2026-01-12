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

#include "widgets_container.h"
#include "widget.h"

WidgetsContainer::WidgetsContainer(Window* parent, const rect_t& rect, uint8_t zoneCount) :
  Window(parent, rect), zoneCount(zoneCount)
{
  widgets = new Widget*[zoneCount];
  for (int i = 0; i < zoneCount; i += 1) widgets[i] = nullptr;
}

void WidgetsContainer::deleteLater(bool detach, bool trash)
{
  if (deleted()) return;
  for (int i = 0; i < zoneCount; i += 1)
    if (widgets[i]) widgets[i]->deleteLater();
  if (widgets) delete[] widgets;
  widgets = nullptr;
  Window::deleteLater(detach, trash);
}

Widget* WidgetsContainer::getWidget(unsigned int index)
{
  if (index < zoneCount) return widgets[index];

  return nullptr;
}

void WidgetsContainer::removeWidget(unsigned int index)
{
  if (index >= zoneCount) return;

  if (widgets[index]) {
    widgets[index]->deleteLater();
  }

  widgets[index] = nullptr;
}

void WidgetsContainer::removeAllWidgets()
{
  for (unsigned int i = 0; i < zoneCount; i++)
    removeWidget(i);
}

void WidgetsContainer::updateZones()
{
  for (int i = 0; i < zoneCount; i++) {
    if (widgets[i]) {
      auto zone = getZone(i);
      widgets[i]->setRect(zone);
      widgets[i]->updateZoneRect(zone);
    }
  }
}

void WidgetsContainer::showWidgets(bool visible)
{
  for (int i = 0; i < zoneCount; i++) {
    if (widgets[i]) {
      widgets[i]->show(visible);
    }
  }
}

void WidgetsContainer::refreshWidgets(bool inForeground)
{
  if (!_deleted) {
    for (int i = 0; i < zoneCount; i++) {
      if (widgets[i]) {
        if ((inForeground && widgets[i]->isOnScreen()) || widgets[i]->isFullscreen())
          widgets[i]->foreground();
        else
          widgets[i]->background();
      }
    }
  }
}
