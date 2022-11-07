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
#include "switchchoice.h"

#include "dataconstants.h"
#include "draw_functions.h"
#include "libopenui_config.h"
#include "menu.h"
#include "menutoolbar.h"
#include "opentx.h"
#include "strhelpers.h"
#include "switches.h"

class SwitchChoiceMenuToolbar : public MenuToolbar
{
 public:
  SwitchChoiceMenuToolbar(SwitchChoice* choice, Menu* menu) :
      MenuToolbar(choice, menu)
  {
    addButton(STR_CHAR_SWITCH, SWSRC_FIRST_SWITCH, SWSRC_LAST_SWITCH);
    addButton(STR_CHAR_TRIM, SWSRC_FIRST_TRIM, SWSRC_LAST_TRIM);
    addButton(STR_CHAR_SWITCH, SWSRC_FIRST_LOGICAL_SWITCH,
              SWSRC_LAST_LOGICAL_SWITCH);
  }
};

void SwitchChoice::LongPressHandler(void* data)
{
  SwitchChoice* swch = (SwitchChoice*)data;
  if (!swch) return;
  int16_t val = swch->_getValue();
  if (swch->isValueAvailable && swch->isValueAvailable(-val)) {
    swch->setValue(-val);
    swch->invalidate();
  }
}

SwitchChoice::SwitchChoice(Window* parent, const rect_t& rect, int vmin,
                           int vmax, std::function<int16_t()> getValue,
                           std::function<void(int16_t)> setValue) :
    ChoiceEx(parent, rect, vmin, vmax, getValue, setValue)
{
  setBeforeDisplayMenuHandler([=](Menu* menu) {
    auto tb = new SwitchChoiceMenuToolbar(this, menu);
    menu->setToolbar(tb);

#if defined(AUTOSWITCH)
    menu->setWaitHandler([menu, this, setValue, tb]() {
      swsrc_t val = 0;
      swsrc_t swtch = getMovedSwitch();
      if (swtch) {
        div_t info = switchInfo(swtch);
        if (IS_CONFIG_TOGGLE(info.quot)) {
          if (info.rem != 0) {
            val = (val == swtch ? swtch - 2 : swtch);
          }
        } else {
          val = swtch;
        }
        if (val && (!isValueAvailable || isValueAvailable(val))) {
          // if (filtered) fillMenu(menu);
          tb->resetFilter();
          menu->select(getIndexFromValue(val));
        }
      }
    });
#endif
  });

  setTextHandler([=](int value) {
    if (isValueAvailable && !isValueAvailable(value))
      return std::to_string(0);  // we will fix this later

    return std::string(getSwitchPositionName(value));
  });

  set_lv_LongPressHandler(LongPressHandler, this);

  setAvailableHandler(isSwitchAvailableInMixes);
}
