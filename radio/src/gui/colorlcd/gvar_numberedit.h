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

#ifndef _GVAR_NUMBEREDIT_H_
#define _GVAR_NUMBEREDIT_H_

#include "form.h"
#include "choice.h"
#include "numberedit.h"
#include "gvars.h"

constexpr coord_t GVAR_BUTTON_WIDTH = 30;


class GVarNumberEdit : public FormGroup
{
 public:
  GVarNumberEdit(FormGroup* parent, const rect_t& rect, int32_t vmin,
                 int32_t vmax, std::function<int32_t()> getValue,
                 std::function<void(int32_t)> setValue,
                 WindowFlags windowFlags = 0, LcdFlags textFlags = 0,
                 int32_t voffset = 0) :
      FormGroup(parent, rect, FORWARD_SCROLL | FORM_FORWARD_FOCUS),
      vmin(vmin),
      vmax(vmax),
      voffset(voffset),
      getValue(getValue),
      setValue(setValue),
      windowFlags(windowFlags),
      textFlags(textFlags)
  {
    update();
  }

  void switchGVarMode()
  {
    int32_t value = getValue();
    setValue(
        GV_IS_GV_VALUE(value, vmin, vmax)
            ? ((textFlags & PREC1)
                   ? GET_GVAR_PREC1(value, vmin, vmax, mixerCurrentFlightMode)
                   : GET_GVAR(value, vmin, vmax, mixerCurrentFlightMode))
            : GV_GET_GV1_VALUE(vmin, vmax));

    update();
  }

  void setSuffix(std::string value)
  {
    suffix = std::move(value);
    update();
  }

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override
  {
    if (event == EVT_KEY_LONG(KEY_ENTER)) {
      killEvents(event);
      switchGVarMode();
      field->setFocus(SET_FOCUS_DEFAULT);
    } else {
      FormGroup::onEvent(event);
    }
  }
#endif

 protected:
  FormField* field = nullptr;
  int32_t vmin;
  int32_t vmax;
  int32_t voffset;
  std::function<int32_t()> getValue;
  std::function<void(int32_t)> setValue;
  WindowFlags windowFlags;
  LcdFlags textFlags;
  std::string suffix;

  void update()
  {
    clear();

    // The main field
    int32_t value = getValue();
    if (GV_IS_GV_VALUE(value, vmin, vmax)) {
      auto result = new Choice(
          this, {0, 0, width() - GVAR_BUTTON_WIDTH, height()}, -MAX_GVARS,
          MAX_GVARS - 1,
          [=]() {
            uint16_t gvar1 = GV_GET_GV1_VALUE(vmin, vmax);
            return GV_INDEX_CALC_DELTA(getValue(), gvar1);
          },
          [=](int idx) {
            uint16_t gvar1 = GV_GET_GV1_VALUE(vmin, vmax);
            if (idx < 0)
              setValue(GV_CALC_VALUE_IDX_NEG(idx, gvar1));
            else
              setValue(GV_CALC_VALUE_IDX_POS(idx, gvar1));
          });
      result->setTextHandler(
          [=](int32_t value) { return getGVarString(value); });
      field = result;
    } else {
      auto result = new NumberEdit(
          this, {0, 0, width() - GVAR_BUTTON_WIDTH, height()}, vmin, vmax,
          [=]() { return getValue() + voffset; },
          [=](int32_t newValue) { return setValue(newValue - voffset); },
          windowFlags, textFlags);
      result->setSuffix(suffix);
      field = result;
    }
#if defined(GVARS)
    // The GVAR button
    new TextButton(
        this, {width() - GVAR_BUTTON_WIDTH, 0, GVAR_BUTTON_WIDTH, height()},
        STR_GV,
        [=]() {
          switchGVarMode();
          return 0;
        },
        BUTTON_BACKGROUND | OPAQUE | FORM_DETACHED);
#endif        
  }
};


#endif // _GVAR_NUMBEREDIT_H_
