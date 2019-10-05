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

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "form.h"
#include <string>

enum ButtonFlags {
  BUTTON_BACKGROUND = 1,
  BUTTON_CHECKED = 2,
  BUTTON_NOFOCUS = 4,
  BUTTON_CHECKED_ON_FOCUS = 8,
  BUTTON_DISABLED = 16,
};

class Button : public FormField {
  public:
    Button(Window * parent, const rect_t & rect, std::function<uint8_t(void)> pressHandler=nullptr, uint8_t flags=0):
      FormField(parent, rect),
      pressHandler(pressHandler),
      flags(flags)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "Button";
    }
#endif

    void enable(bool enabled=true)
    {
      if (!enabled != bool(flags & BUTTON_DISABLED)) {
        flags ^= BUTTON_DISABLED;
        invalidate();
      }
    }

    void disable()
    {
      enable(false);
    }

    void check(bool checked=true)
    {
      if (checked != bool(flags & BUTTON_CHECKED)) {
        flags ^= BUTTON_CHECKED;
        invalidate();
      }
    }

    bool enabled()
    {
      return !(flags & BUTTON_DISABLED);
    }

    bool checked()
    {
      if (flags & BUTTON_CHECKED_ON_FOCUS)
        return hasFocus();
      else
        return flags & BUTTON_CHECKED;
    }

    void setPressHandler(std::function<uint8_t(void)> handler)
    {
      pressHandler = std::move(handler);
    }

    void setCheckHandler(std::function<void(void)> handler)
    {
      checkHandler = std::move(handler);
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event);
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

    void checkEvents() override;

  protected:
    std::function<uint8_t(void)> pressHandler;
    std::function<void(void)> checkHandler;
    uint8_t flags;

    void onPress();
};

class TextButton : public Button {
  public:
    TextButton(Window * parent, const rect_t & rect, std::string text, std::function<uint8_t(void)> pressHandler=nullptr, uint8_t flags=BUTTON_BACKGROUND):
      Button(parent, rect, pressHandler, flags),
      text(std::move(text))
    {
      windowFlags = OPAQUE;
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "TextButton \"" + text + "\"";
    }
#endif

    void setText(std::string value)
    {
      if (value != text) {
        text = std::move(value);
        invalidate();
      }
    }

    void paint(BitmapBuffer * dc) override;

  protected:
    std::string text;
};

class IconButton: public Button {
  public:
    IconButton(Window * parent, const rect_t & rect, uint8_t icon, std::function<uint8_t(void)> pressHandler, uint8_t flags = 0):
      Button(parent, rect, pressHandler, flags),
      icon(icon)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "IconButton(" + std::to_string(icon) + ")";
    }
#endif

    void paint(BitmapBuffer * dc) override;

  protected:
    uint8_t icon;
};

#endif
