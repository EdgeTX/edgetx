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

#include "edgetx.h"
//#include "eeprom_common.h"

void eepromCheck()
{
}

void storageFormat()
{
}

bool eepromOpen()
{
  return true;
}

uint16_t eeLoadModelData(uint8_t index)
{
  modelDefault(index);
  return sizeof(g_model);
}

bool eeLoadGeneral()
{
  generalDefault();
  return true;
}

void eeLoadModelName(uint8_t id, char *name)
{
  memclear(name, sizeof(g_model.header.name));
}

bool eeModelExists(uint8_t id)
{
  return true;
}

void storageCheck(bool immediately)
{
}

void eeLoadModelHeader(uint8_t id, ModelHeader * header)
{
  memclear(header, sizeof(ModelHeader));
}

bool eeCopyModel(uint8_t dst, uint8_t src)
{
  return true;
}

void eeSwapModels(uint8_t id1, uint8_t id2)
{
}

void eeDeleteModel(uint8_t idx)
{
}
