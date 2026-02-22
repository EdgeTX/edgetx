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

#include "quick_menu_favorites.h"

#include "choice.h"
#include "edgetx.h"
#include "getset_helpers.h"
#include "pagegroup.h"
#include "qmpagechoice.h"
#include "radio_tools.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

QMFavoritesPage::QMFavoritesPage():
        SubPage(ICON_RADIO, STR_MAIN_MENU_RADIO_SETTINGS, STR_QUICK_MENU_FAVORITES, true)
{
  auto qmPages = QuickMenu::menuPageNames(true);

  for (int i = 0; i < MAX_QM_FAVORITES; i += 1) {
    char nm[50];
    strAppendUnsigned(strAppend(strAppend(nm, "#"), " "), i + 1);
    setupLine(nm, [=](Window* parent, coord_t x, coord_t y) {
          auto c = new QMPageChoice(
              parent, {LCD_W / 4, y, LCD_W * 2 / 3, 0}, qmPages, QM_NONE, qmPages.size() - 1,
              [=]() -> int {
                auto pg = g_eeGeneral.qmFavorites[i].shortcut;
                if (pg < QM_APP)
                  return pg;
                std::string nm = g_eeGeneral.getFavoriteToolName(i);
                int idx = getLuaToolId(nm);
                if (idx >= 0)
                  return pg + idx;
                return QM_NONE;
              },
              [=](int32_t pg) {
                if (pg < QM_APP) {
                  g_eeGeneral.qmFavorites[i].shortcut = (QMPage)pg;
                  g_eeGeneral.setFavoriteToolName(i, "");
                } else {
                  g_eeGeneral.qmFavorites[i].shortcut = QM_APP;
                  g_eeGeneral.setFavoriteToolName(i, getLuaTool(pg - QM_APP)->label);
                }
                changed = true;
                SET_DIRTY();
              }, STR_QUICK_MENU_FAVORITES);

          c->setAvailableHandler(
              [=](int pg) {
                if (pg == QM_NONE) return true;
                if (pg < QM_APP) {
                  for (int n = 0; n < MAX_QM_FAVORITES; n += 1)
                    if (g_eeGeneral.qmFavorites[n].shortcut == (QMPage)pg)
                      return n == i;
                } else {
                  for (int n = 0; n < MAX_QM_FAVORITES; n += 1)
                    if (g_eeGeneral.qmFavorites[n].shortcut == QM_APP) {
                      int idx = getLuaToolId(g_eeGeneral.getFavoriteToolName(n)) + QM_APP;
                      if (idx == pg)
                        return n == i;
                    }
                }
                return pg != QM_OPEN_QUICK_MENU; }
              );
        });
  }

  enableRefresh();
}

void QMFavoritesPage::onCancel()
{
  SubPage::onCancel();

  if (changed) {
    // Delete quick menu, and close parent page group (in case it is Favorites group)
    QuickMenu::resetFavorites();
    QuickMenu::setCurrentPage(QM_NONE);
    Window::topWindow()->onCancel();
  }
}
