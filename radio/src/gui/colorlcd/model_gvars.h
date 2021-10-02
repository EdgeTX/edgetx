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

#include "tabsgroup.h"
#include "page.h"
#include "window.h"
#include "numberedit.h"

class GVarButton: public Button
{
  public:
    GVarButton(FormGroup * parent, const rect_t & rect, uint8_t gvar);

  protected:
    void checkEvents() override;
    void paint(BitmapBuffer * dc) override;

  protected:
    void drawFlightMode(BitmapBuffer * dc, coord_t x, coord_t y, int fm, LcdFlags attr);
    uint8_t gvarIdx;
    int lines;
    int32_t gvarSum = 0; // used for invalidation
    uint8_t currentFlightMode = 0; // used for invalidation
};

class ModelGVarsPage: public PageTab
{
  public:
    ModelGVarsPage():
      PageTab(STR_MENU_GLOBAL_VARS, ICON_MODEL_GVARS)
    {
    }

  protected:
    void build(FormWindow * window) override;
    void rebuild(FormWindow * window);
};

class GVarRenderer: public Window
{
  friend class GVarEditWindow;

  public:
    GVarRenderer(Window * window, rect_t rect, uint8_t gvarIndex) :
      Window(window, rect),
      index(gvarIndex)
    {
    }

  protected:
    void paint(BitmapBuffer * dc) override;
    void checkEvents() override;
    bool isUpdated();

  protected:
    uint8_t index;
    gvar_t lastGVar = 0;
    uint8_t lastFlightMode = 0;
    bool updated = false;
};


class GVarEditWindow: public Page
{
  public:
    explicit GVarEditWindow(uint8_t gvarIndex) :
      Page(ICON_MODEL_GVARS),
      index(gvarIndex)
    {
      buildHeader(&header);
      buildBody(&body);
    }

  protected:
    uint8_t index;
    NumberEdit * min = nullptr;
    NumberEdit * max = nullptr;
    NumberEdit * values[MAX_FLIGHT_MODES] = {};
    GVarRenderer * gVarInHeader = nullptr;
    void buildHeader(Window * window);
    void buildBody(FormWindow * window);
    void setProperties(int onlyForFlightMode = -1);
    void checkEvents() override;
};
