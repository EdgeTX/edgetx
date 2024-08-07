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

#include "edgetx.h"

enum MenuModelHeliItems {
  ITEM_HELI_SWASHTYPE,
  ITEM_HELI_SWASHRING,
  ITEM_HELI_ELE,
  ITEM_HELI_ELE_WEIGHT,
  ITEM_HELI_AIL,
  ITEM_HELI_AIL_WEIGHT,
  ITEM_HELI_COL,
  ITEM_HELI_COL_WEIGHT,
  ITEM_HELI_MAX
};

#if LCD_W >= 212
#define MODEL_HELI_2ND_COLUMN          (LCD_W-17*FW-MENUS_SCROLLBAR_WIDTH)
#else
#define MODEL_HELI_2ND_COLUMN          (14*FW)
#endif

void menuModelHeli(event_t event)
{
  SIMPLE_MENU(STR_MENUHELISETUP, menuTabModel, MENU_MODEL_HELI, HEADER_LINE+ITEM_HELI_MAX);

  uint8_t sub = menuVerticalPosition - HEADER_LINE;

  for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i + menuVerticalOffset;
    LcdFlags blink = (s_editMode > 0 ? BLINK|INVERS : INVERS);
    LcdFlags attr = (sub == k ? blink : 0);

    switch (k) {
      case ITEM_HELI_SWASHTYPE:
        g_model.swashR.type = editChoice(MODEL_HELI_2ND_COLUMN, y, STR_SWASHTYPE, STR_VSWASHTYPE, g_model.swashR.type, 0, SWASH_TYPE_MAX, attr, event);
        break;

      case ITEM_HELI_SWASHRING:
        lcdDrawTextAlignedLeft(y, STR_SWASHRING);
        lcdDrawNumber(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.value, LEFT|attr);
        if (attr) CHECK_INCDEC_MODELVAR_ZERO(event, g_model.swashR.value, 100);
        break;

      case ITEM_HELI_ELE:
        lcdDrawTextAlignedLeft(y, STR_ELEVATOR);
        drawSource(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.elevatorSource, attr);
        if (attr) CHECK_INCDEC_MODELSOURCE(event, g_model.swashR.elevatorSource, 0, MIXSRC_LAST_CH);
        break;

      case ITEM_HELI_ELE_WEIGHT:
        lcdDrawTextIndented(y, STR_WEIGHT);
        lcdDrawNumber(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.elevatorWeight, LEFT|attr);
        if (attr) CHECK_INCDEC_MODELVAR(event, g_model.swashR.elevatorWeight, -100, 100);
        break;

      case ITEM_HELI_AIL:
        lcdDrawTextAlignedLeft(y, STR_AILERON);
        drawSource(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.aileronSource, attr);
        if (attr) CHECK_INCDEC_MODELSOURCE(event, g_model.swashR.aileronSource, 0, MIXSRC_LAST_CH);
        break;

      case ITEM_HELI_AIL_WEIGHT:
        lcdDrawTextIndented(y, STR_WEIGHT);
        lcdDrawNumber(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.aileronWeight, LEFT|attr);
        if (attr) CHECK_INCDEC_MODELVAR(event, g_model.swashR.aileronWeight, -100, 100);
        break;

      case ITEM_HELI_COL:
        lcdDrawTextAlignedLeft(y, STR_COLLECTIVE);
        drawSource(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.collectiveSource, attr);
        if (attr) CHECK_INCDEC_MODELSOURCE(event, g_model.swashR.collectiveSource, 0, MIXSRC_LAST_CH);
        break;

      case ITEM_HELI_COL_WEIGHT:
        lcdDrawTextIndented(y, STR_WEIGHT);
        lcdDrawNumber(MODEL_HELI_2ND_COLUMN, y, g_model.swashR.collectiveWeight, LEFT|attr);
        if (attr) CHECK_INCDEC_MODELVAR(event, g_model.swashR.collectiveWeight, -100, 100);
        break;
    }
  }
}
