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

#ifndef _MENU_H_
#define _MENU_H_

#include <vector>
#include <functional>
#include <utility>
#include "mainwindow.h"

class Menu;
class MenuWindow;

class MenuBody: public Window {
  friend class MenuWindow;
  friend class Menu;

  class MenuLine {
      friend class MenuBody;

    public:
      MenuLine(std::string text, std::function<void()> onPress):
        text(std::move(text)),
        onPress(std::move(onPress))
      {
      }

      MenuLine(std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine, std::function<void()> onPress):
        drawLine(std::move(drawLine)),
        onPress(std::move(onPress))
      {
      }

      MenuLine(MenuLine &) = delete;

      MenuLine(MenuLine &&) = default;

    protected:
      std::string text;
      std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine;
      std::function<void()> onPress;
  };

  public:
    MenuBody(Window * parent, const rect_t & rect, WindowFlags flags):
      Window(parent, rect, flags)
    {
      setPageHeight(MENUS_LINE_HEIGHT);
    }

    void select(int index);

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

    void addLine(const std::string & text, std::function<void()> onPress)
    {
      lines.emplace_back(text, onPress);
      invalidate();
    }

    void addCustomLine(std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine, std::function<void()> onPress)
    {
      lines.emplace_back(drawLine, onPress);
      invalidate();
    }

    void removeLines()
    {
      lines.clear();
      invalidate();
    }

    void setCancelHandler(std::function<void()> handler)
    {
      onCancel = handler;
    }

    void paint(BitmapBuffer * dc) override;

  protected:
    std::vector<MenuLine> lines;
#if defined(HARDWARE_TOUCH)
    int selectedIndex = -1;
#else
    int selectedIndex = 0;
#endif
    std::function<void()> onCancel;
};

class MenuWindow: public Window {
  friend class Menu;

  public:
    explicit MenuWindow(Menu * parent);

    ~MenuWindow() override
    {
      body.detach();
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "MenuWindow";
    }
#endif

    void setTitle(const std::string text)
    {
      title = std::move(text);
    }

    void paint(BitmapBuffer * dc) override;

  protected:
    MenuBody body;
    std::string title;
};

class Menu: public Window {
  public:
    Menu();
    
    void setCancelHandler(std::function<void()> handler)
    {
      menuWindow->body.setCancelHandler(handler);
    }

    void deleteLater()
    {
#if !defined(HARDWARE_TOUCH)
      if (previousFocus) {
        previousFocus->setFocus();
      }
#endif
      Window::deleteLater();
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "Menu";
    }
#endif

    void setToolbar(Window * window)
    {
      toolbar = window;
      menuWindow->setLeft(toolbar->right());
      menuWindow->setTop(toolbar->top());
      menuWindow->setHeight(toolbar->height());
    }

    void setTitle(const std::string text);

    void addLine(const std::string & text, std::function<void()> onPress);

    void addCustomLine(std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine, std::function<void()> onPress);

    void removeLines();

    inline void select(int index)
    {
      menuWindow->body.select(index);
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchStart(coord_t x, coord_t y) override
    {
      return true;
    }

    bool onTouchEnd(coord_t x, coord_t y) override;

    bool onTouchSlide(coord_t x, coord_t y, coord_t startX, coord_t startY, coord_t slideX, coord_t slideY) override;
#endif

  protected:
    Window * previousFocus = nullptr;
    MenuWindow * menuWindow;
    Window * toolbar = nullptr;
    void updatePosition();
};

#endif
