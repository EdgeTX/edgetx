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

class SwitchChoiceMenuToolbar : public MenuToolbar<SwitchChoice>
{
 public:
  SwitchChoiceMenuToolbar(SwitchChoice* choice, Menu* menu) :
      MenuToolbar<SwitchChoice>(choice, menu)
  {
    addButton(CHAR_SWITCH, SWSRC_FIRST_SWITCH, SWSRC_LAST_SWITCH);
    addButton(CHAR_TRIM, SWSRC_FIRST_TRIM, SWSRC_LAST_TRIM);
    addButton(CHAR_SWITCH, SWSRC_FIRST_LOGICAL_SWITCH,
              SWSRC_LAST_LOGICAL_SWITCH);
  }
};

SwitchChoice::SwitchChoice(FormGroup* parent, const rect_t& rect, int vmin,
                           int vmax, std::function<int16_t()> getValue,
                           std::function<void(int16_t)> setValue) :
    ChoiceEx(parent, rect, vmin, vmax, getValue, setValue)
{
  setBeforeDisplayMenuHandler([=](Menu* menu) {
    menu->setToolbar(new SwitchChoiceMenuToolbar(this, menu));
#if defined(AUTOSWITCH)
    menu->setWaitHandler([menu, this, setValue]() {
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
          if (setValue) setValue(val);
          this->fillMenu(menu);
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

  setLongPressHandler([=](event_t) {
    int16_t val = getValue();
    if (isValueAvailable && isValueAvailable(-val)) {
      setValue(-val);
      invalidate();
    }
  });

  setAvailableHandler(isSwitchAvailableInMixes);
}

void SwitchChoice::fillMenu(Menu* menu, std::function<bool(int16_t)> filter)
{
  auto value = getValue();
  int count = 0;
  int current = 0;

  menu->removeLines();

  for (int i = vmin; i <= vmax; ++i) {
    if (filter && !filter(i)) continue;
    if (isValueAvailable && !isValueAvailable(i)) continue;

    menu->addLine(getSwitchPositionName(i), [=]() { setValue(i); });
    if (value == i) {
      current = count;
    }
    ++count;
  }

  if (current >= 0) {
    menu->select(current);
  }
}
