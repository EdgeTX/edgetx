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

#include "window.h"
#include "messaging.h"

class ColorType;

constexpr int MAX_BARS = 3;

enum COLOR_EDITOR_TYPE {
  RGB_COLOR_EDITOR = 0,
  HSV_COLOR_EDITOR,
  THM_COLOR_EDITOR,
  FXD_COLOR_EDITOR,
};

// the ColorEditor() control is a group of other controls
class ColorEditor : public Window
{
 public:
  ColorEditor(Window* parent, const rect_t& rect, uint32_t color,
              std::function<void(uint32_t rgb)> setValue = nullptr,
              COLOR_EDITOR_FMT fmt = ETX_RGB565, COLOR_EDITOR_TYPE typ = HSV_COLOR_EDITOR);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ColorEditor"; }
#endif

  void setColorEditorType(COLOR_EDITOR_TYPE colorType);

  static LAYOUT_VAL_SCALED(BAR_MARGIN, 5)
  static LAYOUT_VAL_SCALED(BAR_HEIGHT_OFFSET, 25)
  static LAYOUT_VAL_SCALED(LBL_YO, 9)
  static LAYOUT_VAL_SCALED(VAL_XO, 10)
  static LAYOUT_VAL_SCALED_EVEN(CRSR_SZ, 10)

 protected:
  ColorType* _colorType = nullptr;
  std::function<void(uint32_t)> _setValue;
  uint32_t _color;
  COLOR_EDITOR_FMT format;
  Messaging colorUpdateMsg;

  void setText();
  void setRGB();
};
