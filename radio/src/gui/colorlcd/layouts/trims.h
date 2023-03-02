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

#include "libopenui.h"

class MainViewTrim : public Window
{
  public:
    MainViewTrim(Window * parent, const rect_t & rect, uint8_t idx);

    void checkEvents() override;
    void paint(BitmapBuffer * dc) override;

  protected:
    uint8_t idx;
    int value = 0;
    bool showChange = false;
    int trimMin = 0, trimMax = 0;

    void setRange();

    virtual coord_t sx() { return 0; }
    virtual coord_t sy() { return 0; }

    virtual void drawLine(BitmapBuffer * dc) = 0;
    virtual void drawMarkerLines(BitmapBuffer * dc, coord_t x, coord_t y) = 0;
    virtual void drawValue(BitmapBuffer * dc) = 0;
};

class MainViewHorizontalTrim : public MainViewTrim
{
  public:
    using MainViewTrim::MainViewTrim;
    MainViewHorizontalTrim(Window* parent, uint8_t idx);

  protected:
    coord_t sx() override;
    void drawLine(BitmapBuffer * dc) override;
    void drawMarkerLines(BitmapBuffer * dc, coord_t x, coord_t y) override;
    void drawValue(BitmapBuffer * dc) override;
};

class MainViewVerticalTrim : public MainViewTrim
{
  public:
    MainViewVerticalTrim(Window* parent, uint8_t idx);

  protected:
    coord_t sy() override;
    void drawLine(BitmapBuffer * dc) override;
    void drawMarkerLines(BitmapBuffer * dc, coord_t x, coord_t y) override;
    void drawValue(BitmapBuffer * dc) override;
};
