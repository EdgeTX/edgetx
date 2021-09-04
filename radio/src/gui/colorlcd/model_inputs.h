/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _MODEL_INPUTS_H
#define _MODEL_INPUTS_H

#include "tabsgroup.h"

class ModelInputsPage: public PageTab {
  public:
    ModelInputsPage();

    void build(FormWindow * window) override
    {
      build(window, 0);
    }

  protected:
    void build(FormWindow * window, int8_t focusIndex);
    void rebuild(FormWindow * window, int8_t focusIndex);
    void editInput(FormWindow * window, uint8_t channel, uint8_t index);
    uint8_t s_copySrcIdx;
};

class CommonInputOrMixButton : public Button {
  public:
    CommonInputOrMixButton(FormGroup * parent, const rect_t & rect, uint8_t index):
      Button(parent, rect, nullptr, 0, COLOR_THEME_PRIMARY1),
      index(index)
    {
      setFocusHandler([=](bool active) {
        if (active) {
          bringToTop();
        }
      });
    }

    virtual bool isActive() const = 0;

    void checkEvents() override;

    void drawFlightModes(BitmapBuffer *dc, FlightModesType value, LcdFlags textColor);

    void paint(BitmapBuffer * dc) override;

    virtual void paintBody(BitmapBuffer * dc) = 0;

  protected:
    uint8_t index;
    bool active = false;
};


#endif //_MODEL_INPUTS_H
