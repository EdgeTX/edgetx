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

#include "audio.h"
#include "translations.h"

#include "widgets/widgets_container_impl.h"
#include "view_main_decoration.h"
#include "layout.h"

#include <memory>

#define LAYOUT_COMMON_OPTIONS                                       \
  {STR_TOP_BAR, ZoneOption::Bool, OPTION_VALUE_BOOL(true)},         \
  {STR_FLIGHT_MODE, ZoneOption::Bool, OPTION_VALUE_BOOL(true)},     \
  {STR_SLIDERS, ZoneOption::Bool, OPTION_VALUE_BOOL(true)},         \
  {STR_TRIMS, ZoneOption::Bool, OPTION_VALUE_BOOL(true)},           \
  {STR_MIRROR, ZoneOption::Bool, OPTION_VALUE_BOOL(false)}

#define LAYOUT_OPTIONS_END \
  { nullptr, ZoneOption::Bool }

typedef WidgetsContainerImpl<MAX_LAYOUT_ZONES, MAX_LAYOUT_OPTIONS> LayoutBase;

class Layout: public LayoutBase
{
  friend class LayoutFactory;

  public:

    Layout(Window* parent, const LayoutFactory * factory, PersistentData * persistentData);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "Layout";
    }

    void paint(BitmapBuffer * dc) override;
#endif

    void create() override;
  
    const LayoutFactory * getFactory() const
    {
      return factory;
    }

    void checkEvents() override;
  
    bool hasTopbar() const {
      return getOptionValue(LAYOUT_OPTION_TOPBAR)->boolValue;
    }

    bool hasFlightMode() const {
      return getOptionValue(LAYOUT_OPTION_FM)->boolValue;
    }

    bool hasSliders() const {
      return getOptionValue(LAYOUT_OPTION_SLIDERS)->boolValue;
    }

    bool hasTrims() const {
      return getOptionValue(LAYOUT_OPTION_TRIMS)->boolValue;
    }

    bool isMirrored() const {
      return getOptionValue(LAYOUT_OPTION_MIRRORED)->boolValue;
    }

    // Set decoration visibility
    void setTrimsVisible(bool visible);
    void setSlidersVisible(bool visible);
    void setFlightModeVisible(bool visible);

    // Updates settings for trims, sliders, pots, etc...
    void adjustLayout() override;

  protected:
    const LayoutFactory * factory  = nullptr;
    std::unique_ptr<ViewMainDecoration> decoration;

    // Decoration settings bitmask to detect updates
    uint8_t  decorationSettings = 255;

    // Last time we refreshed the window
    uint32_t lastRefresh = 0;
  
    // Get the available space for widgets
    rect_t getMainZone() const;
};

template<class T>
class BaseLayoutFactory: public LayoutFactory
{
  public:
    BaseLayoutFactory(const char * id, const char * name, const uint8_t * bitmap, const ZoneOption * options):
      LayoutFactory(id, name),
      bitmap(bitmap),
      options(options)
    {
    }

    const uint8_t* getBitmap() const override { return bitmap; }

    void drawThumb(BitmapBuffer * dc, uint16_t x, uint16_t y, uint32_t flags) const override
    {
      dc->drawBitmapPattern(x, y, bitmap, flags);
    }

    const ZoneOption * getOptions() const override
    {
      return options;
    }

    Layout * create(Window* parent, Layout::PersistentData * persistentData) const override
    {
      initPersistentData(persistentData, true);
      Layout * layout = new T(parent, this, persistentData);
      if (layout) {
        layout->create();
      }
      return layout;
    }

    Layout * load(Window* parent, Layout::PersistentData * persistentData) const override
    {
      initPersistentData(persistentData, false);
      Layout * layout = new T(parent, this, persistentData);
      if (layout) {
        layout->load();
      }
      return layout;
    }

    void initPersistentData(Layout::PersistentData* persistentData,
                            bool setDefault) const override
    {
      if (setDefault) {
        memset(persistentData, 0, sizeof(Layout::PersistentData));
      }
      if (options) {
        int i = 0;
        for (const ZoneOption* option = options; option->name; option++, i++) {
          TRACE("LayoutFactory::initPersistentData() setting option '%s'",
                option->name);
          // TODO compiler bug? The CPU freezes ... persistentData->options[i++]
          // = option->deflt;
          auto optVal = &persistentData->options[i];
          if (setDefault) {
            memcpy(&optVal->value, &option->deflt, sizeof(ZoneOptionValue));
          }
          optVal->type = zoneValueEnumFromType(option->type);
        }
      }
    }

  protected:
    const uint8_t * bitmap;
    const ZoneOption * options;
};
