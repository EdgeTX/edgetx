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
#include "sourcechoice.h"
#include "menutoolbar.h"
#include "menu.h"
#include "dataconstants.h"
#include "lcd.h"
#include "strhelpers.h"
#include "draw_functions.h"
#include "opentx.h"

class SourceChoiceMenuToolbar : public MenuToolbar<SourceChoice>
{
  public:
    SourceChoiceMenuToolbar(SourceChoice * choice, Menu * menu):
      MenuToolbar<SourceChoice>(choice, menu)
    {
      addButton(CHAR_INPUT, MIXSRC_FIRST_INPUT, MIXSRC_LAST_INPUT);
#if defined(LUA_MODEL_SCRIPTS)
      addButton(CHAR_LUA, MIXSRC_LAST_LUA, MIXSRC_FIRST_LUA);
#endif
      addButton(CHAR_STICK, MIXSRC_FIRST_STICK, MIXSRC_LAST_STICK);
      addButton(CHAR_POT, MIXSRC_FIRST_POT, MIXSRC_LAST_POT);
      addButton(CHAR_FUNCTION, MIXSRC_MAX, MIXSRC_MAX);
#if defined(HELI)
      addButton(CHAR_CYC, MIXSRC_FIRST_HELI, MIXSRC_LAST_HELI);
#endif
      addButton(CHAR_TRIM, MIXSRC_FIRST_TRIM, MIXSRC_LAST_TRIM);
      addButton(CHAR_SWITCH, MIXSRC_FIRST_SWITCH, MIXSRC_LAST_SWITCH);
      addButton(CHAR_TRAINER, MIXSRC_FIRST_TRAINER, MIXSRC_LAST_TRAINER);
      addButton(CHAR_CHANNEL, MIXSRC_FIRST_CH, MIXSRC_LAST_CH);
#if defined(GVARS)
      addButton(CHAR_SLIDER, MIXSRC_LAST_GVAR, MIXSRC_FIRST_GVAR);
#endif
      addButton(CHAR_TELEMETRY, MIXSRC_FIRST_TELEM, MIXSRC_LAST_TELEM);
    }
};

void SourceChoice::paint(BitmapBuffer * dc)
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
  drawSource(dc, FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value, textColor);
}

void SourceChoice::fillMenu(Menu * menu, const std::function<bool(int16_t)> & filter)
{
  auto value = getValue();
  int count = 0;
  int current = 0;

  menu->removeLines();

  for (int i = vmin; i <= vmax; ++i) {
    if (filter && !filter(i)) continue;
    if (isValueAvailable && !isValueAvailable(i)) continue;
    menu->addLine(getSourceString(i), [=]() { setValue(i); });
    if (value == i) {
      current = count;
    }
    ++count;
  }

  if (current >= 0) {
    menu->select(current);
  }

#if defined(AUTOSOURCE)
  menu->setWaitHandler([=]() {
      int8_t val = getMovedSource(0);
      if (val) {
        if (filter && filter(val)) {
          return;
        }
        if (setValue) {
          setValue(val);
        }
        this->fillMenu(menu);
        menu->setFocusBody();
      }
    });
#endif
}

void SourceChoice::openMenu()
{
  auto menu = new Menu(this);
  fillMenu(menu);
  menu->setToolbar(new SourceChoiceMenuToolbar(this, menu));
  menu->setCloseHandler([=]() {
      editMode = false;
      setFocus(SET_FOCUS_DEFAULT);
  });
}

#if defined(HARDWARE_KEYS)
void SourceChoice::onEvent(event_t event)
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
bool SourceChoice::onTouchEnd(coord_t, coord_t)
{
  setFocus(SET_FOCUS_DEFAULT);
  setEditMode(true);
  openMenu();
  return true;
}
#endif
