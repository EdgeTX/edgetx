/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <stdlib.h>
#include <list>
#include <string.h>
#include <utility>
#include <functional>
#include <string>
#include "bitmapbuffer.h"
#include "libopenui_defines.h"
#include "libopenui_helpers.h"
#include "libopenui_config.h"

// OPAQUE/TRANSPARENT defined in WinGDI
#ifndef OPAQUE
 #define OPAQUE               1
 #define TRANSPARENT          2
#endif

constexpr uint8_t NO_SCROLLBAR =          4;
constexpr uint8_t FORWARD_SCROLL =        8;
constexpr uint8_t REFRESH_ALWAYS =        16;
constexpr uint8_t BORDER_FOCUS_ONLY =     32;
constexpr uint8_t PAINT_CHILDREN_FIRST =  64;

class Window {
  friend class GridLayout;

  public:
    Window(Window * parent, const rect_t & rect, uint8_t flags=0);

    virtual ~Window();

#if defined(DEBUG_WINDOWS)
    virtual std::string getName()
    {
      return "Window";
    }

    std::string getRectString()
    {
      char result[32];
      sprintf(result, "[%d, %d, %d, %d]", left(), top(), width(), height());
      return result;
    }

    std::string getIndentString()
    {
      std::string result;
      auto tmp = parent;
      while (tmp) {
        result += "  ";
        tmp = tmp->getParent();
      }
      return result;
    }

    std::string getWindowDebugString()
    {
      return getIndentString() + getName() + " " + getRectString();
    }
#endif

    Window * getParent() const
    {
      return parent;
    }

    uint8_t getWindowFlags() const
    {
      return windowFlags;
    }

    void setWindowFlags(uint8_t flags)
    {
      windowFlags = flags;
    }

    void setCloseHandler(std::function<void()> handler)
    {
      closeHandler = std::move(handler);
    }

    void setFocusHandler(std::function<void()> handler)
    {
      focusHandler = std::move(handler);
    }

    void deleteLater(bool detach=true);

    void clear();

    void deleteChildren();

    bool hasFocus() const
    {
      return focusWindow == this;
    }

    static Window * getFocus()
    {
      return focusWindow;
    }

    void scrollTo(Window * child);

    static void clearFocus();

    void setFocus();

    void setRect(rect_t rect)
    {
      this->rect = rect;
      invalidate();
    }

    void setWidth(coord_t value)
    {
      rect.w = value;
      invalidate();
    }

    void setHeight(coord_t value)
    {
      rect.h = value;
      if (innerHeight <= value) {
        setScrollPositionY(0);
      }
      invalidate();
    }

    coord_t getHeight()
    {
      return rect.h;
    }

    void setLeft(coord_t x)
    {
      rect.x = x;
      invalidate();
    }

    void setTop(coord_t y)
    {
      rect.y = y;
      invalidate();
    }

    coord_t left() const
    {
      return rect.x;
    }

    coord_t right() const
    {
      return rect.x + rect.w;
    }

    coord_t top() const
    {
      return rect.y;
    }

    coord_t bottom() const
    {
      return rect.y + rect.h;
    }

    coord_t width() const
    {
      return rect.w;
    }

    coord_t height() const
    {
      return rect.h;
    }

    rect_t getRect() const
    {
      return rect;
    }

    void setInnerWidth(coord_t w)
    {
      innerWidth = w;
      if (width() >= w) {
        scrollPositionX = 0;
      }
    }

    void setInnerHeight(coord_t h)
    {
      innerHeight = h;
      if (height() >= h) {
        scrollPositionY = 0;
      }
    }

    coord_t getScrollPositionY() const
    {
      return scrollPositionY;
    }

    void setScrollPositionX(coord_t value);

    void setScrollPositionY(coord_t value);

    bool isChildVisible(Window * window);

    bool isChildFullSize(Window * window);

    bool isVisible()
    {
      return parent && parent->isChildVisible(this);
    }

    virtual void paint(BitmapBuffer * dc)
    {
    }

    void drawVerticalScrollbar(BitmapBuffer * dc);

    void paintChildren(BitmapBuffer * dc);

    void fullPaint(BitmapBuffer * dc);

    bool pointInRect(coord_t x, coord_t y, rect_t & rect)
    {
      return (x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h);
    }

    virtual void onFocusLost()
    {
      TRACE_WINDOWS("%s onFocusLost()", getWindowDebugString().c_str());
      invalidate();
    };

#if defined(HARDWARE_KEYS)
    virtual void onKeyEvent(event_t event);
#endif

#if defined(HARDWARE_TOUCH)
    virtual bool onTouchStart(coord_t x, coord_t y);

    virtual bool onTouchEnd(coord_t x, coord_t y);

    virtual bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY);
#endif

    void adjustInnerHeight();

    coord_t adjustHeight();

    void moveWindowsTop(coord_t y, coord_t delta);

    void invalidate()
    {
      invalidate({0, 0, rect.w, rect.h});
    }

    void bringToTop()
    {
      attach(parent); // does a detach + attach
    }

    virtual void checkEvents();

  protected:
    Window * parent;
    std::list<Window *> children;
    rect_t rect;
    coord_t innerWidth;
    coord_t innerHeight;
    coord_t scrollPositionX = 0;
    coord_t scrollPositionY = 0;
    uint8_t windowFlags;

    static Window * focusWindow;
    static std::list<Window *> trash;

    std::function<void()> closeHandler;
    std::function<void()> focusHandler;

    void  attach(Window * window);

    void detach();

    void addChild(Window * window)
    {
      children.push_back(window);
    }

    void removeChild(Window * window)
    {
      children.remove(window);
      invalidate();
    }

    virtual void invalidate(const rect_t & rect);
};

#endif // _WINDOW_H_
