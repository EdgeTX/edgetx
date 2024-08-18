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

#include "input_edit_adv.h"

#include "fm_matrix.h"
#include "edgetx.h"
#include "etx_lv_theme.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

InputEditAdvanced::InputEditAdvanced(uint8_t input_n, uint8_t index) :
    Page(ICON_MODEL_INPUTS)
{
  std::string title2(getSourceString(MIXSRC_FIRST_INPUT + input_n));
  header->setTitle(STR_MENUINPUTS);
  header->setTitle2(title2);

  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);
  body->setFlexLayout();

  ExpoData* input = expoAddress(index);

  // Side
  auto line = body->newLine(grid);
  new StaticText(line, rect_t{}, STR_SIDE);
  new Choice(
      line, rect_t{}, STR_VCURVEFUNC, 1, 3,
      [=]() -> int16_t { return 4 - input->mode; },
      [=](int16_t newValue) {
        input->mode = 4 - newValue;
        SET_DIRTY();
      });

  // Trim
  line = body->newLine(grid);
  new StaticText(line, rect_t{}, STR_TRIM);
  const auto trimLast = TRIM_OFF + keysGetMaxTrims() - 1;
  auto c = new Choice(line, rect_t{}, -TRIM_OFF, trimLast,
                      GET_VALUE(-input->trimSource),
                      SET_VALUE(input->trimSource, -newValue));

  uint16_t srcRaw = input->srcRaw;
  c->setAvailableHandler([=](int value) {
    return value != TRIM_ON || srcRaw <= MIXSRC_LAST_STICK;
  });
  c->setTextHandler([=](int value) -> std::string {
    return getTrimSourceLabel(srcRaw, -value);
  });

  // Flight modes
  if (modelFMEnabled()) {
    line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_FLMODE);
    new FMMatrix<ExpoData>(line, rect_t{}, input);
  }
}
