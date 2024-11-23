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

#include "sourcechoice.h"

#include "dataconstants.h"
#include "lcd.h"
#include "menu.h"
#include "menutoolbar.h"
#include "edgetx.h"
#include "strhelpers.h"
#include "switches.h"

class SourceChoiceMenuToolbar : public MenuToolbar
{
 public:
  SourceChoiceMenuToolbar(SourceChoice* choice, Menu* menu) :
      MenuToolbar(choice, menu, FILTER_COLUMNS)
  {
    addButton(STR_CHAR_INPUT, MIXSRC_FIRST_INPUT, MIXSRC_LAST_INPUT, nullptr,
              STR_MENU_INPUTS);
#if defined(LUA_MODEL_SCRIPTS)
    if (modelCustomScriptsEnabled())
      addButton(STR_CHAR_LUA, MIXSRC_FIRST_LUA, MIXSRC_LAST_LUA, nullptr,
                STR_MENU_LUA);
#endif
#if defined(PCBHORUS)
    auto lastSource = MIXSRC_LAST_SPACEMOUSE;
#elif defined(IMU)
    auto lastSource = MIXSRC_TILT_Y;
#else
    auto lastSource = MIXSRC_LAST_STICK;
#endif
    addButton(
        STR_CHAR_STICK, MIXSRC_FIRST_STICK, lastSource,
        [=](int16_t index) {
          if (index >= MIXSRC_FIRST_POT && index <= MIXSRC_LAST_POT)
            return false;
#if MAX_AXIS > 0
          if (index >= MIXSRC_FIRST_AXIS && index <= MIXSRC_LAST_AXIS)
            return false;
#endif
          return index >= MIXSRC_FIRST_STICK && index <= lastSource;
        },
        STR_MENU_STICKS);
    addButton(STR_CHAR_POT, MIXSRC_FIRST_POT, MIXSRC_LAST_POT, nullptr,
              STR_MENU_POTS);
    addButton(
        STR_CHAR_FUNCTION, MIXSRC_MIN, MIXSRC_LAST_TIMER,
        [=](int16_t index) {
          return (index >= MIXSRC_MIN && index <= MIXSRC_MAX) ||
                 (index >= MIXSRC_TX_VOLTAGE && index <= MIXSRC_LAST_TIMER);
        },
        STR_MENU_OTHER);
#if defined(HELI)
    if (modelHeliEnabled())
      addButton(STR_CHAR_CYC, MIXSRC_FIRST_HELI, MIXSRC_LAST_HELI, nullptr,
                STR_MENU_HELI);
#endif
    addButton(STR_CHAR_TRIM, MIXSRC_FIRST_TRIM, MIXSRC_LAST_TRIM, nullptr,
              STR_MENU_TRIMS);
#if defined(FUNCTION_SWITCHES)
    addButton(STR_CHAR_SWITCH, MIXSRC_FIRST_SWITCH, MIXSRC_LAST_CUSTOMSWITCH_GROUP, nullptr,
              STR_MENU_SWITCHES);
#else
    addButton(STR_CHAR_SWITCH, MIXSRC_FIRST_SWITCH, MIXSRC_LAST_SWITCH, nullptr,
              STR_MENU_SWITCHES);
#endif
    if (modelLSEnabled())
      addButton("LS", MIXSRC_FIRST_LOGICAL_SWITCH, MIXSRC_LAST_LOGICAL_SWITCH,
                nullptr, STR_MENU_LOGICAL_SWITCHES);
    addButton(STR_CHAR_TRAINER, MIXSRC_FIRST_TRAINER, MIXSRC_LAST_TRAINER,
              nullptr, STR_MENU_TRAINER);
    addButton(STR_CHAR_CHANNEL, MIXSRC_FIRST_CH, MIXSRC_LAST_CH, nullptr,
              STR_MENU_CHANNELS);
#if defined(GVARS)
    if (modelGVEnabled())
      addButton(STR_CHAR_SLIDER, MIXSRC_FIRST_GVAR, MIXSRC_LAST_GVAR, nullptr,
                STR_MENU_GVARS);
#endif
    if (modelTelemetryEnabled())
      addButton(STR_CHAR_TELEMETRY, MIXSRC_FIRST_TELEM, MIXSRC_LAST_TELEM,
                nullptr, STR_MENU_TELEMETRY);

    if ((nxtBtnPos > filterColumns) && choice->isValueAvailable &&
        choice->isValueAvailable(0))
      addButton(STR_SELECT_MENU_CLR, 0, 0, nullptr, nullptr, true);

    if (choice->canInvert) {
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
  }

  void invertChoice()
  {
    SourceChoice* sourceChoice = (SourceChoice*)choice;
    if (sourceChoice->canInvert) {
      sourceChoice->inverted = !sourceChoice->inverted;
      auto idx = menu->selection();
      sourceChoice->fillMenu(menu, filter);
      menu->select(idx);
      invertBtn->check(sourceChoice->inverted);
    }
  }

  void longPress()
  {
    lv_indev_t* indev = lv_indev_get_act();
    if (indev->driver->type == LV_INDEV_TYPE_KEYPAD) {
      invertChoice();
    }
  }

  static LAYOUT_VAL(FILTER_COLUMNS, 3, 2)

 protected:
  MenuToolbarButton* invertBtn = nullptr;
};

bool SourceChoice::onLongPress()
{
  if (canInvert) {
    int16_t val = _getValue();
    if (isValueAvailable && isValueAvailable(-val))
      setValue(-val);
  }
  return true;
}

void SourceChoice::setValue(int value)
{
  if (inMenu) {
    if (inverted) value = -value;
    inMenu = false;
  }
  Choice::setValue(value);
}

int SourceChoice::getIntValue() const
{
  int value = Choice::getIntValue();
  if (inMenu) value = abs(value);
  return value;
}

// defined in gui/gui_common.cpp
uint8_t switchToMix(uint8_t source);

void SourceChoice::openMenu()
{
  setEditMode(true);  // this needs to be done first before menu is created.

  inverted = getIntValue() < 0;
  inMenu = true;

  auto menu = new Menu();
  if (menuTitle) menu->setTitle(menuTitle);

  auto tb = new SourceChoiceMenuToolbar(this, menu);
  menu->setToolbar(tb);

  if (canInvert)
    menu->setLongPressHandler([=]() { tb->invertChoice(); });

#if defined(AUTOSOURCE)
  menu->setWaitHandler([=]() {
    int16_t val = getMovedSource(vmin);
    if (val) {
      tb->resetFilter();
      menu->select(getIndexFromValue(val));
    }
#if defined(AUTOSWITCH)
    else {
      swsrc_t swtch = abs(getMovedSwitch());
      if (swtch && !IS_SWITCH_MULTIPOS(swtch)) {
        val = switchToMix(swtch);
        if (val && (val >= vmin) && (val <= vmax)) {
          tb->resetFilter();
          menu->select(getIndexFromValue(val));
        }
      }
    }
#endif
  });
#endif

  // fillMenu(menu); - called by MenuToolbar

  menu->setCloseHandler([=]() { setEditMode(false); });
}

SourceChoice::SourceChoice(Window *parent, const rect_t &rect, int16_t vmin,
                           int16_t vmax, std::function<int16_t()> getValue,
                           std::function<void(int16_t)> setValue, bool allowInvert) :
    Choice(parent, rect, vmin, vmax, getValue, setValue, STR_SOURCE), canInvert(allowInvert)
{
  setTextHandler([=](int value) {
    if (inMenu && inverted) value = -value;

    if (isValueAvailable && !isValueAvailable(value))
      return std::to_string(0);  // we will fix this later

    return std::string(getSourceString(value));
  });

  setAvailableHandler([](int v) { return isSourceAvailable(v); });
}
