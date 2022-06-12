/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
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
#include <algorithm>
#include <vector>
#include <iostream>
#include "bitmapbuffer.h"
#include "libopenui.h"
#include "theme_manager.h"
#include "listbox.h"

constexpr int MAX_BARS = 3;
typedef std::function<uint32_t (int pos)> getRGBFromPos;

struct ColorBar : public FormField {
  uint32_t maxValue = 0;
  uint32_t value = 0;
  bool invert = false;
  getRGBFromPos getRGB = nullptr;

  ColorBar(Window* parent, const rect_t& r, uint32_t value = 0,
           uint32_t maxValue = 0, bool invert = false);

  int valueToScreen(int value);
  uint32_t screenToValue(int pos);

  static void pressing(lv_event_t* e);
  static void on_key(lv_event_t* e);
  static void draw_end(lv_event_t* e);
};

struct ColorType
{
  ColorBar* bars[MAX_BARS];
  uint32_t screenHeight;

  ColorType(Window* parent, coord_t screenHeight);
  virtual ~ColorType();

  virtual uint32_t getRGB() = 0;
  virtual const char** getLabelChars() = 0;
};

struct HSVColorType : public ColorType {
  HSVColorType(Window* parent, uint32_t color);
  uint32_t getRGB() override;
  const char** getLabelChars() override;
};

struct RGBColorType : public ColorType {
  RGBColorType(Window* parent, uint32_t color);
  uint32_t getRGB() override;
  const char** getLabelChars() override;
};

enum COLOR_EDITOR_TYPE
{
  RGB_COLOR_EDITOR = 0,
  HSV_COLOR_EDITOR
};

// the ColorEditor() control is a group of other controls
class ColorEditor : public FormGroup
{
 public:
  ColorEditor(FormGroup* window, const rect_t rect, uint32_t color,
              std::function<void(uint32_t rgb)> setValue = nullptr);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ColorEditor"; }
#endif

  void setColorEditorType(COLOR_EDITOR_TYPE colorType);

 protected:
  ColorType* _colorType = nullptr;
  lv_obj_t* barLabels[MAX_BARS];
  lv_obj_t* barValLabels[MAX_BARS];
  std::function<void(uint32_t)> _setValue;
  uint32_t _color;

  void setRGB();
  static void value_changed(lv_event_t* e);
};
