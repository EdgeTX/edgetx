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

#include "coloredit.h"
#include "numberedit.h"
#include "bitfield.h"

constexpr uint8_t PART_BITS[3][2] = { {0, 5}, {5, 6}, {11, 5} };

class ColorBox: public Window {
  public:
    ColorBox(Window * parent, const rect_t & rect, LcdFlags color):
      Window(parent, rect),
      color(color)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "ColorBox";
    }
#endif

    void paint(BitmapBuffer * dc) override
    {
      dc->drawSolidFilledRect(0, 0, width(), height(), COLOR_THEME_SECONDARY1);
      dc->drawSolidFilledRect(1, 1, width() - 2, height() - 2, color << 16);
    }

    void setColor(pixel_t value)
    {
      color = value;
      invalidate();
    }

  protected:
    pixel_t color;
};

ColorEdit::ColorEdit(FormGroup * parent, const rect_t & rect, std::function<uint16_t()> getValue, std::function<void(uint16_t)> setValue):
  FormGroup(parent, rect, FORWARD_SCROLL | FORM_FORWARD_FOCUS)
{
  auto width = rect.w / 4 - 5;

  // The color box
  auto box = new ColorBox(this, {0, 0, rect.w - 3 * width, rect.h}, getValue());

  // The 3 parts of RGB
  for (uint8_t part = 0; part < 3; part++) {
    new NumberEdit(this, {rect.w - (3 - part) * width + 5, 0, width - 5, rect.h}, 0, (1 << PART_BITS[part][1]) - 1,
                   [=]() {
                     return bfGet(getValue(), PART_BITS[part][0], PART_BITS[part][1]);
                   },
                   [=](uint16_t newValue) {
                     uint16_t value = getValue();
                     value = bfSet(value, newValue, PART_BITS[part][0], PART_BITS[part][1]);
                     setValue(value);
                     box->setColor(value);
                   });
  }
}
