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

#include "menu.h"
#include "menutoolbar.h"
#include "edgetx.h"

uint16_t SwitchChoice::switchTypeCount(uint8_t type)
{
  switch (type) {
    case SWITCH_TYPE_NONE:
      return 1;
    case SWITCH_TYPE_SWITCH:
      return MAX_SWITCHES * 3;
    case SWITCH_TYPE_MULTIPOS:
      return MAX_XPOTS_POSITIONS;
    case SWITCH_TYPE_TRIM:
      return MAX_TRIMS * 2;
    case SWITCH_TYPE_LOGICAL:
      return MAX_LOGICAL_SWITCHES;
    case SWITCH_TYPE_ON:
      return 1;
    case SWITCH_TYPE_ONE:
      return 1;
    case SWITCH_TYPE_FLIGHT_MODE:
      return MAX_FLIGHT_MODES;
    case SWITCH_TYPE_TELEMETRY:
      return 1;
    case SWITCH_TYPE_SENSOR:
      return MAX_TELEMETRY_SENSORS;
    case SWITCH_TYPE_RADIO_ACTIVITY:
      return 1;
    case SWITCH_TYPE_TRAINER:
      return 1;
    default:
      return 0;
  }
}

int SwitchChoice::findEntry(SwitchRef ref) const
{
  // Strip inversion for lookup
  SwitchRef lookup = ref;
  lookup.flags &= ~SWITCH_FLAG_INVERTED;

  for (int i = 0; i < (int)entries.size(); i++) {
    if (entries[i].type == lookup.type && entries[i].index == lookup.index)
      return i;
  }
  return -1;
}

void SwitchChoice::buildEntries()
{
  entries.clear();
  typeGroups.clear();

  for (uint8_t t = SWITCH_TYPE_NONE; t < SWITCH_TYPE_LAST; t++) {
    uint16_t count = switchTypeCount(t);
    if (count == 0) continue;

    int groupStart = (int)entries.size();
    bool hasEntries = false;

    for (uint16_t i = 0; i < count; i++) {
      SwitchRef ref = {t, 0, i};

      // Check availability using swsrc_t-based isSwitchAvailableInMixes
      // as the default hardware filter (bridge to legacy code)
      swsrc_t swsrc = switchRefToSwSrc(ref);
      if (!isSwitchAvailableInMixes(swsrc)) continue;

      // Check custom SwitchRef-based filter
      if (isRefAvailable && !isRefAvailable(ref)) continue;

      entries.push_back(ref);
      hasEntries = true;
    }

    if (hasEntries) {
      typeGroups.push_back({t, groupStart, (int)entries.size() - 1});
    }
  }

  // Update Choice's vmin/vmax to match entry indices
  vmin = 0;
  vmax = entries.empty() ? 0 : (int)entries.size() - 1;
}

