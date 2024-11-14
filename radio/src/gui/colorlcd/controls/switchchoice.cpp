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
#include "menu.h"
#include "menutoolbar.h"
#include "edgetx.h"
#include "strhelpers.h"
#include "switches.h"

class SwitchChoiceMenuToolbar : public MenuToolbar
{
 public:
  SwitchChoiceMenuToolbar(SwitchChoice* choice, Menu* menu) :
      MenuToolbar(choice, menu, 2)
  {
    addButton(STR_CHAR_SWITCH, SWSRC_FIRST_SWITCH, SWSRC_LAST_MULTIPOS_SWITCH,
              nullptr, STR_MENU_SWITCHES);
    addButton(STR_CHAR_TRIM, SWSRC_FIRST_TRIM, SWSRC_LAST_TRIM, nullptr,
              STR_MENU_TRIMS);
    addButton("LS", SWSRC_FIRST_LOGICAL_SWITCH, SWSRC_LAST_LOGICAL_SWITCH,
              nullptr, STR_MENU_LOGICAL_SWITCHES);
    addButton("FM", SWSRC_FIRST_FLIGHT_MODE, SWSRC_LAST_FLIGHT_MODE, nullptr,
              STR_FLIGHT_MODE);
    addButton(STR_CHAR_TELEMETRY, SWSRC_FIRST_SENSOR, SWSRC_LAST_SENSOR,
              nullptr, STR_MENU_TELEMETRY);
#if defined(DEBUG_LATENCY)
    auto lastSource = SWSRC_LATENCY_TOGGLE;
#else
    auto lastSource = SWSRC_TRAINER_CONNECTED;
#endif
    addButton(
        STR_CHAR_FUNCTION, SWSRC_ON, lastSource,
        [=](int16_t index) {
          index = abs(index);
          return index == 0 || index == SWSRC_ON || index == SWSRC_ONE ||
                 index == SWSRC_TRAINER_CONNECTED ||
                 (index >= SWSRC_TELEMETRY_STREAMING && index <= lastSource &&
                  !(index >= SWSRC_FIRST_SENSOR && index <= SWSRC_LAST_SENSOR));
        },
        STR_MENU_OTHER);

    if ((nxtBtnPos > filterColumns) && choice->isValueAvailable &&
        choice->isValueAvailable(0))
      addButton(STR_SELECT_MENU_CLR, 0, 0, nullptr, nullptr, true);

    invertBtn = new MenuToolbarButton(this, {0, 0, LV_PCT(100), 0},
                                      STR_SELECT_MENU_INV);
    invertBtn->check(choice->inverted);
    lv_obj_align(invertBtn->getLvObj(), LV_ALIGN_BOTTOM_MID, 0, 0);

    invertBtn->setPressHandler([=]() {
      lv_obj_clear_state(invertBtn->getLvObj(), LV_STATE_FOCUSED);
      invertChoice();
      return choice->inverted;
    });
  }

  void invertChoice()
  {
    SwitchChoice* switchChoice = (SwitchChoice*)choice;
    switchChoice->inverted = !switchChoice->inverted;
    auto idx = menu->selection();
    switchChoice->fillMenu(menu, filter);
    menu->select(idx);
    invertBtn->check(switchChoice->inverted);
  }

  void longPress()
  {
    lv_indev_t* indev = lv_indev_get_act();
    if (indev->driver->type == LV_INDEV_TYPE_KEYPAD) {
      invertChoice();
    }
  }

 protected:
  MenuToolbarButton* invertBtn = nullptr;
};

bool SwitchChoice::onLongPress()
{
  int16_t val = _getValue();
  if (isValueAvailable && isValueAvailable(-val))
    setValue(-val);
  return true;
}

void SwitchChoice::setValue(int value)
{
  if (inMenu) {
    if (inverted) value = -value;
    inMenu = false;
  }
  Choice::setValue(value);
}

int SwitchChoice::getIntValue() const
{
  int value = Choice::getIntValue();
  if (inMenu) value = abs(value);
  return value;
}

void SwitchChoice::openMenu()
{
  setEditMode(true);  // this needs to be done first before menu is created.

  auto menu = new Menu();
  if (menuTitle) menu->setTitle(menuTitle);

  inverted = _getValue() < 0;
  inMenu = true;

  auto tb = new SwitchChoiceMenuToolbar(this, menu);
  menu->setToolbar(tb);

  menu->setLongPressHandler([=]() { tb->invertChoice(); });

#if defined(AUTOSWITCH)
  menu->setWaitHandler([=]() {
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
        tb->resetFilter();
        menu->select(getIndexFromValue(val));
      }
    }
  });
#endif

  // fillMenu(menu); - called by MenuToolbar

  menu->setCloseHandler([=]() { setEditMode(false); });
}

SwitchChoice::SwitchChoice(Window* parent, const rect_t& rect, int vmin,
                           int vmax, std::function<int16_t()> getValue,
                           std::function<void(int16_t)> setValue) :
    Choice(parent, rect, 0, vmax, getValue, setValue, STR_SWITCH)
{
  setTextHandler([=](int value) {
    if (inMenu && inverted) value = -value;

    if (isValueAvailable && !isValueAvailable(value))
      return std::to_string(0);  // we will fix this later

    return std::string(getSwitchPositionName(value));
  });

  setAvailableHandler(isSwitchAvailableInMixes);
}
