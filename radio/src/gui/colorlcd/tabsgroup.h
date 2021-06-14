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

#ifndef _TABSGROUP_H_
#define _TABSGROUP_H_

#include <vector>
#include "button.h"
#include "form.h"
#include "keyboard_base.h"

class TabsGroup;

class PageTab {
  friend class TabsCarousel;
  friend class TabsGroup;

  public:
    PageTab()
    {
    }

    PageTab(std::string title, unsigned icon):
      title(std::move(title)),
      icon(icon)
    {
    }

    virtual ~PageTab() = default;

    virtual void build(FormWindow * window) = 0;

    virtual void checkEvents()
    {
    }

    void destroy()
    {
      if (onPageDestroyed) {
        onPageDestroyed();
      }
      delete this;
    }

    void setOnPageDestroyedHandler(std::function<void()> handler)
    {
      onPageDestroyed = std::move(handler);
    }

    void setTitle(std::string value)
    {
      title = std::move(value);
    }

    void setIcon(unsigned icon)
    {
      this->icon = icon;
    }

    unsigned getIcon() const
    {
      return icon;
    }

  protected:
    std::string title;
    unsigned icon = 0;
    std::function<void()> onPageDestroyed;
};

class TabsCarousel: public Window {
  public:
    TabsCarousel(Window * parent, TabsGroup * menu);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "TabsCarousel";
    }
#endif

    inline void setCurrentIndex(uint8_t index)
    {
      currentIndex = index;
    }

    inline uint8_t getCurrentIndex()
    {
      return currentIndex;
    }

    void updateInnerWidth();

    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

  protected:
    constexpr static uint8_t padding_left = 3;
    TabsGroup * menu;
    uint8_t currentIndex = 0;
};

class TabsGroupHeader: public FormGroup {
    friend class TabsGroup;

  public:
    TabsGroupHeader(TabsGroup * menu, uint8_t icon);

    void deleteLater(bool detach = true, bool trash = true) override
    {
      if (_deleted)
        return;

#if defined(HARDWARE_TOUCH)
      back.deleteLater(true, false);
#endif

      carousel.deleteLater(true, false);

      FormField::deleteLater(detach, trash);
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "TabsGroupHeader";
    }
#endif

    void paint(BitmapBuffer * dc) override;

    void setTitle(const char * value)
    {
      title = value;
    }

  protected:
#if defined(HARDWARE_TOUCH)
    Button back;
#endif
    uint8_t icon;
    TabsCarousel carousel;
    const char * title = nullptr;
};

class TabsGroup: public Window
{
    friend class TabsCarousel;

  public:
    explicit TabsGroup(uint8_t icon);

    ~TabsGroup() override;

    void deleteLater(bool detach = true, bool trash = true) override
    {
      if (_deleted)
        return;

#if defined(HARDWARE_TOUCH)
      Keyboard::hide();
#endif

      header.deleteLater(true, false);
      body.deleteLater(true, false);

      Window::deleteLater(detach, trash);
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "TabsGroup";
    }
#endif

    unsigned getTabs() const
    {
      return tabs.size();
    }
  
    void addTab(PageTab * page);

    // Return the index of the found tab
    // or -1 if the tab could not be found
    int removeTab(PageTab * page);

    void removeTab(unsigned index);

    void removeAllTabs();

    void setCurrentTab(unsigned index)
    {
      if (index < tabs.size()) {
        header.carousel.setCurrentIndex(index);
        setVisibleTab(tabs[index]);
      }
    }
  
    void checkEvents() override;

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

  protected:
    TabsGroupHeader header;
    FormWindow body;
    std::vector<PageTab *> tabs;
    PageTab * currentTab = nullptr;

    void setVisibleTab(PageTab * tab);
};

#endif
