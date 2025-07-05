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

#include "page.h"

class StaticText;

class RadioCalibrationPage: public Page {
  public:
    explicit RadioCalibrationPage();

    void checkEvents() override;
    void onCancel() override;

  protected:
    StaticText* title2 = nullptr;
    TextButton* nxtBtn = nullptr;
    TextButton* axisBtn = nullptr;
    TextButton* potsBtn = nullptr;
    void nextStep();
    void buildHeader(Window * window);
    void buildBody(Window * window);
    void setState();

    static LAYOUT_VAL_SCALED(NXT_W, 80);
    static constexpr coord_t NXT_X = LCD_W - NXT_W - PAD_LARGE * 4;
    static LAYOUT_VAL_SCALED(AXIS_W, 80);
    static constexpr coord_t AXIS_X = PAD_LARGE * 4;
    static LAYOUT_VAL_SCALED(POTS_W, 80);
    static constexpr coord_t POTS_X = AXIS_X + AXIS_W + PAD_LARGE;
};

void startCalibration();
