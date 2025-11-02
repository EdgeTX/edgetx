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

#include "key_shortcuts.h"
#include "edgetx.h"
#include "pagegroup.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

static bool hasKey(event_t event)
{
#if defined(USE_HATS_AS_KEYS)
  return true;
#else
  EnumKeys key = (EnumKeys)EVT_KEY_MASK(event);
  return (keysGetSupported() & (1 << key));
#endif
}

void QMKeyShortcutsPage::addKey(event_t event, std::vector<std::string> qmPages, const char* nm)
{
  if (hasKey(event)) {
    extern uint16_t keyMapping(uint16_t event);
    event = keyMapping(event);

    setupLine(nm, [=](Window* parent, coord_t x, coord_t y) {
          auto c = new Choice(
              parent, {LCD_W / 4, y, LCD_W * 2 / 3, 0}, qmPages, QM_NONE, QM_TOOLS_DEBUG,
              GET_DEFAULT(g_eeGeneral.getKeyShortcut(event)),
              [=](int32_t newValue) {
                g_eeGeneral.setKeyShortcut(event, (QMPage)newValue);
                SET_DIRTY();
              }, STR_KEY_SHORTCUTS);

          c->setPopupWidth(LCD_W * 3 / 4);
          c->setAvailableHandler(
              [=](int newValue) { return newValue <= QM_UI_SCREEN1 || newValue > QM_UI_ADD_PG; });
        });
  }
}

QMKeyShortcutsPage::QMKeyShortcutsPage():
        SubPage(ICON_RADIO, STR_MAIN_MENU_RADIO_SETTINGS, STR_KEY_SHORTCUTS, true)
{
  std::vector<std::string> qmPages = QuickMenu::menuPageNames(false);

  setupLine(STR_SHORT_PRESS, nullptr);
  addKey(EVT_KEY_BREAK(KEY_SYS), qmPages, "SYS");
  addKey(EVT_KEY_BREAK(KEY_MODEL), qmPages, "MDL");
  addKey(EVT_KEY_BREAK(KEY_TELE), qmPages, "TELE");

  setupLine(STR_LONG_PRESS, nullptr);
  addKey(EVT_KEY_LONG(KEY_SYS), qmPages, "SYS");
  addKey(EVT_KEY_LONG(KEY_MODEL), qmPages, "MDL");
  addKey(EVT_KEY_LONG(KEY_TELE), qmPages, "TELE");

  new TextButton(body, {LV_PCT(10), y + PAD_LARGE, LV_PCT(80), 0}, STR_SF_RESET,
                  [=]() {
                    g_eeGeneral.defaultKeyShortcuts();
                    SET_DIRTY();
                    return 0;
                  });

  enableRefresh();
}
