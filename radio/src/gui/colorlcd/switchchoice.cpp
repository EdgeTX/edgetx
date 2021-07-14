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
#include "opentx.h"

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
    textColor = FOCUS_COLOR;
  // else if (value == 0)
  //   textColor = DISABLE_COLOR;
  else
    textColor = DEFAULT_COLOR;

  drawSwitch(dc, FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value, textColor);
  dc->drawBitmapPattern(rect.w - 20, (rect.h - 11) / 2, LBM_DROPDOWN,
                        textColor);
}

void SwitchChoice::fillMenu(Menu * menu, std::function<bool(int16_t)> filter)
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
        if (setValue) setValue(val);
        this->fillMenu(menu);
      }
    }
  });
#endif
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

  if (event == EVT_KEY_LONG(KEY_ENTER)) {
    int16_t value = getValue();
    setValue(-value);
  }

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
bool SwitchChoice::onTouchEnd(coord_t x, coord_t y)
{
  setFocus(SET_FOCUS_DEFAULT);
  Window::onTouchEnd(x, y);

  if (isLongPress()) {
    int16_t val = getValue();
    setValue(-val);
  }

  setEditMode(true);
  openMenu();
  return true;
}
#endif
