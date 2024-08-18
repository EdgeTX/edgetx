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

#include <list>

#include "dataconstants.h"
#include "widgets_container.h"

#define MAX_LAYOUT_ZONES 10
#define MAX_LAYOUT_OPTIONS 10

#if !defined(YAML_GENERATOR)
typedef WidgetsContainerPersistentData<MAX_LAYOUT_ZONES, MAX_LAYOUT_OPTIONS>
    LayoutPersistentData;
#else
struct LayoutPersistentData {
  ZonePersistentData zones[MAX_LAYOUT_ZONES];
  ZoneOptionValueTyped options[MAX_LAYOUT_OPTIONS];
};
#endif

class LayoutFactory
{
 public:
  LayoutFactory(const char* id, const char* name);

  const char* getId() const { return id; }
  const char* getName() const { return name; }

  virtual const uint8_t* getBitmap() const = 0;

  virtual const ZoneOption* getOptions() const = 0;

  virtual WidgetsContainer* create(
      Window* parent, LayoutPersistentData* persistentData) const = 0;

  virtual WidgetsContainer* load(
      Window* parent, LayoutPersistentData* persistentData) const = 0;

  virtual void initPersistentData(LayoutPersistentData* persistentData,
                                  bool setDefault) const = 0;

  // Remove custom screen from the model
  static void disposeCustomScreen(unsigned idx);

  // delete all custom screens from memory
  static void deleteCustomScreens();

  // intended for existing models
  static void loadCustomScreens();

  // intented for new models
  static void loadDefaultLayout();

  // List of registered layout factories
  static std::list<const LayoutFactory*>& getRegisteredLayouts();

  WidgetsContainer* createCustomScreen(unsigned customScreenIndex) const;

  static LAYOUT_VAL(TRIM_LINE_WIDTH, 8, 8)
  static LAYOUT_VAL(TRIM_SQUARE_SIZE, 17, 17)

  static LAYOUT_VAL(BM_W, 51, 22)
  static LAYOUT_VAL(BM_H, 25, 34)

 protected:
  const char* id;
  const char* name;

  static WidgetsContainer* loadLayout(Window* parent, const char* name,
                                      LayoutPersistentData* persistentData);
  static const LayoutFactory* getLayoutFactory(const char* name);
};

extern const LayoutFactory* defaultLayout;

extern WidgetsContainer* customScreens[MAX_CUSTOM_SCREENS];
