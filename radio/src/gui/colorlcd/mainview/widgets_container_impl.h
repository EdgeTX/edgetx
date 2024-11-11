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

#include "strhelpers.h"
#include "widget.h"

template <int N, int O>
class WidgetsContainerImpl : public WidgetsContainer
{
 public:
  typedef WidgetsContainerPersistentData<N, O> PersistentData;

  WidgetsContainerImpl(Window* parent, const rect_t& rect,
                       PersistentData* persistentData) :
      WidgetsContainer(parent, rect), persistentData(persistentData)
  {
    setWindowFlag(NO_FOCUS);
  }

  Widget* createWidget(unsigned int index,
                       const WidgetFactory* factory) override
  {
    if (index >= N) return nullptr;

    // remove old one if existing
    removeWidget(index);

    Widget* widget = nullptr;
    if (factory) {
      strAppend(persistentData->zones[index].widgetName, factory->getName(),
                WIDGET_NAME_LEN);
      widget = factory->create(this, getZone(index),
                               &persistentData->zones[index].widgetData);
    }
    widgets[index] = widget;

    if (widget) widget->attach(this);

    return widget;
  }

  void removeWidget(unsigned int index) override
  {
    if (index >= N) return;

    if (widgets[index]) {
      widgets[index]->deleteLater();
    }

    widgets[index] = nullptr;
    memset(persistentData->zones[index].widgetName, 0,
           sizeof(ZonePersistentData::widgetName));
    memset(&persistentData->zones[index].widgetData, 0,
           sizeof(Widget::PersistentData));
  }

  Widget* getWidget(unsigned int index) override
  {
    if (index < N) return widgets[index];

    return nullptr;
  }

  virtual void create() { memset(persistentData, 0, sizeof(PersistentData)); }

  virtual void load()
  {
    unsigned int count = getZonesCount();
    for (unsigned int i = 0; i < count; i++) {
      // remove old widget
      if (widgets[i]) {
        widgets[i]->deleteLater();
        widgets[i] = nullptr;
      }
    }

    for (unsigned int i = 0; i < count; i++) {
      // and load new one if required
      if (persistentData->zones[i].widgetName[0]) {
        char name[WIDGET_NAME_LEN + 1];
        memset(name, 0, sizeof(name));
        strAppend(name, persistentData->zones[i].widgetName, WIDGET_NAME_LEN);
        widgets[i] = WidgetFactory::newWidget(
            name, this, getZone(i), &persistentData->zones[i].widgetData);
      }
    }
  }

  inline ZoneOptionValue* getOptionValue(unsigned int index) const
  {
    return &persistentData->options[index].value;
  }

  unsigned int getZonesCount() const override = 0;

  rect_t getZone(unsigned int index) const override = 0;

  virtual void background()
  {
    for (int i = 0; i < N; i++) {
      if (widgets[i]) {
        widgets[i]->background();
      }
    }
  }

  void updateZones() override
  {
    adjustLayout();
    for (int i = 0; i < N; i++) {
      if (widgets[i]) {
        auto zone = getZone(i);
        widgets[i]->setRect(zone);
        widgets[i]->updateZoneRect(zone);
      }
    }
  }

  void showWidgets(bool visible = true) override
  {
    for (int i = 0; i < N; i++) {
      if (widgets[i]) {
        widgets[i]->show(visible);
      }
    }
  }

  void adjustLayout() override {}

  void runBackground() override
  {
    for (int i = 0; i < N; i++) {
      if (widgets[i]) {
        widgets[i]->background();
      }
    }
  }

 protected:
  PersistentData* persistentData;
  Widget* widgets[N] = {nullptr};
};
