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

#include "menu.h"
#include "menutoolbar.h"
#include "edgetx.h"

uint16_t SourceChoice::sourceTypeCount(uint8_t type)
{
  switch (type) {
    case SOURCE_TYPE_NONE:
      return 1;
    case SOURCE_TYPE_INPUT:
      return MAX_INPUTS;
#if defined(LUA_INPUTS)
    case SOURCE_TYPE_LUA:
      return MAX_SCRIPTS * MAX_SCRIPT_OUTPUTS;
#endif
    case SOURCE_TYPE_STICK:
      return MAX_STICKS;
    case SOURCE_TYPE_POT:
      return MAX_POTS;
#if defined(IMU)
    case SOURCE_TYPE_IMU:
      return 2;
#endif
#if defined(PCBHORUS)
    case SOURCE_TYPE_SPACEMOUSE:
      return 6;
#endif
    case SOURCE_TYPE_MIN:
      return 1;
    case SOURCE_TYPE_MAX:
      return 1;
#if defined(HELI)
    case SOURCE_TYPE_HELI:
      return 3;
#endif
    case SOURCE_TYPE_TRIM:
      return MAX_TRIMS;
    case SOURCE_TYPE_SWITCH:
      return MAX_SWITCHES;
#if defined(FUNCTION_SWITCHES)
    case SOURCE_TYPE_CUSTOM_SWITCH_GROUP:
      return NUM_FUNCTIONS_GROUPS;
#endif
    case SOURCE_TYPE_LOGICAL_SWITCH:
      return MAX_LOGICAL_SWITCHES;
    case SOURCE_TYPE_TRAINER:
      return MAX_TRAINER_CHANNELS;
    case SOURCE_TYPE_CHANNEL:
      return MAX_OUTPUT_CHANNELS;
#if defined(GVARS)
    case SOURCE_TYPE_GVAR:
      return MAX_GVARS;
#endif
    case SOURCE_TYPE_TX_VOLTAGE:
      return 1;
    case SOURCE_TYPE_TX_TIME:
      return 1;
    case SOURCE_TYPE_TX_GPS:
      return 1;
    case SOURCE_TYPE_TIMER:
      return MAX_TIMERS;
    case SOURCE_TYPE_TELEMETRY:
      return 3 * MAX_TELEMETRY_SENSORS;
#if defined(LUMINOSITY_SENSOR)
    case SOURCE_TYPE_LIGHT:
      return 1;
#endif
    default:
      return 0;
  }
}

int SourceChoice::findEntry(SourceRef ref) const
{
  // Strip inversion for lookup
  SourceRef lookup = ref;
  lookup.flags &= ~SOURCE_FLAG_INVERTED;

  for (int i = 0; i < (int)entries.size(); i++) {
    if (entries[i].type == lookup.type && entries[i].index == lookup.index)
      return i;
  }
  return -1;
}

