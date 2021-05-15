/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "libopenui_config.h"
#include "switchchoice.h"
#include "menutoolbar.h"
#include "menu.h"
#include "draw_functions.h"
#include "strhelpers.h"
#include "dataconstants.h"
#include "edgetx.h"

class SwitchChoiceMenuToolbar : public MenuToolbar<SwitchChoice>
{
  public:
    SwitchChoiceMenuToolbar(SwitchChoice * choice, Menu * menu):
      MenuToolbar<SwitchChoice>(choice, menu)
    {
      addButton(CHAR_SWITCH, SWSRC_FIRST_SWITCH, SWSRC_LAST_SWITCH);
      addButton(CHAR_TRIM, SWSRC_FIRST_TRIM, SWSRC_LAST_TRIM);
      addButton(CHAR_SWITCH, SWSRC_FIRST_LOGICAL_SWITCH, SWSRC_LAST_LOGICAL_SWITCH);
    }
};

void SwitchChoice::paint(BitmapBuffer * dc)
{
  FormField::paint(dc);

  unsigned value = getValue();
  LcdFlags textColor;
  if (editMode)
    textColor = FOCUS_COLOR;
  else if (hasFocus())
    textColor = FOCUS_BGCOLOR;
  else if (value == 0)
    textColor = DISABLE_COLOR;
  else
    textColor = 0;
  drawSwitch(dc, FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value, textColor);
}

void SwitchChoice::fillMenu(Menu * menu, std::function<bool(int16_t)> filter)
{
  auto value = getValue();
  int count = 0;
#if defined(HARDWARE_TOUCH)
  int current = -1;
#else
  int current = 0;
#endif

  menu->removeLines();

  for (int i = vmin; i <= vmax; ++i) {
    if (filter && !filter(i))
      continue;
    if (isValueAvailable && !isValueAvailable(i))
      continue;
    menu->addLine(getSwitchPositionName(i), [=]() {
      setValue(i);
    });
    if (value == i) {
      current = count;
    }
    ++count;
  }

  if (current >= 0) {
    menu->select(current);
  }
}

void SwitchChoice::openMenu()
{
  auto menu = new Menu(this);
  fillMenu(menu);

  menu->setToolbar(new SwitchChoiceMenuToolbar(this, menu));
  menu->setCloseHandler([=]() {
      editMode = false;
      setFocus(SET_FOCUS_DEFAULT);
  });
}

#if defined(HARDWARE_KEYS)
void SwitchChoice::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    editMode = true;
    invalidate();
    openMenu();
  }
  else {
    FormField::onEvent(event);
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool SwitchChoice::onTouchEnd(coord_t, coord_t)
{
  openMenu();
  setFocus(SET_FOCUS_DEFAULT);
  setEditMode(true);
  return true;
}
#endif
