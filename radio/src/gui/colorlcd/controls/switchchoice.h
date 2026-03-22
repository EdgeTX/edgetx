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

#include "choice.h"
#include "form.h"
#include "sourceref.h"

class SwitchChoiceMenuToolbar;

class SwitchChoice : public Choice
{
 public:
  SwitchChoice(Window* parent, const rect_t& rect,
               std::function<SwitchRef()> getValue,
               std::function<void(SwitchRef)> setValue);

  void setAvailableHandler(std::function<bool(SwitchRef)> handler);

 protected:
  friend SwitchChoiceMenuToolbar;

  bool inMenu = false;

  std::function<SwitchRef()> _getSwitchRef;
  std::function<void(SwitchRef)> _setSwitchRef;
  std::function<bool(SwitchRef)> isRefAvailable;

  // Flat list of all valid SwitchRefs and toolbar group boundaries
  std::vector<SwitchRef> entries;

  struct TypeGroup {
    uint8_t type;
    int startIdx;
    int endIdx;  // inclusive
  };
  std::vector<TypeGroup> typeGroups;

  void buildEntries();

  void setValue(int value) override;
  int getIntValue() const override;
  bool onLongPress() override;

  void invertChoice();

  void openMenu() override;

  // Count of candidate indices for a given switch type (before availability filtering)
  static uint16_t switchTypeCount(uint8_t type);

  // Find the index in entries for a given SwitchRef (ignoring inversion flag)
  int findEntry(SwitchRef ref) const;

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "SwitchChoice"; }
#endif
};
