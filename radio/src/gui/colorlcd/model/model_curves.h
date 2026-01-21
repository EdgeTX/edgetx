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

#include "edgetx.h"
#include "pagegroup.h"

class ModelCurvesPage : public PageGroupItem
{
 public:
  ModelCurvesPage(const PageDef& pageDef);

  static void pushEditCurve(int index, std::function<void(void)> refreshView, mixsrc_t source);

  virtual void build(Window* window) override;

  static LAYOUT_ORIENTATION(PER_ROW, 3, 2)

 protected:
  uint8_t focusIndex = -1;
  ButtonBase* addButton = nullptr;

  void rebuild(Window* window);
  void editCurve(Window* window, uint8_t curve);
  void presetMenu(Window* window, uint8_t index);
  void plusPopup(Window* window);
  void newCV(Window* window, bool presetCV);
};
