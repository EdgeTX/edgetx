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

#include "form.h"

constexpr WindowFlags BUTTON_BACKGROUND = FORM_FLAGS_LAST << 1u;
constexpr WindowFlags BUTTON_CHECKED = FORM_FLAGS_LAST << 2u;
constexpr WindowFlags BUTTON_CHECKED_ON_FOCUS = FORM_FLAGS_LAST << 3u;

class Button: public FormField
{
  public:
   Button(FormGroup* parent, const rect_t& rect,
          std::function<uint8_t(void)> pressHandler = nullptr,
          WindowFlags windowFlags = 0, LcdFlags textFlags = 0,
          LvglCreate objConstruct = nullptr) :
       FormField(parent, rect, windowFlags, textFlags, objConstruct),
       pressHandler(std::move(pressHandler))
   {
     lv_obj_set_style_bg_opa(lvobj, LV_OPA_TRANSP, LV_PART_MAIN);
   }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "Button";
    }
#endif

    void check(bool checked = true)
    {
      if (checked != bool(windowFlags & BUTTON_CHECKED)) {
        windowFlags ^= BUTTON_CHECKED;
        invalidate();
      }
    }

    bool checked() const
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
    void onEvent(event_t event) override;
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchStart(coord_t x, coord_t y) override;
    bool onTouchEnd(coord_t x, coord_t y) override;
#endif

    void checkEvents() override;

  protected:
    std::function<uint8_t(void)> pressHandler;
    std::function<void(void)> checkHandler;

    void onPress();
};


class TextButton: public Button
{
  public:
   TextButton(FormGroup* parent, const rect_t& rect, std::string text,
              std::function<uint8_t(void)> pressHandler = nullptr,
              WindowFlags windowFlags = BUTTON_BACKGROUND | OPAQUE,
              LcdFlags textFlags = 0);
              
#if defined(DEBUG_WINDOWS)
   std::string getName() const override
   {
     return "TextButton \"" + text + "\"";
   }
#endif

   void setText(std::string value)
   {
     if (value != text) {
       text = std::move(value);
       lv_label_set_text(label, text.c_str());
     }
   }

   void setBgColorHandler(std::function<LcdFlags(void)> handler = nullptr)
   {
     bgColorHandler = std::move(handler);
   }

  protected:
    lv_obj_t * label = nullptr;

    std::string text;
    std::function<LcdFlags(void)> bgColorHandler = nullptr;
};

class IconButton: public Button
{
  public:
    IconButton(FormGroup * parent, const rect_t & rect, uint8_t icon, std::function<uint8_t(void)> pressHandler, WindowFlags flags = 0):
      Button(parent, rect, std::move(pressHandler), flags),
      icon(icon)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "IconButton(" + std::to_string(icon) + ")";
    }
#endif

    void paint(BitmapBuffer * dc) override;

  protected:
    uint8_t icon;
};
