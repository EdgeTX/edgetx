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
#include "modal_window.h"

class Menu;
class MenuWindowContent;

class MenuBody: public Window {
  friend class MenuWindowContent;
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
    MenuBody(Window * parent, const rect_t & rect):
      Window(parent, rect)
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

class MenuWindowContent: public ModalWindowContent {
  friend class Menu;

  public:
    explicit MenuWindowContent(Menu * parent);

    ~MenuWindowContent() override
    {
      body.detach();
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "MenuWindowContent";
    }
#endif

    void paint(BitmapBuffer * dc) override;

  protected:
    MenuBody body;
};

class Menu: public ModalWindow {
  public:
    Menu();
    
    void setCancelHandler(std::function<void()> handler)
    {
      content->body.setCancelHandler(handler);
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
      content->setLeft(toolbar->right());
      content->setTop(toolbar->top());
      content->setHeight(toolbar->height());
    }

    void setTitle(const std::string text);

    void addLine(const std::string & text, std::function<void()> onPress);

    void addCustomLine(std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine, std::function<void()> onPress);

    void removeLines();

    inline void select(int index)
    {
      content->body.select(index);
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

  protected:
    MenuWindowContent * content;
    Window * toolbar = nullptr;
    void updatePosition();
};

#endif // _MENU_H_
