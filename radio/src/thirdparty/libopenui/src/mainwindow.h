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

#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

#include <utility>
#include "window.h"
#include "bitmapbuffer.h"

class MainWindow: public Window {
  public:
    MainWindow():
      Window(nullptr, {0, 0, LCD_W, LCD_H}),
      invalidatedRect(rect)
    {
    }

    ~MainWindow() override
    {
      children.clear();
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "MainWindow";
    }
#endif

    void checkEvents() override;

    void invalidate()
    {
      invalidate({0, 0, rect.w, rect.h});
    }

    void invalidate(const rect_t & rect) override;

    bool refresh();

    void run(bool trash=true);

  protected:
    static void emptyTrash();
    rect_t invalidatedRect;
    const char * shutdown = nullptr;
};

extern MainWindow mainWindow;

#endif // _MAINWINDOW_H_
