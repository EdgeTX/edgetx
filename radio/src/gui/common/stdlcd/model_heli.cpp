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

static int numberField(const char* name, coord_t y, int val, int min, LcdFlags attr, event_t event)
{
  return editNumberField(name, INDENT_WIDTH, MODEL_HELI_2ND_COLUMN, y, val, min, 100, attr, event);
}

static uint8_t sourceField(const char* name, coord_t y, uint8_t val, LcdFlags attr, event_t event)
{
  lcdDrawTextAlignedLeft(y, name);
  drawSource(MODEL_HELI_2ND_COLUMN, y, val, attr);
  if (attr) CHECK_INCDEC_MODELSOURCE(event, val, 0, MIXSRC_LAST_CH);
  return val;
}

void menuModelHeli(event_t event)
{
  SIMPLE_MENU(STR_MENUHELISETUP, menuTabModel, MENU_MODEL_HELI, HEADER_LINE+ITEM_HELI_MAX);

  uint8_t sub = menuVerticalPosition - HEADER_LINE;

  coord_t y = MENU_HEADER_HEIGHT + 1;

  for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
    uint8_t k = i + menuVerticalOffset;
    LcdFlags blink = (s_editMode > 0 ? BLINK|INVERS : INVERS);
    LcdFlags attr = (sub == k ? blink : 0);

    switch (k) {
      case ITEM_HELI_SWASHTYPE:
        g_model.swashR.type = editChoice(MODEL_HELI_2ND_COLUMN, y, STR_SWASHTYPE, STR_VSWASHTYPE, g_model.swashR.type, 0, SWASH_TYPE_MAX, attr, event);
        break;

      case ITEM_HELI_SWASHRING:
        g_model.swashR.value = numberField(STR_SWASHRING, y, g_model.swashR.value, 0, attr, event);
        break;

      case ITEM_HELI_ELE:
        g_model.swashR.elevatorSource = sourceField(STR_ELEVATOR, y, g_model.swashR.elevatorSource, attr, event);
        break;

      case ITEM_HELI_ELE_WEIGHT:
        g_model.swashR.elevatorWeight = numberField(STR_WEIGHT, y, g_model.swashR.elevatorWeight, -100, attr, event);
        break;

      case ITEM_HELI_AIL:
        g_model.swashR.aileronSource = sourceField(STR_AILERON, y, g_model.swashR.aileronSource, attr, event);
        break;

      case ITEM_HELI_AIL_WEIGHT:
        g_model.swashR.aileronWeight = numberField(STR_WEIGHT, y, g_model.swashR.aileronWeight, -100, attr, event);
        break;

      case ITEM_HELI_COL:
        g_model.swashR.collectiveSource = sourceField(STR_COLLECTIVE, y, g_model.swashR.collectiveSource, attr, event);
        break;

      case ITEM_HELI_COL_WEIGHT:
        g_model.swashR.collectiveWeight = numberField(STR_WEIGHT, y, g_model.swashR.collectiveWeight, -100, attr, event);
        break;
    }

    y += FH;
  }
}
