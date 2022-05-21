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

#include <vector>
#include <functional>
#include <utility>
#include <memory>
#include "modal_window.h"

class Menu;
class MenuWindowContent;

struct lvobj_delete {
  constexpr lvobj_delete() = default;
  void operator()(lv_obj_t* obj) const {
    lv_obj_del(obj);
  }
};

class MenuBody: public Window
{
  friend class MenuWindowContent;
  friend class Menu;

  enum MENU_DIRECTION
  {
    DIRECTION_UP = 1,
    DIRECTION_DOWN = -1
  };

  class MenuLine
  {
    friend class MenuBody;

   public:
    MenuLine(std::function<void()> onPress, std::function<bool()> isChecked,
             lv_obj_t *icon) :
        onPress(std::move(onPress)), isChecked(std::move(isChecked)), icon(icon)
    {
    }

    MenuLine(MenuLine &) = delete;
    MenuLine(MenuLine &&) = default;

    lv_obj_t* getIcon() { return icon.get(); }
    
   protected:
    std::function<void()> onPress;
    std::function<bool()> isChecked;
    std::unique_ptr<lv_obj_t, lvobj_delete> icon;
  };

  public:
    MenuBody(Window * parent, const rect_t & rect);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "MenuBody";
    }
#endif

    void setIndex(int index);

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

    void addLine(const std::string &text, std::function<void()> onPress,
                 std::function<bool()> isChecked);

    void addLine(const uint8_t *icon_mask, const std::string &text,
                 std::function<void()> onPress,
                 std::function<bool()> isChecked);

    void removeLines();

    void setCancelHandler(std::function<void()> handler)
    {
      onCancel = std::move(handler);
    }

    coord_t getContentHeight();

    void onPress(size_t index);
  
  protected:
    friend void menu_draw_begin(lv_event_t *);
    friend void menu_draw_end(lv_event_t *);

    void selectNext(MENU_DIRECTION direction);
    int rangeCheck(int);

    std::vector<MenuLine> lines;
    int selectedIndex = 0;
    std::function<void()> onCancel;

    inline Menu * getParentMenu();
};

class MenuWindowContent: public ModalWindowContent
{
  friend class Menu;

  public:
    explicit MenuWindowContent(Menu * parent);

    coord_t getHeaderHeight() const;

    void deleteLater(bool detach = true, bool trash = true) override;

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "MenuWindowContent";
    }
#endif

  protected:
    MenuBody body;
};

class Menu: public ModalWindow
{
  friend class MenuBody;

  public:
    explicit Menu(Window * parent, bool multiple = false);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "Menu";
    }
#endif
    
    void setCancelHandler(std::function<void()> handler)
    {
      content->body.setCancelHandler(std::move(handler));
    }

    void setWaitHandler(std::function<void()> handler)
    {
      waitHandler = std::move(handler);
    }

    void setFocusBody(uint8_t flag = SET_FOCUS_DEFAULT)
    {
      content->body.setFocus(flag);
    }

    void setToolbar(Window * window);

    void setTitle(std::string text);

    void addLine(const std::string &text, std::function<void()> onPress,
                 std::function<bool()> isChecked = nullptr);

    void addLine(const uint8_t *icon_mask, const std::string &text,
                 std::function<void()> onPress,
                 std::function<bool()> isChecked = nullptr);

    void addSeparator();

    void removeLines();

    unsigned count() const
    {
      return content->body.count();
    }

    int selection() const
    {
      return content->body.selection();
    }

    void select(int index)
    {
      content->body.setIndex(index);
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
    friend void menuBodyEventCallback(lv_event_t *);
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

