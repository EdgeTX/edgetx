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

#include "definitions.h"
#include "dataconstants.h"
#include "widget.h"
#include "view_main_decoration.h"
#include "messaging.h"

#include <memory>

class LayoutFactory;

//-----------------------------------------------------------------------------

struct LayoutOption
{
  enum Type {
    Integer,
    Bool,
    Color,
  };

  STR_TYP name;
  LayoutOption::Type type;
  LayoutOptionValue deflt;
};

// Common 'LayoutOptionValue's among all layouts
enum {
  LAYOUT_OPTION_TOPBAR = 0,
  LAYOUT_OPTION_FM,
  LAYOUT_OPTION_SLIDERS,
  LAYOUT_OPTION_TRIMS,
  LAYOUT_OPTION_MIRRORED,

  LAYOUT_OPTION_LAST_DEFAULT=LAYOUT_OPTION_MIRRORED
};

#define LAYOUT_COMMON_OPTIONS                           \
  {STR_DEF(STR_TOP_BAR), LayoutOption::Bool, true},     \
  {STR_DEF(STR_FLIGHT_MODE), LayoutOption::Bool, true}, \
  {STR_DEF(STR_SLIDERS), LayoutOption::Bool, true},     \
  {STR_DEF(STR_TRIMS), LayoutOption::Bool, true},       \
  {STR_DEF(STR_MIRROR), LayoutOption::Bool, false}

#define LAYOUT_OPTIONS_END \
  { nullptr, LayoutOption::Bool }

extern const LayoutOption defaultLayoutOptions[];

//-----------------------------------------------------------------------------

#define LAYOUT_MAP_DIV      60
#define LAYOUT_MAP_0        0
#define LAYOUT_MAP_1QTR     15
#define LAYOUT_MAP_1THIRD   20
#define LAYOUT_MAP_HALF     30
#define LAYOUT_MAP_2THIRD   40
#define LAYOUT_MAP_3QTR     45
#define LAYOUT_MAP_FULL     60

//-----------------------------------------------------------------------------

class Layout: public WidgetsContainer
{
 public:
  Layout(Window* parent, const LayoutFactory * factory,
          int screenNum, uint8_t zoneCount, uint8_t* zoneMap);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override
  {
    return "Layout";
  }
#endif

  Widget* createWidget(unsigned int index,
                       const WidgetFactory* factory) override;

  void load();

  LayoutOptionValue* getOptionValue(unsigned int index) const;

  const LayoutFactory * getFactory() const
  {
    return factory;
  }

  virtual bool hasTopbar() const;
  virtual bool hasFlightMode() const;
  virtual bool hasSliders() const;
  virtual bool hasTrims() const;
  virtual bool isMirrored() const;

  // Updates settings for trims, sliders, pots, etc...
  virtual void updateDecorations();
  void show(bool visible = true) override;

  bool isLayout() override { return true; }

  void removeWidget(unsigned int index) override;

  bool hasFullScreenWidget() const;

 protected:
  const LayoutFactory* factory = nullptr;
  ViewMainDecoration* decoration = nullptr;
  uint8_t* zoneMap = nullptr;
  int screenNum;
  rect_t lastMainZone = {0,0,0,0};
  Messaging decorationUpdateMsg;
  bool zoneUpdateRequired = false;

  // Last time we refreshed the window
  uint32_t lastRefresh = 0;

  // Get the available space for widgets
  rect_t getWidgetsZone() const;

  unsigned int getZonesCount() const override { return zoneCount; }
  rect_t getZone(unsigned int index) const override;

  void checkEvents() override;
};

//-----------------------------------------------------------------------------

class LayoutFactory
{
 public:
  LayoutFactory(const char* id, const char* name, const LayoutOption * options, uint8_t zoneCount, uint8_t* zoneMap);
  ~LayoutFactory();

  const char* getId() const { return id; }
  const char* getName() const { return name; }

  const MaskBitmap* getBitmap() const { return bitmap; }

  const LayoutOption * getLayoutOptions() const
  {
    return options;
  }

  virtual Layout* createNew(Window* parent, int screenNum) const = 0;

  WidgetsContainer* create(Window* parent, int screenNum) const;
  WidgetsContainer* load(Window* parent, int screenNum) const;

  void initPersistentData(int screenNum, bool setDefault) const;

  // delete all custom screens from memory
  static void deleteCustomScreens(bool clearTopBar = false);

  // intended for existing models
  static void loadCustomScreens();

  // intented for new models
  static void loadDefaultLayout();

  // List of registered layout factories
  static std::list<const LayoutFactory*>& getRegisteredLayouts();

  WidgetsContainer* createCustomScreen(unsigned customScreenIndex) const;

  static LAYOUT_ORIENTATION_SCALED(BM_W, 51, 22)
  static LAYOUT_ORIENTATION_SCALED(BM_H, 25, 34)

 protected:
  const char* id;
  const char* name;
  const LayoutOption * options;
  uint8_t zoneCount;
  uint8_t* zoneMap;
  MaskBitmap * bitmap = nullptr;

  static WidgetsContainer* loadLayout(Window* parent, int screenNum);
  static const LayoutFactory* getLayoutFactory(const char* name);
};

//-----------------------------------------------------------------------------

template<class T>
class BaseLayoutFactory: public LayoutFactory
{
 public:
  BaseLayoutFactory(const char * id, const char * name,
                    const LayoutOption * options, uint8_t zoneCount, uint8_t* zoneMap) :
    LayoutFactory(id, name, options, zoneCount, zoneMap)
  {
  }

  Layout* createNew(Window* parent, int screenNum) const override
  {
    return new T(parent, this, screenNum, zoneCount, zoneMap);
  }
};

extern const LayoutFactory* defaultLayout;

extern WidgetsContainer* customScreens[MAX_CUSTOM_SCREENS];

//-----------------------------------------------------------------------------