class SwitchChoiceMenuToolbar : public MenuToolbar
{
 public:
  SwitchChoiceMenuToolbar(SwitchChoice* choice, Menu* menu) :
      MenuToolbar(choice, menu, 2)
  {
    auto addTypeButton = [&](const char* picto, uint8_t type,
                             const char* title) {
      for (auto& g : choice->typeGroups) {
        if (g.type == type) {
          addButton(picto, g.startIdx, g.endIdx, nullptr, title);
          return;
        }
      }
    };

    // Combined filter for multiple types
    auto addMultiTypeButton = [&](const char* picto,
                                  std::initializer_list<uint8_t> types,
                                  const Choice::FilterFct& filterFunc,
                                  const char* title) {
      int minIdx = INT_MAX, maxIdx = -1;
      for (auto& g : choice->typeGroups) {
        for (auto t : types) {
          if (g.type == t) {
            if (g.startIdx < minIdx) minIdx = g.startIdx;
            if (g.endIdx > maxIdx) maxIdx = g.endIdx;
          }
        }
      }
      if (maxIdx >= 0) {
        addButton(picto, minIdx, maxIdx, filterFunc, title);
      }
    };

    // Switches + multipos
    {
      std::initializer_list<uint8_t> switchTypes = {
          SWITCH_TYPE_SWITCH,
          SWITCH_TYPE_MULTIPOS,
      };
      addMultiTypeButton(CHAR_SWITCH, switchTypes,
          [choice, switchTypes](int16_t index) {
            if (index < 0 || index >= (int)choice->entries.size()) return false;
            uint8_t t = choice->entries[index].type;
            for (auto st : switchTypes) {
              if (t == st) return true;
            }
            return false;
          },
          STR_MENU_SWITCHES);
    }

    addTypeButton(CHAR_TRIM, SWITCH_TYPE_TRIM, STR_MENU_TRIMS);

    addTypeButton("LS", SWITCH_TYPE_LOGICAL,
                  STR_MENU_LOGICAL_SWITCHES);

    addTypeButton("FM", SWITCH_TYPE_FLIGHT_MODE,
                  STR_FLIGHT_MODE);

    addTypeButton(CHAR_TELEMETRY, SWITCH_TYPE_SENSOR,
                  STR_MENU_TELEMETRY);

    // Other: ON, ONE, TELEMETRY_STREAMING, RADIO_ACTIVITY, TRAINER
    {
      std::initializer_list<uint8_t> otherTypes = {
          SWITCH_TYPE_ON,
          SWITCH_TYPE_ONE,
          SWITCH_TYPE_TELEMETRY,
          SWITCH_TYPE_RADIO_ACTIVITY,
          SWITCH_TYPE_TRAINER,
      };
      addMultiTypeButton(
          CHAR_FUNCTION, otherTypes,
          [choice, otherTypes](int16_t index) {
            if (index < 0 || index >= (int)choice->entries.size()) return false;
            uint8_t t = choice->entries[index].type;
            for (auto ot : otherTypes) {
              if (t == ot) return true;
            }
            return false;
          },
          STR_MENU_OTHER);
    }

    // "None" button (clear)
    for (auto& g : choice->typeGroups) {
      if (g.type == SWITCH_TYPE_NONE) {
        if ((nxtBtnPos > filterColumns))
          addButton(STR_SELECT_MENU_CLR, g.startIdx, g.endIdx, nullptr,
                    nullptr, true);
        break;
      }
    }

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
  SwitchRef ref = _getSwitchRef();
  if (ref.isInverted())
    ref.flags &= ~SWITCH_FLAG_INVERTED;
  else
    ref.flags |= SWITCH_FLAG_INVERTED;

  // Check availability of the inverted version via bridge
  swsrc_t swsrc = switchRefToSwSrc(ref);
  if (isSwitchAvailableInMixes(swsrc)) {
    _setSwitchRef(ref);
    update();
  }
  return true;
}

void SwitchChoice::setValue(int value)
{
  if (value >= 0 && value < (int)entries.size()) {
    SwitchRef ref = entries[value];
    if (inMenu && inverted) ref.flags |= SWITCH_FLAG_INVERTED;
    inMenu = false;
    _setSwitchRef(ref);
  }
  // Call ChoiceBase update path
  Choice::setValue(value);
}

int SwitchChoice::getIntValue() const
{
  SwitchRef ref = _getSwitchRef();
  return const_cast<SwitchChoice*>(this)->findEntry(ref);
}

void SwitchChoice::setAvailableHandler(std::function<bool(SwitchRef)> handler)
{
  isRefAvailable = std::move(handler);
  buildEntries();
}

void SwitchChoice::openMenu()
{
  setEditMode(true);  // this needs to be done first before menu is created.

  SwitchRef cur = _getSwitchRef();
  inverted = cur.isInverted();
  inMenu = true;

  auto menu = new Menu();
  if (menuTitle) menu->setTitle(menuTitle);

  auto tb = new SwitchChoiceMenuToolbar(this, menu);
  menu->setToolbar(tb);

  menu->setLongPressHandler([=]() { tb->invertChoice(); });

#if defined(AUTOSWITCH)
  menu->setWaitHandler([=]() {
    swsrc_t swtch = getMovedSwitch();
    if (swtch) {
      // Convert swsrc_t to SwitchRef and find in entries
      SwitchRef movedRef = swSrcToSwitchRef(swtch);
      // Strip inversion for lookup since menu handles inversion separately
      movedRef.flags &= ~SWITCH_FLAG_INVERTED;
      int idx = findEntry(movedRef);
      if (idx >= 0) {
        tb->resetFilter();
        menu->select(idx);
      }
    }
  });
#endif

  // fillMenu(menu); - called by MenuToolbar

  menu->setCloseHandler([=]() { setEditMode(false); });
}

SwitchChoice::SwitchChoice(Window* parent, const rect_t& rect,
                           std::function<SwitchRef()> getValue,
                           std::function<void(SwitchRef)> setValue) :
    Choice(parent, rect, 0, 0,
           nullptr,  // getValue - set below
           nullptr,  // setValue - set below
           STR_SWITCH),
    _getSwitchRef(std::move(getValue)),
    _setSwitchRef(std::move(setValue))
{
  // Build the entries vector
  buildEntries();

  // Set up Choice's integer getter: find current SwitchRef in entries
  _getValue = [this]() -> int {
    return getIntValue();
  };

  // Set up Choice's integer setter
  _setValue = [this](int value) {
    if (value >= 0 && value < (int)entries.size()) {
      SwitchRef ref = entries[value];
      if (inMenu && inverted) ref.flags |= SWITCH_FLAG_INVERTED;
      inMenu = false;
      _setSwitchRef(ref);
    }
  };

  setTextHandler([=](int value) -> std::string {
    if (value < 0 || value >= (int)entries.size()) {
      // Current value not in entries - show raw switch string
      SwitchRef ref = _getSwitchRef();
      return std::string(getSwitchPositionName(ref));
    }

    SwitchRef ref = entries[value];
    if (inMenu && inverted) ref.flags |= SWITCH_FLAG_INVERTED;
    else if (!inMenu) {
      // Outside menu, show the actual stored value (may be inverted)
      ref = _getSwitchRef();
    }
    return std::string(getSwitchPositionName(ref));
  });

  // isValueAvailable always true - pre-filtered via buildEntries
  Choice::setAvailableHandler([](int) { return true; });
}