void SourceChoice::buildEntries()
{
  entries.clear();
  typeGroups.clear();

  for (uint8_t t = SOURCE_TYPE_NONE; t < SOURCE_TYPE_LAST; t++) {
    uint16_t count = sourceTypeCount(t);
    if (count == 0) continue;

    int groupStart = (int)entries.size();
    bool hasEntries = false;

    for (uint16_t i = 0; i < count; i++) {
      SourceRef ref = {t, 0, i};

      // Check availability
      if (!isSourceAvailable(ref)) continue;

      // Check custom SourceRef-based filter
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

class SourceChoiceMenuToolbar : public MenuToolbar
{
 public:
  SourceChoiceMenuToolbar(SourceChoice* choice, Menu* menu) :
      MenuToolbar(choice, menu, FILTER_COLUMNS)
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
      // Find the overall min/max index range covering these types
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

    addTypeButton(CHAR_INPUT, SOURCE_TYPE_INPUT, STR_MENU_INPUTS);

#if defined(LUA_MODEL_SCRIPTS)
    if (modelCustomScriptsEnabled())
      addTypeButton(CHAR_LUA, SOURCE_TYPE_LUA, STR_MENU_LUA);
#endif

    // Sticks button: sticks but NOT pots (pots have their own button)
    addMultiTypeButton(
        CHAR_STICK,
        {SOURCE_TYPE_STICK,
#if defined(IMU)
         SOURCE_TYPE_IMU,
#endif
#if defined(PCBHORUS)
         SOURCE_TYPE_SPACEMOUSE,
#endif
        },
        [choice](int16_t index) {
          if (index < 0 || index >= (int)choice->entries.size()) return false;
          uint8_t t = choice->entries[index].type;
          return t == SOURCE_TYPE_STICK
#if defined(IMU)
                 || t == SOURCE_TYPE_IMU
#endif
#if defined(PCBHORUS)
                 || t == SOURCE_TYPE_SPACEMOUSE
#endif
              ;
        },
        STR_MENU_STICKS);

    addTypeButton(CHAR_POT, SOURCE_TYPE_POT, STR_MENU_POTS);

    // Other: MIN, MAX, TX_VOLTAGE, TX_TIME, TX_GPS, TIMER, LIGHT
    {
      std::initializer_list<uint8_t> otherTypes = {
          SOURCE_TYPE_MIN, SOURCE_TYPE_MAX,
          SOURCE_TYPE_TX_VOLTAGE, SOURCE_TYPE_TX_TIME, SOURCE_TYPE_TX_GPS,
          SOURCE_TYPE_TIMER,
#if defined(LUMINOSITY_SENSOR)
          SOURCE_TYPE_LIGHT,
#endif
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

#if defined(HELI)
    if (modelHeliEnabled())
      addTypeButton(CHAR_CYC, SOURCE_TYPE_HELI, STR_MENU_HELI);
#endif

    addTypeButton(CHAR_TRIM, SOURCE_TYPE_TRIM, STR_MENU_TRIMS);

    // Switches: physical + function switch groups
    {
      std::initializer_list<uint8_t> switchTypes = {
          SOURCE_TYPE_SWITCH,
#if defined(FUNCTION_SWITCHES)
          SOURCE_TYPE_CUSTOM_SWITCH_GROUP,
#endif
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

    if (modelLSEnabled())
      addTypeButton("LS", SOURCE_TYPE_LOGICAL_SWITCH,
                    STR_MENU_LOGICAL_SWITCHES);

    addTypeButton(CHAR_TRAINER, SOURCE_TYPE_TRAINER, STR_MENU_TRAINER);

    addTypeButton(CHAR_CHANNEL, SOURCE_TYPE_CHANNEL, STR_MENU_CHANNELS);

#if defined(GVARS)
    if (modelGVEnabled())
      addTypeButton(CHAR_SLIDER, SOURCE_TYPE_GVAR, STR_MENU_GVARS);
#endif

    if (modelTelemetryEnabled())
      addTypeButton(CHAR_TELEMETRY, SOURCE_TYPE_TELEMETRY,
                    STR_MENU_TELEMETRY);

    // "None" button (clear)
    for (auto& g : choice->typeGroups) {
      if (g.type == SOURCE_TYPE_NONE) {
        if ((nxtBtnPos > filterColumns))
          addButton(STR_SELECT_MENU_CLR, g.startIdx, g.endIdx, nullptr,
                    nullptr, true);
        break;
      }
    }

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

  static LAYOUT_SIZE(FILTER_COLUMNS, 3, 2)

 protected:
  MenuToolbarButton* invertBtn = nullptr;
};

bool SourceChoice::onLongPress()
{
  if (canInvert) {
    SourceRef ref = _getSourceRef();
    if (ref.isInverted())
      ref.flags &= ~SOURCE_FLAG_INVERTED;
    else
      ref.flags |= SOURCE_FLAG_INVERTED;
    _setSourceRef(ref);
    update();
  }
  return true;
}

void SourceChoice::setValue(int value)
{
  if (value >= 0 && value < (int)entries.size()) {
    SourceRef ref = entries[value];
    if (inMenu && inverted) ref.flags |= SOURCE_FLAG_INVERTED;
    inMenu = false;
    _setSourceRef(ref);
  }
  // Call ChoiceBase update path
  Choice::setValue(value);
}

int SourceChoice::getIntValue() const
{
  SourceRef ref = _getSourceRef();
  return const_cast<SourceChoice*>(this)->findEntry(ref);
}

void SourceChoice::setAvailableHandler(std::function<bool(SourceRef)> handler)
{
  isRefAvailable = std::move(handler);
  buildEntries();
}

void SourceChoice::openMenu()
{
  setEditMode(true);  // this needs to be done first before menu is created.

  SourceRef cur = _getSourceRef();
  inverted = cur.isInverted();
  inMenu = true;

  auto menu = new Menu();
  if (menuTitle) menu->setTitle(menuTitle);

  auto tb = new SourceChoiceMenuToolbar(this, menu);
  menu->setToolbar(tb);

  if (canInvert)
    menu->setLongPressHandler([=]() { tb->invertChoice(); });

#if defined(AUTOSOURCE)
  menu->setWaitHandler([=]() {
    SourceRef moved = getMovedSource();
    if (!moved.isNone()) {
      int idx = findEntry(moved);
      if (idx >= 0) {
        tb->resetFilter();
        menu->select(idx);
      }
    }
#if defined(AUTOSWITCH)
    else {
      SwitchRef swtch = getMovedSwitch();
      if (!swtch.isNone() && swtch.type == SWITCH_TYPE_SWITCH) {
        // Convert switch to source (switch index = position / 3)
        SourceRef switchRef = {SOURCE_TYPE_SWITCH, 0, (uint16_t)(swtch.index / 3)};
        int idx = findEntry(switchRef);
        if (idx >= 0) {
          tb->resetFilter();
          menu->select(idx);
        }
      }
    }
#endif
  });
#endif

  // fillMenu(menu); - called by MenuToolbar

  menu->setCloseHandler([=]() { setEditMode(false); });
}

SourceChoice::SourceChoice(Window *parent, const rect_t &rect,
                           std::function<SourceRef()> getValue,
                           std::function<void(SourceRef)> setValue,
                           bool allowInvert) :
    Choice(parent, rect, 0, 0,
           nullptr,  // getValue - set below
           nullptr,  // setValue - set below
           STR_SOURCE),
    canInvert(allowInvert),
    _getSourceRef(std::move(getValue)),
    _setSourceRef(std::move(setValue))
{
  // Build the entries vector
  buildEntries();

  // Set up Choice's integer getter: find current SourceRef in entries
  _getValue = [this]() -> int {
    return getIntValue();
  };

  // Set up Choice's integer setter
  _setValue = [this](int value) {
    if (value >= 0 && value < (int)entries.size()) {
      SourceRef ref = entries[value];
      if (inMenu && inverted) ref.flags |= SOURCE_FLAG_INVERTED;
      inMenu = false;
      _setSourceRef(ref);
    }
  };

  setTextHandler([=](int value) -> std::string {
    if (value < 0 || value >= (int)entries.size()) {
      // Current value not in entries - show raw source string
      SourceRef ref = _getSourceRef();
      return std::string(getSourceString(ref));
    }

    SourceRef ref = entries[value];
    if (inMenu && inverted) ref.flags |= SOURCE_FLAG_INVERTED;
    else if (!inMenu) {
      // Outside menu, show the actual stored value (may be inverted)
      ref = _getSourceRef();
    }
    return std::string(getSourceString(ref));
  });

  // isValueAvailable always true - pre-filtered via buildEntries
  Choice::setAvailableHandler([](int) { return true; });
}
