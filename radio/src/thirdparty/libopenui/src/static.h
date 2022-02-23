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

#include "window.h"
#include "button.h" // TODO just for BUTTON_BACKGROUND

class StaticText: public Window
{
  public:
    StaticText(Window * parent, const rect_t & rect, std::string text = "", WindowFlags windowFlags = 0, LcdFlags textFlags = 0) :
      Window(parent, rect, windowFlags, textFlags),
      text(std::move(text))
    {
      label = lv_label_create(lvobj);
      lv_label_set_text(label, this->text.c_str());
      lv_obj_clear_flag(label, LV_OBJ_FLAG_CLICK_FOCUSABLE);
      lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

      lv_obj_set_style_text_font(lvobj, &lv_font_roboto_13, LV_PART_MAIN);
      lv_obj_set_style_text_color(label, makeLvColor(textFlags), LV_PART_MAIN);
      if (textFlags & CENTERED)
        lv_obj_center(label);
      else if (textFlags & RIGHT)
        lv_obj_align(label, LV_ALIGN_TOP_RIGHT, 0, 0);

      if (textFlags & FONT(BOLD)) {
      }

      if (windowFlags & BUTTON_BACKGROUND) {
        lv_obj_set_style_bg_opa(lvobj, LV_OPA_100, LV_PART_MAIN);
        setBackgroundColor(COLOR_THEME_SECONDARY2);
      }
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "StaticText \"" + text + "\"";
    }
#endif

    void setText(std::string value)
    {
      if (text != value) {
        text = std::move(value);
        lv_label_set_text(label, this->text.c_str());
      }
    }

    void setBackgroundColor(LcdFlags color)
    {
      bgColor = color;
      auto actualColor = COLOR_VAL(bgColor);
      lv_obj_set_style_bg_color(lvobj, lv_color_make(GET_RED(actualColor), GET_GREEN(actualColor), GET_BLUE(actualColor)), LV_PART_MAIN);
    }

  protected:
    lv_obj_t *label = nullptr;
    std::string text;
    LcdFlags bgColor = 0;
};

class Subtitle: public StaticText {
  public:
    Subtitle(Window * parent, const rect_t & rect, const char * text, WindowFlags windowFlags = 0, LcdFlags textFlags = 0):
      StaticText(parent, rect, text, windowFlags, textFlags | FONT(BOLD))
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "Subtitle \"" + text + "\"";
    }
#endif
};

class StaticBitmap: public Window
{
  public:
    StaticBitmap(Window * parent, const rect_t & rect, bool scale = false):
      Window(parent, rect),
      scale(scale)
    {
    }

    StaticBitmap(Window * parent, const rect_t & rect, const char * filename, bool scale = false):
      Window(parent, rect),
      bitmap(BitmapBuffer::loadBitmap(filename)),
      scale(scale)
    {
    }

    StaticBitmap(Window * parent, const rect_t & rect, const BitmapBuffer * bitmap, bool scale = false):
      Window(parent, rect),
      bitmap(bitmap),
      scale(scale)
    {
    }

    StaticBitmap(Window * parent, const rect_t & rect, const BitmapBuffer * mask, LcdFlags color, bool scale = false):
      Window(parent, rect),
      bitmap(mask),
      color(color),
      scale(scale)
    {
    }

    void setBitmap(const char * filename)
    {
      setBitmap(BitmapBuffer::loadBitmap(filename));
    }

    void setMaskColor(LcdFlags value)
    {
      color = value;
    }

    void setBitmap(const BitmapBuffer * newBitmap)
    {
      delete bitmap;
      bitmap = newBitmap;
      invalidate();
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "StaticBitmap";
    }
#endif

    void paint(BitmapBuffer * dc) override
    {
      if (bitmap) {
        if (color != 0xFFFFFFFF)
          dc->drawMask(0, 0, bitmap, color);
        else if (scale)
          dc->drawScaledBitmap(bitmap, 0, 0, width(), height());
        else
          dc->drawBitmap((width() - bitmap->width()) / 2, (height() - bitmap->height()) / 2, bitmap);
      }
    }

  protected:
    const BitmapBuffer * bitmap = nullptr;
    LcdFlags color = 0xFFFFFFFF;
    bool scale = false;
};

class DynamicText: public StaticText
{
  public:
    DynamicText(Window * parent, const rect_t & rect, std::function<std::string()> textHandler, LcdFlags textFlags = 0):
      StaticText(parent, rect, "", 0, textFlags),
      textHandler(std::move(textHandler))
    {
    }

    void checkEvents() override
    {
      StaticText::checkEvents();
      std::string newText = textHandler();
      if (newText != text) {
        text = newText;
        invalidate();
      }
    }

  protected:
    std::function<std::string()> textHandler;
};

template <class T>
class DynamicNumber: public Window
{
  public:
    DynamicNumber(Window * parent, const rect_t & rect, std::function<T()> numberHandler, LcdFlags textFlags = 0, const char * prefix = nullptr, const char * suffix = nullptr):
      Window(parent, rect, 0, textFlags),
      numberHandler(std::move(numberHandler)),
      prefix(prefix),
      suffix(suffix)
    {
    }

    void paint(BitmapBuffer * dc) override
    {
      dc->drawNumber(0, FIELD_PADDING_TOP, value, textFlags, 0, prefix, suffix);
    }

    void checkEvents() override
    {
      T newValue = numberHandler();
      if (value != newValue) {
        value = newValue;
        invalidate();
      }
    }

  protected:
    T value = 0;
    std::function<T()> numberHandler;
    const char * prefix;
    const char * suffix;
};
