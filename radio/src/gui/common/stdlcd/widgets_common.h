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

typedef int choice_t;

choice_t editChoice(coord_t x, coord_t y, const char *label,
                    const char *const *values, choice_t value, choice_t min,
                    choice_t max, LcdFlags attr, event_t event);
choice_t editChoice(coord_t x, coord_t y, const char *label,
                    const char *const *values, choice_t value, choice_t min,
                    choice_t max, LcdFlags attr, event_t event, coord_t lblX);
choice_t editChoice(coord_t x, coord_t y, const char *label,
                    const char *const *values, choice_t value, choice_t min,
                    choice_t max, LcdFlags attr, event_t event, coord_t lblX,
                    IsValueAvailable isValueAvailable);

uint8_t editCheckBox(uint8_t value, coord_t x, coord_t y, const char *label,
                     LcdFlags attr, event_t event);
uint8_t editCheckBox(uint8_t value, coord_t x, coord_t y, const char *label,
                     LcdFlags attr, event_t event, coord_t lblX);

swsrc_t editSwitch(coord_t x, coord_t y, swsrc_t value, LcdFlags attr,
                   event_t event);

uint16_t editSrcVarFieldValue(coord_t x, coord_t y, const char* title, uint16_t value,
                              int16_t min, int16_t max, LcdFlags attr, event_t event,
                              IsValueAvailable isValueAvailable, int16_t sourceMin, int16_t sourceMax);

int16_t editGVarFieldValue(coord_t x, coord_t y, int16_t value, int16_t min,
                           int16_t max, LcdFlags attr, uint8_t editflags,
                           event_t event);

#define GVAR_MENU_ITEM(x, y, v, min, max, attr, editflags, event) \
  editGVarFieldValue(x, y, v, min, max, attr, editflags, event)

#if defined(GVARS)
void editGVarValue(coord_t x, coord_t y, event_t event, uint8_t gvar,
                   uint8_t flightMode, LcdFlags flags);
#endif

void editCurveRef(coord_t x, coord_t y, CurveRef & curve, event_t event, LcdFlags flags,
                  IsValueAvailable isValueAvailable, int16_t sourceMin, int16_t sourceMax);

extern uint8_t editNameCursorPos;

void editName(coord_t x, coord_t y, char *name, uint8_t size, event_t event,
              uint8_t active, LcdFlags attr, uint8_t old_editMode);

void editSingleName(coord_t x, coord_t y, const char *label, char *name,
                    uint8_t size, event_t event, uint8_t active,
                    uint8_t old_editMode, coord_t lblX = 0);

uint8_t editDelay(coord_t y, event_t event, uint8_t attr, const char * str, uint8_t delay, uint8_t prec);

int editNumberField(const char* name, coord_t lx, coord_t vx, coord_t y, int val,
                    int min, int max, LcdFlags attr, event_t event, const char* zeroStr = nullptr, int ofst = 0);
