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

// the content page of the ColorEditorPupup
class ColorEditorContent : public ModalWindowContent
{
  friend class ColorEditorPopup;

 public:
  ColorEditorContent(ModalWindow *window, const rect_t rect, uint32_t color,
                     std::function<void(uint32_t rgb)> setValue = nullptr);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ColorEditorContent"; }
#endif

#if defined(HARDWARE_TOUCH)
  bool onTouchEnd(coord_t x, coord_t y) override;
  bool onTouchStart(coord_t x, coord_t y) override;
#endif

  void onEvent(event_t event) override;
  void paint(BitmapBuffer *dc) override;

 protected:
  bool sliding = false;
  bool colorPicking = false;
  int r, g, b;
  int hue = 0, s, v;
  NumberEdit *rEdit;
  NumberEdit *gEdit;
  NumberEdit *bEdit;

  void drawHueBar(BitmapBuffer *dc);
  void drawGrid(BitmapBuffer *dc);
  void drawColorBox(BitmapBuffer *dc);
  void setRGB();
  std::function<void(uint32_t rgb)> setValue;
};

// a color editor popup
class ColorEditorPopup : public ModalWindow
{
 public:
  ColorEditorPopup(Window *window, std::function<uint32_t()> getValue,
                   std::function<void(uint32_t value)> setValue = nullptr);

  void deleteLater(bool detach = true, bool trash = true) override;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "ColorEditorPopup"; }
#endif

#if defined(HARDWARE_TOUCH)
  bool onTouchEnd(coord_t x, coord_t y) override;
#endif

 protected:
  uint32_t color;
  ColorEditorContent *content;
  std::function<uint32_t()> _getValue;
  std::function<void(uint32_t value)> _setValue;
};
