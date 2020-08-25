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

class MenuBody : public Window {
  friend class MenuWindowContent;
  friend class Menu;

  class MenuLine {
      friend class MenuBody;

    public:
      MenuLine(std::string text, std::function<void()> onPress, std::function<bool()> isChecked):
        text(std::move(text)),
        onPress(std::move(onPress)),
        isChecked(std::move(isChecked))
      {
      }

      MenuLine(std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine, std::function<void()> onPress, std::function<bool()> isChecked):
        drawLine(std::move(drawLine)),
        onPress(std::move(onPress)),
        isChecked(std::move(isChecked))
      {
      }

      MenuLine(MenuLine &) = delete;

      MenuLine(MenuLine &&) = default;

    protected:
      std::string text;
      std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine;
      std::function<void()> onPress;
      std::function<bool()> isChecked;
  };

  public:
    MenuBody(Window * parent, const rect_t & rect):
      Window(parent, rect, OPAQUE)
    {
      setPageHeight(MENUS_LINE_HEIGHT);
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "MenuBody";
    }
#endif

    void select(int index);

    int selection() const
    {
      return selectedIndex;
    }

    int count() const
    {
      return lines.size();
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

    void addLine(const std::string & text, std::function<void()> onPress, std::function<bool()> isChecked)
    {
      lines.emplace_back(text, onPress, isChecked);
      invalidate();
    }

    void addCustomLine(std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine, std::function<void()> onPress, std::function<bool()> isChecked)
    {
      lines.emplace_back(drawLine, onPress, isChecked);
      invalidate();
    }

    void removeLines()
    {
      lines.clear();
      invalidate();
    }

    void setCancelHandler(std::function<void()> handler)
    {
      onCancel = std::move(handler);
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

    inline Menu * getParentMenu();
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
    std::string getName() const override
    {
      return "MenuWindowContent";
    }
#endif

    void paint(BitmapBuffer * dc) override;

  protected:
    MenuBody body;
};

class Menu: public ModalWindow {
  friend class MenuBody;

  public:
    explicit Menu(Window * parent, bool multiple = false);
    
    void setCancelHandler(std::function<void()> handler)
    {
      content->body.setCancelHandler(std::move(handler));
    }

    void setWaitHandler(std::function<void()> handler)
    {
      waitHandler = std::move(handler);
    }

    void setToolbar(Window * window)
    {
      toolbar = window;
      content->setLeft(toolbar->right());
      content->setTop(toolbar->top());
      content->setHeight(toolbar->height());
    }

    void setTitle(std::string text);

    void addLine(const std::string & text, std::function<void()> onPress, std::function<bool()> isChecked = nullptr);

    void addCustomLine(std::function<void(BitmapBuffer * dc, coord_t x, coord_t y, LcdFlags flags)> drawLine, std::function<void()> onPress, std::function<bool()> isChecked = nullptr);

    void removeLines();

    unsigned count() const
    {
      return content->body.count();
    }

    void select(int index)
    {
      content->body.select(index);
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

    void checkEvents() override
    {
      ModalWindow::checkEvents();
      if (waitHandler) {
        waitHandler();
      }
    }

  protected:
    MenuWindowContent * content;
    bool multiple;
    Window * toolbar = nullptr;
    std::function<void()> waitHandler;
    void updatePosition();
};

Menu * MenuBody::getParentMenu()
{
  return static_cast<Menu *>(getParent()->getParent());
}

#endif // _MENU_H_
