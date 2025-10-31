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

#include "radio_key_shortcuts.h"
#include "edgetx.h"
#include "pagegroup.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

static std::string replaceAll(std::string str, const std::string& from, const std::string& to)
{
    auto&& pos = str.find(from, size_t{});
    while (pos != std::string::npos)
    {
        str.replace(pos, from.length(), to);
        // easy to forget to add to.length()
        pos = str.find(from, pos + to.length());
    }
    return str;
}

void RadioKeyShortcutsPage::addKey(event_t event, std::vector<std::string> qmPages)
{
  EnumKeys key = (EnumKeys)EVT_KEY_MASK(event);
  auto keys = keysGetSupported();
  if (keys & (1 << key)) {
    extern uint16_t keyMapping(uint16_t event);
    event = keyMapping(event);

    setupLine(keysGetLabel(key), [=](Window* parent, coord_t x, coord_t y) {
          auto c = new Choice(
              parent, {LCD_W / 4, y, 0, 0}, qmPages, QM_NONE, QM_TOOLS_DEBUG,
              GET_DEFAULT(g_eeGeneral.getKeyShortcut(event)),
              [=](int32_t newValue) {
                g_eeGeneral.setKeyShortcut(event, (QMPage)newValue);
                SET_DIRTY();
              });

          c->setPopupWidth(LCD_W * 3 / 4);
          c->setAvailableHandler(
              [=](int newValue) { return newValue <= QM_UI_SCREEN1 || newValue > QM_UI_ADD_PG; });
        });
  }
}

RadioKeyShortcutsPage::RadioKeyShortcutsPage():
        SubPage(ICON_RADIO, STR_MAIN_MENU_RADIO_SETTINGS, "Key Shortcuts", true)
{
  std::vector<std::string> qmPages;

  qmPages.emplace_back(STR_NONE);
  qmPages.emplace_back("Open Quick Menu");

  for (int i = 0; qmTopItems[i].icon != EDGETX_ICONS_COUNT; i += 1) {
    if (qmTopItems[i].pageAction == QM_ACTION) {
      qmPages.emplace_back(qmTopItems[i].title);
    } else {
      PageDef* sub = qmTopItems[i].subMenuItems;
      for (int j = 0; sub[j].icon != EDGETX_ICONS_COUNT; j += 1) {
        std::string s = qmTopItems[i].title;
        s += " - ";
        if (sub[j].qmPage >= QM_UI_SCREEN1 && sub[j].qmPage <= QM_UI_SCREEN10)
          s += "Current Screen";
        else if (sub[j].title)
          s += sub[j].title;
        s = replaceAll(s, "\n", " ");
        qmPages.emplace_back(s);
      }
    }
  }

  setupLine("Short Press", nullptr);
  addKey(EVT_KEY_BREAK(KEY_SYS), qmPages);
  addKey(EVT_KEY_BREAK(KEY_MODEL), qmPages);
  addKey(EVT_KEY_BREAK(KEY_TELE), qmPages);

  setupLine("Long Press", nullptr);
  addKey(EVT_KEY_LONG(KEY_SYS), qmPages);
  addKey(EVT_KEY_LONG(KEY_MODEL), qmPages);
  addKey(EVT_KEY_LONG(KEY_TELE), qmPages);

  enableRefresh();
}
