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

#include "curve.h"
#include "page.h"
#include "window.h"

class NumberEdit;
class CurveEdit;
class CurveDataEdit;

class CurveEditWindow : public Page
{
 public:
  CurveEditWindow(uint8_t index,
                  std::function<void(void)> refreshView = nullptr);

  void setCurrentSource(mixsrc_t source);

  static LAYOUT_VAL(NUMEDT_W, 70, 70)
  static LAYOUT_VAL(CURVE_WIDTH, 215, 232)

 protected:
  uint8_t index;
  CurveEdit* curveEdit = nullptr;
  CurveDataEdit* curveDataEdit = nullptr;
  std::function<void(void)> refreshView = nullptr;

  void buildHeader(Window* window);
  void buildBody(Window* window);

  void onCancel() override;
};
