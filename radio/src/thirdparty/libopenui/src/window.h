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

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <list>
#include <string>
#include <utility>
#include <functional>
#include "bitmapbuffer.h"
#include "libopenui_defines.h"
#include "libopenui_helpers.h"
#include "libopenui_config.h"
#include "LvglWrapper.h"

typedef uint32_t WindowFlags;

#if !defined(_GNUC_)
  #undef OPAQUE
  #undef TRANSPARENT
#endif

constexpr int INFINITE_HEIGHT = INT32_MAX;

constexpr WindowFlags OPAQUE =                1u << 0u;
constexpr WindowFlags TRANSPARENT =           1u << 1u;
constexpr WindowFlags NO_SCROLLBAR =          1u << 2u;
constexpr WindowFlags NO_FOCUS =              1u << 3u;
constexpr WindowFlags FORWARD_SCROLL =        1u << 4u;
constexpr WindowFlags REFRESH_ALWAYS =        1u << 5u;
constexpr WindowFlags PAINT_CHILDREN_FIRST =  1u << 6u;
constexpr WindowFlags PUSH_FRONT =  1u << 7u;
constexpr WindowFlags WINDOW_FLAGS_LAST =  PUSH_FRONT;

enum SetFocusFlag
{
  SET_FOCUS_DEFAULT  = 0,
  SET_FOCUS_FORWARD  = 1,
  SET_FOCUS_BACKWARD = 2,
  SET_FOCUS_FIRST    = 3,
  SET_FOCUS_NO_SCROLL= 4
};

typedef lv_obj_t *(*LvglCreate)(lv_obj_t *);

class Window
{
  friend class GridLayout;

  public:
   Window(Window *parent, const rect_t &rect, WindowFlags windowFlags = 0,
          LcdFlags textFlags = 0, LvglCreate objConstruct = nullptr);

   Window(Window *parent, lv_obj_t* lvobj);

   virtual ~Window();

#if defined(DEBUG_WINDOWS)
    virtual std::string getName() const;
    std::string getRectString() const;
    std::string getIndentString() const;
    std::string getWindowDebugString(const char * name = nullptr) const;
#endif

    Window *getParent() const { return parent; }

    bool isChild(Window *window) const
    {
      return window == this || (parent && parent->isChild(window));
    }

    Window *getFullScreenWindow();

    WindowFlags getWindowFlags() const { return windowFlags; }
    void setWindowFlags(WindowFlags flags);

    LcdFlags getTextFlags() const { return textFlags; }
    void setTextFlags(LcdFlags flags);

    typedef std::function<void()> CloseHandler;
    void setCloseHandler(CloseHandler h) { closeHandler = std::move(h); }

    typedef std::function<void(bool)> FocusHandler;
    void setFocusHandler(FocusHandler h) { focusHandler = std::move(h); }

    const std::list<Window *> getChildren() { return children; }

    void clear();
    virtual void deleteLater(bool detach = true, bool trash = true);

    bool hasFocus() const
    {
      return focusWindow == this;
    }

    static Window * getFocus()
    {
      return focusWindow;
    }

    void scrollTo(const rect_t & rect);

    void scrollTo(Window * child);

    static void clearFocus();

    virtual void setFocus(uint8_t flag = SET_FOCUS_DEFAULT, Window * from = nullptr);

    void setRect(rect_t value)
    {
      rect = value;
      lv_obj_set_pos(lvobj, rect.x, rect.y);
      lv_obj_set_width(lvobj, rect.w);
      lv_obj_set_height(lvobj, rect.h);
      invalidate();
    }

    void setWidth(coord_t value)
    {
      rect.w = value;
      lv_obj_set_width(lvobj, rect.w);
      invalidate();
    }

    void setWindowCentered()
    {
      rect.x = (parent->width() - width()) / 2;
      rect.y = (parent->height() - height()) / 2;
      lv_obj_set_pos(lvobj, rect.x, rect.y);
    }

    void setHeight(coord_t value)
    {
      rect.h = value;
      if (lvobj != nullptr) lv_obj_set_height(lvobj, rect.h);
      invalidate();
    }

    void setLeft(coord_t x)
    {
      rect.x = x;
      lv_obj_set_pos(lvobj, rect.x, rect.y);
      invalidate();
    }

    void setTop(coord_t y)
    {
      rect.y = y;
      lv_obj_set_pos(lvobj, rect.x, rect.y);
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

    bool isChildVisible(const Window * window) const;
    bool isChildFullSize(const Window * window) const;

    bool isVisible() const
    {
      return parent && parent->isChildVisible(this);
    }

    virtual void onEvent(event_t event);

    void adjustInnerHeight();

    coord_t adjustHeight();

    void moveWindowsTop(coord_t y, coord_t delta);

    virtual void updateSize();

    void invalidate()
    {
      invalidate({0, 0, rect.w, rect.h});
    }

    void bringToTop();

    virtual void checkEvents();

    void attach(Window * window);

    void detach();

    bool deleted() const
    {
      return _deleted;
    }

    virtual void paint(BitmapBuffer *)
    {
    }
  
#if defined(HARDWARE_TOUCH)
    virtual bool onTouchStart(coord_t x, coord_t y);
    virtual bool onTouchEnd(coord_t x, coord_t y);
#endif
  
    inline lv_obj_t *getLvObj() { return lvobj; }

  protected:
    static Window* focusWindow;
    static std::list<Window*> trash;

    rect_t rect;

    Window*   parent = nullptr;
    lv_obj_t* lvobj = nullptr;

    std::list<Window *> children;

    WindowFlags windowFlags = 0;
    LcdFlags    textFlags = 0;

    bool _deleted = false;

    std::function<void()> closeHandler;
    std::function<void(bool)> focusHandler;

    void deleteChildren();

    virtual void addChild(Window * window);
    void removeChild(Window * window);

    virtual void invalidate(const rect_t & rect);

    void fullPaint(BitmapBuffer * dc);
    void paintChildren(BitmapBuffer * dc, std::list<Window *>::iterator it);

    virtual void onFocusLost()
    {
      TRACE_WINDOWS("%s onFocusLost()", getWindowDebugString().c_str());
      if (focusHandler) {
        focusHandler(false);
      }
      invalidate();
    }

#if defined(HARDWARE_TOUCH)
    virtual bool onTouchSlide(coord_t x, coord_t y, coord_t startX,
                              coord_t startY, coord_t slideX, coord_t slideY);
#endif


    bool hasOpaqueRect(const rect_t & testRect) const;
};

