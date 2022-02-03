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
#include <algorithm>
#include <vector>
#include <iostream>
#include <string>
#include <set>
#include "bitmapbuffer.h"
#include "libopenui.h"
#include "touch.h"

// base class for lists of elements with names
class ListBase : public FormField
{
  public:
    ListBase(Window *parent, const rect_t &rect, std::vector<std::string> names,
              std::function<uint32_t()> getValue = nullptr,
              std::function<void(uint32_t)> setValue = nullptr, 
              uint8_t lineHeight = MENUS_LINE_HEIGHT,
              WindowFlags windowFlags = 0, LcdFlags lcdFlags = 0);

    typedef enum {
      LISTBOX_SINGLE_SELECT,
      LISTBOX_MULTI_SELECT
    } selecttype_e;

    // draw one line of the list. Default implementation just draws the names.  Oher types of lists
    // can draw anything they want.
    virtual void drawLine(BitmapBuffer *dc, const rect_t &rect, uint32_t index, LcdFlags lcdFlags);
    void paint (BitmapBuffer *dc) override;

    virtual void setSelected(int selected);
    virtual void setFocusLine(int selected);
    void clearSelection() {selectedIndexes.clear(); invalidate();}

    void setMultiSelectHandler(std::function<void(std::set<uint32_t>)> handler)
    {
      _multiSelectHandler = std::move(handler);
    }

    void setLongPressHandler(std::function<void(event_t)> handler)
    {
      longPressHandler = std::move(handler);
    }

    void setPressHandler(std::function<void(event_t)> handler)
    {
      pressHandler = std::move(handler);
    }

    void setSelectionMode(selecttype_e st)
    {
      selectionType = st;
    }

    void setActiveIndex(int index)
    {
      if (index >= 0 && index < (int) names.size()) {
        activeIndex = index;
      } else {
        activeIndex = -1;
      }

      invalidate();
    }

    inline int getSelected() 
    {
      return selected;
    }

    inline void setLineHeight(uint8_t height)
    {
      lineHeight = height;
    }

    inline void setNames(std::vector<std::string> names)
    {
      this->names.assign(names.begin(), names.end());
      setInnerHeight(names.size() * lineHeight);
      invalidate();
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "ListBox";
    }
#endif

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    void checkEvents(void) override;
    bool onTouchEnd(coord_t x, coord_t y) override;
    bool onTouchStart(coord_t x, coord_t y) override;
    bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY) override;
    bool isLongPress();
#endif

  protected:
    std::function<void(event_t)> longPressHandler = nullptr;
    std::function<void(event_t)> pressHandler = nullptr;
    std::vector<std::string> names;
    std::function<uint32_t()> _getValue;
    std::function<void(uint32_t)> _setValue;
    std::function<void(std::set<uint32_t>)> _multiSelectHandler = nullptr;
    int lineHeight;
    int32_t selected = -1;
    int32_t activeIndex = -1;
    std::set<uint32_t> selectedIndexes;
    selecttype_e selectionType = LISTBOX_SINGLE_SELECT;

#if defined(HARDWARE_TOUCH)
    uint32_t duration10ms;
    coord_t yDown;
#endif
};


class ListBox : public ListBase
{
 public:
  using ListBase::ListBase;

  inline void setTitle(std::string title)
  {
    this->title = title;
  }

 protected:
  std::string title;
};

