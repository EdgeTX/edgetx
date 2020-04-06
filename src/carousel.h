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

#ifndef _CAROUSEL_H_
#define _CAROUSEL_H_

#include <vector>
#include "button.h"

constexpr coord_t CAROUSEL_SPACING = 20;

class CarouselItem {
  public:
    CarouselItem(Window * front, Window * back):
      front(front),
      back(back)
    {
    }

  public:
    Window * front;
    Window * back;
};

class CarouselWindow: public Window {
  friend class Carousel;

  public:
    CarouselWindow(Window * parent, const rect_t & rect):
      Window(parent, rect, NO_SCROLLBAR)
    {
    }

    ~CarouselWindow() override
    {
      for (auto & item: items) {
        item.front->deleteLater();
        item.back->deleteLater();
      }
    }

    void addItem(CarouselItem item)
    {
      items.emplace_back(item);
      update();
    }

    void clear()
    {
      items.clear();
      Window::clear();
    }

    void select(int index, bool scroll = true)
    {
      selection = index;
      update();
    }

  protected:
    std::vector<CarouselItem> items;
    int selection = 0;
    void update();
};

class Carousel: public Window {
  public:
    Carousel(Window * parent, const rect_t & rect):
      Window(parent, rect, NO_SCROLLBAR)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "Carousel";
    }
#endif

    void addItem(CarouselItem item)
    {
      body->addItem(std::move(item));
    }

    void clear()
    {
      body->clear();
    }

    void select(int index)
    {
      body->select(index);
      previousButton->enable(index > 0);
      nextButton->enable(index < (int)body->items.size() - 1);
    }

    int getSelection()
    {
      return body->selection;
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

  protected:
    Button * previousButton = nullptr;
    Button * nextButton = nullptr;
    CarouselWindow * body = nullptr;
};

#endif // _CAROUSEL_H_
