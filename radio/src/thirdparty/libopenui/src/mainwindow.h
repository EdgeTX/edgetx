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

#include <utility>
#include "layer.h"
#include "bitmapbuffer.h"

extern LvglWidgetFactory mainWindowFactory;

class MainWindow: public Window
{
  protected:
    // singleton
    MainWindow():
      Window(nullptr, {0, 0, LCD_W, LCD_H}, 0, 0, &mainWindowFactory),
      invalidatedRect(rect)
    {
      Layer::push(this);
    }

  public:
    ~MainWindow() override
    {
      children.clear();
    }

    static MainWindow * instance()
    {
      if (!_instance)
        _instance = new MainWindow();

      return _instance;
    }

#if defined(TESTS)
    static void create()
    {
      _instance = new MainWindow();
    }
#endif

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "MainWindow";
    }
#endif

    void checkEvents() override;

#if defined(HARDWARE_TOUCH)
    void setTouchEnabled(bool enable);
    bool touchEventOccured() {
      if(_touchEventOccured)
      {
        _touchEventOccured = false;
        return true;
      }
      return false;
    }
#endif

    void invalidate()
    {
      invalidate({0, 0, rect.w, rect.h});
    }

    void invalidate(const rect_t & rect) override;

    bool needsRefresh() const
    {
      return invalidatedRect.w > 0;
    }

    bool refresh();

    void run(bool trash=true);

  protected:
    static MainWindow * _instance;
    static void emptyTrash();
    rect_t invalidatedRect;
    const char * shutdown = nullptr;
#if defined(HARDWARE_TOUCH)
    bool lastTouchState = false;
    bool _touchEventOccured = false;

    enum TouchEnableState {
    	TouchOn,
		TouchOn_Touching,
		TouchOnToOff_Touching,
		TouchOff,
		TouchOff_Touching,
		TouchOffToOn_Touching
    };
    TouchEnableState touchEnableState = TouchOn;
#endif
};
