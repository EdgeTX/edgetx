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

#include "button.h"
#include "choice.h"
#include "edgetx.h"
#include "getset_helpers.h"
#include "pagegroup.h"
#include "qmpagechoice.h"
#include "radio_tools.h"

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
          auto c = new QMPageChoice(
              parent, {LCD_W / 4, y, LCD_W * 2 / 3, 0}, qmPages, QM_NONE, qmPages.size() - 1,
              [=]() -> int {
                auto pg = g_eeGeneral.getKeyShortcut(event);
                if (pg < QM_APP)
                  return pg;
                std::string nm = g_eeGeneral.getKeyToolName(event);
                int idx = getLuaToolId(nm);
                if (idx >= 0)
                  return pg + idx;
                return QM_NONE;
              },
              [=](int32_t newValue) {
                if (newValue < QM_APP) {
                  g_eeGeneral.setKeyShortcut(event, (QMPage)newValue);
                  g_eeGeneral.setKeyToolName(event, "");
                } else {
                  g_eeGeneral.setKeyShortcut(event, QM_APP);
                  g_eeGeneral.setKeyToolName(event, getLuaTool(newValue - QM_APP)->label);
                }
                SET_DIRTY();
              }, STR_KEY_SHORTCUTS);

          c->setAvailableHandler(
              [=](int pg) {
                if (pg == QM_NONE) return true;
                if (g_eeGeneral.hasKeyShortcut((QMPage)pg, event))
                  return false;
                return pg <= QM_UI_SCREEN1 || pg > QM_UI_ADD_PG; }
              );
        });
  }
}

QMKeyShortcutsPage::QMKeyShortcutsPage():
        SubPage(ICON_RADIO, STR_MAIN_MENU_RADIO_SETTINGS, STR_KEY_SHORTCUTS, true)
{
  auto qmPages = QuickMenu::menuPageNames(false);

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
