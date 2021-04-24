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

#ifndef _WIDGETS_CONTAINER_H_
#define _WIDGETS_CONTAINER_H_

#include <stdlib.h>
#include "libopenui_types.h"
#include "form.h"
#include "zone.h"

#define WIDGET_NAME_LEN     10
#define MAX_WIDGET_OPTIONS   5 // Name?

#define MAX_TOPBAR_ZONES     4
#define MAX_TOPBAR_OPTIONS   1 // just because of VC++ which doesn't like 0-size arrays :(

// Common 'ZoneOptionValue's among all layouts
enum {
  LAYOUT_OPTION_TOPBAR = 0,
  LAYOUT_OPTION_FM,
  LAYOUT_OPTION_SLIDERS,
  LAYOUT_OPTION_TRIMS,
  LAYOUT_OPTION_MIRRORED,

  LAYOUT_OPTION_LAST_DEFAULT=LAYOUT_OPTION_MIRRORED
};

class Widget;
class WidgetFactory;
class LayoutFactory;

struct WidgetPersistentData {
  ZoneOptionValueTyped options[MAX_WIDGET_OPTIONS] USE_IDX;
};

struct ZonePersistentData {
  char widgetName[WIDGET_NAME_LEN];
  WidgetPersistentData widgetData;
};

template<int N, int O>
struct WidgetsContainerPersistentData {
  ZonePersistentData   zones[N];
  ZoneOptionValueTyped options[O];
};

typedef WidgetsContainerPersistentData<MAX_TOPBAR_ZONES, MAX_TOPBAR_OPTIONS> TopBarPersistentData;

class WidgetsContainer: public FormGroup
{
  public:
    using FormGroup::FormGroup;
  
    virtual unsigned int getZonesCount() const = 0;
    virtual rect_t getZone(unsigned int index) const = 0;
    virtual Widget * createWidget(unsigned int index, const WidgetFactory * factory) = 0;
    virtual Widget * getWidget(unsigned int index) = 0;
    virtual void removeWidget(unsigned int index) = 0;
    virtual void adjustLayout() = 0;
    virtual void updateZones() = 0;
};


#endif // _WIDGETS_CONTAINER_H_
