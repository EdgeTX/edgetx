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

#include "widget.h"
#include "widgets_container.h"

template<int N, int O>
class WidgetsContainerImpl: public WidgetsContainer
{
  public:
    typedef WidgetsContainerPersistentData<N,O> PersistentData;

    WidgetsContainerImpl(const rect_t & rect, PersistentData * persistentData):
      WidgetsContainer(nullptr, rect, FORM_FORWARD_FOCUS),
      persistentData(persistentData)
    {
    }

    Widget * createWidget(unsigned int index, const WidgetFactory * factory) override
    {
      if (index >= N)
        return nullptr;

      // remove old one if existing
      removeWidget(index);

      Widget * widget = nullptr;
      if (factory) {
        strncpy(persistentData->zones[index].widgetName, factory->getName(), sizeof(ZonePersistentData::widgetName));
        widget = factory->create(this, getZone(index), &persistentData->zones[index].widgetData);
      }
      widgets[index] = widget;

      if (widget)
        widget->attach(this);

      return widget;
    }

    void removeWidget(unsigned int index) override
    {
      if (index >= N)
        return;

      if (widgets[index])
        widgets[index]->deleteLater();

      widgets[index] = nullptr;
      memset(persistentData->zones[index].widgetName, 0, sizeof(ZonePersistentData::widgetName));
      memset(&persistentData->zones[index].widgetData, 0, sizeof(Widget::PersistentData));
    }

    Widget * getWidget(unsigned int index) override
    {
      if (index < N)
        return widgets[index];

      return nullptr;
    }

    virtual void create()
    {
      memset(persistentData, 0, sizeof(PersistentData));
    }

    virtual void load()
    {
      unsigned int count = getZonesCount();
      for (unsigned int i = 0; i < count; i++) {

        // remove old widget
        if (widgets[i]) {
          widgets[i]->deleteLater();
          widgets[i] = nullptr;
        }

        // and load new one if required
        if (persistentData->zones[i].widgetName[0]) {
          char name[WIDGET_NAME_LEN + 1];
          memset(name, 0, sizeof(name));
          strncpy(name, persistentData->zones[i].widgetName, WIDGET_NAME_LEN);
          widgets[i] = loadWidget(name, this, getZone(i), &persistentData->zones[i].widgetData);
        }
      }
    }

    inline ZoneOptionValue * getOptionValue(unsigned int index) const
    {
      return &persistentData->options[index].value;
    }

    inline void setOptionValue(unsigned int index, const ZoneOptionValue& value)
    {
      persistentData->options[index].value = value;
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
      for (int i = 0; i < N; i++) {
        if (widgets[i]) {
          auto zone = getZone(i);
          widgets[i]->setRect(zone);
          widgets[i]->setInnerHeight(zone.h);
        }
      }
    }

    void adjustLayout() override
    {
    }
  
  protected:
    PersistentData * persistentData;
    Widget * widgets[N] = {};
};

template<class T>
class BaseWidgetFactory: public WidgetFactory
{
  public:
    BaseWidgetFactory(const char * name, const ZoneOption * options):
      WidgetFactory(name, options)
    {
    }

    Widget * create(FormGroup * parent, const rect_t & rect, Widget::PersistentData * persistentData, bool init = true) const override
    {
      if (init) {
        initPersistentData(persistentData);
      }

      return new T(this, parent, rect, persistentData);
    }
};
