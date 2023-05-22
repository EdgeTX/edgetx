/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#pragma once

#include "window.h"

class StaticText: public Window
{
  public:
    StaticText(Window *parent, const rect_t &rect, std::string text = "",
               WindowFlags windowFlags = 0, LcdFlags textFlags = 0);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override;
#endif

    void setText(std::string value);
    const std::string& getText() const;

    void setBackgroundColor(LcdFlags color);
    void setBackgroudOpacity(uint32_t opa);
    void setFont(LcdFlags font);


  protected:
    std::string text;
    LcdFlags bgColor = 0;
};

#define Subtitle(parent, text) StaticText(parent, rect_t{}, text, 0, COLOR_THEME_PRIMARY1 | FONT(BOLD))

class StaticBitmap: public Window
{
  public:
    StaticBitmap(Window * parent, const rect_t & rect, const BitmapBuffer * mask, LcdFlags color, bool scale = false):
      Window(parent, rect),
      bitmap(mask),
      color(color),
      scale(scale)
    {
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

class DynamicText : public StaticText
{
 public:
  DynamicText(Window *parent, const rect_t &rect,
              std::function<std::string()> textHandler,
              LcdFlags textFlags = 0) :
      StaticText(parent, rect, "", 0, textFlags),
      textHandler(std::move(textHandler))
  {
  }

  void checkEvents() override
  {
    StaticText::checkEvents();
    setText(textHandler());
  }

 protected:
  std::function<std::string()> textHandler;
};

template <class T>
class DynamicNumber : public StaticText
{
 public:
  DynamicNumber(Window *parent, const rect_t &rect,
                std::function<T()> numberHandler, LcdFlags textFlags = 0,
                const char *prefix = nullptr, const char *suffix = nullptr) :
      StaticText(parent, rect, "", 0, textFlags),
      numberHandler(std::move(numberHandler)),
      prefix(prefix),
      suffix(suffix)
  {
    updateText();
  }

  void checkEvents() override
  {
    T newValue = numberHandler();
    if (value != newValue) {
      value = newValue;
      updateText();
    }
  }

  void setPrefix(const char * value) {
    prefix = value;
    updateText();
  }

  void setSuffix(const char * value) {
    suffix = value;
    updateText();
  }

 protected:
  T value = 0;
  std::function<T()> numberHandler;
  const char *prefix;
  const char *suffix;

  void updateText();
};
