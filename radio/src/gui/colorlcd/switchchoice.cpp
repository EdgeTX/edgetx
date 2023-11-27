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
#include "menu.h"
#include "menutoolbar.h"
#include "opentx.h"
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

#if defined(HARDWARE_TOUCH)
    coord_t y =
        height() - MENUS_TOOLBAR_BUTTON_WIDTH - MENUS_TOOLBAR_BUTTON_PADDING;
    coord_t w = width() - MENUS_TOOLBAR_BUTTON_PADDING * 2;

    invertBtn = new MenuToolbarButton(
        this, {MENUS_TOOLBAR_BUTTON_PADDING, y, w, MENUS_TOOLBAR_BUTTON_WIDTH},
        STR_SELECT_MENU_INV);
    invertBtn->check(choice->inverted);

    invertBtn->setPressHandler([=]() {
      lv_obj_clear_state(invertBtn->getLvObj(), LV_STATE_FOCUSED);
      invertChoice();
      return choice->inverted;
    });
#endif
  }

  void invertChoice()
  {
    SwitchChoice* switchChoice = (SwitchChoice*)choice;
    switchChoice->inverted = !switchChoice->inverted;
    auto idx = menu->selection();
    switchChoice->fillMenu(menu, filter);
    menu->select(idx);
#if defined(HARDWARE_TOUCH)
    invertBtn->check(switchChoice->inverted);
#endif
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

SwitchChoice::SwitchChoice(Window* parent, const rect_t& rect, int vmin,
                           int vmax, std::function<int16_t()> getValue,
                           std::function<void(int16_t)> setValue) :
    Choice(parent, rect, 0, vmax, getValue, setValue)
{
  setMenuTitle(STR_SWITCH);

  setBeforeDisplayMenuHandler([=](Menu* menu) {
    inverted = getValue() < 0;
    inMenu = true;

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
          tb->resetFilter();
          menu->select(getIndexFromValue(val));
        }
      }
    });
#endif
  });

  setTextHandler([=](int value) {
    if (inMenu && inverted) value = -value;

    if (isValueAvailable && !isValueAvailable(value))
      return std::to_string(0);  // we will fix this later

    return std::string(getSwitchPositionName(value));
  });

  set_lv_LongPressHandler(LongPressHandler, this);

  setAvailableHandler(isSwitchAvailableInMixes);
}
