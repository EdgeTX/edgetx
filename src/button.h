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

constexpr WindowFlags BUTTON_BACKGROUND = FORM_FLAGS_LAST << 1;
constexpr WindowFlags BUTTON_CHECKED = FORM_FLAGS_LAST << 2;
constexpr WindowFlags BUTTON_CHECKED_ON_FOCUS = FORM_FLAGS_LAST << 3;
constexpr WindowFlags BUTTON_DISABLED = FORM_FLAGS_LAST << 4;

class Button : public FormField {
  public:
    Button(FormGroup * parent, const rect_t & rect, std::function<uint8_t(void)> pressHandler = nullptr, WindowFlags windowFlags = 0):
      FormField(parent, rect, windowFlags),
      pressHandler(pressHandler)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() override
    {
      return "Button";
    }
#endif

    void enable(bool enabled = true)
    {
      if (!enabled != bool(windowFlags & BUTTON_DISABLED)) {
        windowFlags ^= BUTTON_DISABLED;
        invalidate();
      }
    }

    void disable()
    {
      enable(false);
    }

    void check(bool checked = true)
    {
      if (checked != bool(windowFlags & BUTTON_CHECKED)) {
        windowFlags ^= BUTTON_CHECKED;
        invalidate();
      }
    }

    bool enabled()
    {
      return !(windowFlags & BUTTON_DISABLED);
    }

    bool checked()
    {
      if (windowFlags & BUTTON_CHECKED_ON_FOCUS)
        return hasFocus();
      else
        return windowFlags & BUTTON_CHECKED;
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

    void onPress();
};

class TextButton : public Button {
  public:
    TextButton(FormGroup * parent, const rect_t & rect, std::string text, std::function<uint8_t(void)> pressHandler = nullptr, WindowFlags windowFlags = BUTTON_BACKGROUND):
      Button(parent, rect, pressHandler, windowFlags | OPAQUE),
      text(std::move(text))
    {
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
    IconButton(FormGroup * parent, const rect_t & rect, uint8_t icon, std::function<uint8_t(void)> pressHandler, WindowFlags flags = 0):
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
