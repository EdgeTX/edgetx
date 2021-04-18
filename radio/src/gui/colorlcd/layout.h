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

#include <list>
#include "widgets_container.h"

#define MAX_LAYOUT_ZONES               10
#define MAX_LAYOUT_OPTIONS             10
constexpr coord_t TRIM_LINE_WIDTH = 8;
constexpr coord_t TRIM_SQUARE_SIZE = 17;
constexpr coord_t MAIN_ZONE_BORDER = 10;
constexpr uint32_t LAYOUT_REFRESH = 1000 / 2; // 2 Hz

class BitmapBuffer;

#if !defined(YAML_GENERATOR)
typedef WidgetsContainerPersistentData<MAX_LAYOUT_ZONES,MAX_LAYOUT_OPTIONS> LayoutPersistentData;
#else
struct LayoutPersistentData {
  ZonePersistentData   zones[MAX_LAYOUT_ZONES];
  ZoneOptionValueTyped options[MAX_LAYOUT_OPTIONS];
};
#endif

class LayoutFactory
{
  public:
    LayoutFactory(const char * id, const char * name);
    const char * getId() const { return id; }
    const char * getName() const { return name; }
    virtual void drawThumb(BitmapBuffer * dc, uint16_t x, uint16_t y, LcdFlags flags) const = 0;
    virtual const ZoneOption * getOptions() const = 0;
    virtual WidgetsContainer * create(LayoutPersistentData * persistentData) const = 0;
    virtual WidgetsContainer * load(LayoutPersistentData * persistentData) const = 0;

  protected:
    const char * id;
    const char * name;
};

WidgetsContainer * loadLayout(const char * name, LayoutPersistentData * persistentData);

// intented for new models
void loadDefaultLayout();

// intended for existing models
void loadCustomScreens();

// delete all custom screens from memory
void deleteCustomScreens();

WidgetsContainer *
createCustomScreen(const LayoutFactory* factory, unsigned customScreenIndex);

// Remove custom screen from the model
void disposeCustomScreen(unsigned idx);

// Layout must register to be found
void registerLayout(const LayoutFactory * factory);

// List of registered layout factories
std::list<const LayoutFactory *> & getRegisteredLayouts();
