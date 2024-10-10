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

#include "form.h"
#include "bitmaps.h"

class StaticIcon;

// Base button class - default LVGL style unless overridden on create
class ButtonBase : public FormField
{
 public:
  ButtonBase(Window* parent, const rect_t& rect,
             std::function<uint8_t(void)> pressHandler = nullptr,
             LvglCreate objConstruct = nullptr);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ButtonBase"; }
#endif

  virtual void onPress();
  bool onLongPress() override;

  void onClicked() override;
  void checkEvents() override;

  void check(bool checked = true);
  bool checked() const;

  void setPressHandler(std::function<uint8_t(void)> handler)
  {
    pressHandler = std::move(handler);
  }

  void setLongPressHandler(std::function<uint8_t(void)> handler)
  {
    longPressHandler = std::move(handler);
  }

  void setCheckHandler(std::function<void(void)> handler)
  {
    checkHandler = std::move(handler);
  }

 protected:
  std::function<uint8_t(void)> pressHandler;
  std::function<uint8_t(void)> longPressHandler;
  std::function<void(void)> checkHandler;
};

class Button : public ButtonBase
{
 public:
  Button(Window* parent, const rect_t& rect,
         std::function<uint8_t(void)> pressHandler = nullptr);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "Button"; }
#endif
};

class TextButton : public ButtonBase
{
 public:
  TextButton(Window* parent, const rect_t& rect, std::string text,
             std::function<uint8_t(void)> pressHandler = nullptr);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "TextButton \"" + text + "\""; }
#endif

  void setText(std::string value)
  {
    if (value != text) {
      text = std::move(value);
      lv_label_set_text(label, text.c_str());
    }
  }

  void setWrap()
  {
    lv_obj_set_width(label, lv_pct(100));
    etx_obj_add_style(label, styles->text_align_center, LV_PART_MAIN);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  }

 protected:
  lv_obj_t* label = nullptr;

  std::string text;
};

class IconButton : public ButtonBase
{
 public:
  IconButton(Window* parent, EdgeTxIcon icon, coord_t x, coord_t y,
             std::function<uint8_t(void)> pressHandler = nullptr);
  
  void setIcon(EdgeTxIcon icon);

 protected:
  StaticIcon* iconImage = nullptr;
};
