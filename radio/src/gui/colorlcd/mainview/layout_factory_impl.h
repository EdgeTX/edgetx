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

#include "widgets_container_impl.h"
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

extern const ZoneOption defaultZoneOptions[];

#define LAYOUT_MAP_DIV      60
#define LAYOUT_MAP_0        0
#define LAYOUT_MAP_1QTR     15
#define LAYOUT_MAP_1THIRD   20
#define LAYOUT_MAP_HALF     30
#define LAYOUT_MAP_2THIRD   40
#define LAYOUT_MAP_3QTR     45
#define LAYOUT_MAP_FULL     60

class Layout: public LayoutBase
{
  friend class LayoutFactory;

  public:

    Layout(Window* parent, const LayoutFactory * factory, PersistentData * persistentData,
           uint8_t zoneCount, uint8_t* zoneMap);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "Layout";
    }
#endif

    void create() override {}
  
    const LayoutFactory * getFactory() const
    {
      return factory;
    }
  
    virtual bool hasTopbar() const {
      return getOptionValue(LAYOUT_OPTION_TOPBAR)->boolValue;
    }

    virtual bool hasFlightMode() const {
      return getOptionValue(LAYOUT_OPTION_FM)->boolValue;
    }

    virtual bool hasSliders() const {
      return getOptionValue(LAYOUT_OPTION_SLIDERS)->boolValue;
    }

    virtual bool hasTrims() const {
      return getOptionValue(LAYOUT_OPTION_TRIMS)->boolValue;
    }

    virtual bool isMirrored() const {
      return getOptionValue(LAYOUT_OPTION_MIRRORED)->boolValue;
    }

    virtual bool isAppMode() const { return false; }

    // Set decoration visibility
    void setTrimsVisible(bool visible);
    void setSlidersVisible(bool visible);
    void setFlightModeVisible(bool visible);

    // Updates settings for trims, sliders, pots, etc...
    void adjustLayout() override;
    void show(bool visible = true) override;

    bool isLayout() override { return true; }
  
    static LAYOUT_VAL(MAIN_ZONE_BORDER, 10, 10)

  protected:
    const LayoutFactory * factory  = nullptr;
    std::unique_ptr<ViewMainDecoration> decoration;
    uint8_t zoneCount;
    uint8_t* zoneMap = nullptr;

    enum DecorationSettings {
        DECORATION_NONE       = 0x00,
        DECORATION_TOPBAR     = 0x01,
        DECORATION_SLIDERS    = 0x02,
        DECORATION_TRIMS      = 0x04,
        DECORATION_FLIGHTMODE = 0x08,
        DECORATION_MIRRORED   = 0x10,
        DECORATION_UNKNOWN    = 0xFF
    };

    // Decoration settings bitmask to detect updates
    uint8_t  decorationSettings = DECORATION_UNKNOWN;

    // Last time we refreshed the window
    uint32_t lastRefresh = 0;
  
    // Get the available space for widgets
    rect_t getMainZone() const;

    unsigned int getZonesCount() const override { return zoneCount; }
    rect_t getZone(unsigned int index) const override;
};

template<class T>
class BaseLayoutFactory: public LayoutFactory
{
  public:
    BaseLayoutFactory(const char * id, const char * name,
                      const ZoneOption * options, uint8_t zoneCount, uint8_t* zoneMap):
      LayoutFactory(id, name),
      options(options),
      zoneCount(zoneCount),
      zoneMap(zoneMap)
    {
      this->bitmap = (uint8_t*)malloc(align32(BM_W * BM_H + 2 * sizeof(uint16_t) + 4));

      uint16_t* hdr = (uint16_t*)this->bitmap;
      hdr[0] = BM_W;
      hdr[1] = BM_H;

      uint8_t* bm = (uint8_t*)(this->bitmap + 2 * sizeof(uint16_t));
      memset(bm, 0, BM_W * BM_H);

      memset(bm, 0xFF, BM_W);
      memset(bm+((BM_H-1)*BM_W), 0xFF, BM_W);

      for (int y = 1; y < BM_H - 1; y += 1) {
        bm[y*BM_W] = 0xFF;
        bm[y*BM_W+BM_W-1] = 0xFF;
      }

      for (int i = 0; i < zoneCount * 4; i += 4) {
        uint8_t xo = BM_W * zoneMap[i] / LAYOUT_MAP_DIV;
        uint8_t yo = BM_H * zoneMap[i+1] / LAYOUT_MAP_DIV;
        uint8_t w = BM_W * zoneMap[i+2] / LAYOUT_MAP_DIV;
        uint8_t h = (BM_H * zoneMap[i+3] + LAYOUT_MAP_DIV/2) / LAYOUT_MAP_DIV;

        if (yo > 0)
          memset(bm+(yo*BM_W+xo), 0xFF, w);
        if (xo > 0) {
          for (int y = 0; y < h; y += 1) {
            bm[(yo+y)*BM_W+xo] = 0xFF;
          }
        }
      }
    }

    ~BaseLayoutFactory()
    {
      if (bitmap) {
        free((void*)bitmap);
        bitmap = nullptr;
      }
    }

    const uint8_t* getBitmap() const override { return bitmap; }

    const ZoneOption * getOptions() const override
    {
      return options;
    }

    Layout * create(Window* parent, Layout::PersistentData * persistentData) const override
    {
      initPersistentData(persistentData, true);
      Layout * layout = new T(parent, this, persistentData, zoneCount, zoneMap);
      if (layout) {
        layout->create();
      }
      return layout;
    }

    Layout * load(Window* parent, Layout::PersistentData * persistentData) const override
    {
      initPersistentData(persistentData, false);
      Layout * layout = new T(parent, this, persistentData, zoneCount, zoneMap);
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
    const uint8_t * bitmap = nullptr;
    const ZoneOption * options;
    uint8_t zoneCount;
    uint8_t* zoneMap;
};
