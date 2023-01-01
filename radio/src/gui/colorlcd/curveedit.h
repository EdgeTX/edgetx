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
#include "form.h"
#include "curve.h"

class CurveDataEdit;

class CurveEdit: public FormField
{
  public:
    CurveEdit(Window * parent, const rect_t & rect, uint8_t index);
    static void SetCurrentSource(uint32_t source);

    void deleteLater(bool detach = true, bool trash = true) override
    {
      if (_deleted)
        return;

      preview.deleteLater(true, false);

      FormField::deleteLater(detach, trash);
    }

    void updatePreview();

    void checkEvents(void) override;

    void showCurrent(bool state) { showEdit = state; }

    uint8_t getCurrent() const { return current; }
    uint8_t getCurvePointsCount() const;
    int8_t getX(uint8_t n) const;
    int8_t getX() const { return getX(current); }
    int8_t getY(uint8_t n) const;
    int8_t getY() const { return getY(current); }
    bool isCustomCurve() const;
    bool isEditableX() const;

    bool setCurrent(uint8_t n);
    void selectPoint(int8_t chg);
    void setX(int8_t chg);
    void setY(int8_t chg);
    void setX(uint8_t n, int8_t value);
    void setY(uint8_t n, int8_t value);

   protected:
    Curve preview;
    uint8_t index;
    uint8_t current;
    bool showEdit;
    static mixsrc_t currentSource;
    static bool lockSource;
};

class CurveEditWindow : public Page
{
  public:
    CurveEditWindow(uint8_t index);

  protected:
    uint8_t index;
    CurveEdit * curveEdit = nullptr;
    CurveDataEdit * curveDataEdit = nullptr;

    void buildHeader(Window * window);
    void buildBody(FormWindow * window);
};
