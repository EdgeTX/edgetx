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

// ColorTypes()
// A ColorType implements an editor for selecting a color. Currently we support HSV, RGB and SYS (choose from system defined colors)
class ColorType
{
public:
  ColorType() {}
  virtual ~ColorType() {}

  virtual void setText() {};
  virtual uint32_t getRGB() { return 0; };

protected:
};

// Color editor with three bars for selecting color value. Base class for HSV and RGB color types.
class BarColorType : public ColorType
{
public:
  BarColorType(Window* parent);
  ~BarColorType() override;

  void setText() override;

protected:
  ColorBar* bars[MAX_BARS];
  lv_obj_t* barLabels[MAX_BARS];
  lv_obj_t* barValLabels[MAX_BARS];

  virtual const char** getLabelChars() { return nullptr; };

  lv_obj_t* create_bar_label(lv_obj_t* parent, lv_coord_t x, lv_coord_t y);
  lv_obj_t* create_bar_value_label(lv_obj_t* parent, lv_coord_t x, lv_coord_t y);
};

// Color editor for HSV color input
class HSVColorType : public BarColorType {
public:
  HSVColorType(Window* parent, uint32_t color);
  uint32_t getRGB() override;

protected:
  const char** getLabelChars() override;
};

// Color editor for RGB color input
class RGBColorType : public BarColorType {
public:
  RGBColorType(Window* parent, uint32_t color);
  uint32_t getRGB() override;

protected:
  const char** getLabelChars() override;
};

// Color editor that shows the system theme colors as buttons
class ThemeColorType : public ColorType {
public:
  ThemeColorType(Window* parent, uint32_t color);
  uint32_t getRGB() override;

protected:
  uint32_t m_color;
  void makeButton(Window* parent, uint32_t color);
  void makeButtonsRow(Window* parent, uint32_t c1, uint32_t c2, uint32_t c3);
  lv_style_t style;
};

enum COLOR_EDITOR_TYPE
{
  RGB_COLOR_EDITOR = 0,
  HSV_COLOR_EDITOR,
  THM_COLOR_EDITOR
};

// the ColorEditor() control is a group of other controls
class ColorEditor : public FormWindow
{
 public:
  ColorEditor(Window* parent, const rect_t& rect, uint32_t color,
              std::function<void(uint32_t rgb)> setValue = nullptr);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ColorEditor"; }
#endif

  void setColorEditorType(COLOR_EDITOR_TYPE colorType);

 protected:
  ColorType* _colorType = nullptr;
  std::function<void(uint32_t)> _setValue;
  uint32_t _color;

  void setText();
  void setHSV();
  void setRGB();
  static void value_changed(lv_event_t* e);
};
