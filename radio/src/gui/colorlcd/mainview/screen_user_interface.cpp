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

#include "screen_user_interface.h"

#include "choice.h"
#include "file_preview.h"
#include "theme_manager.h"
#include "view_main.h"

#if LANDSCAPE

// form grid
static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

#else

// form grid
static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

#endif

ScreenUserInterfacePage::ScreenUserInterfacePage(const PageDef& pageDef) :
    PageGroupItem(pageDef, PAD_TINY)
{
}

void ScreenUserInterfacePage::build(Window* window)
{
  window->padAll(PAD_TINY);
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

  FlexGridLayout grid(line_col_dsc, line_row_dsc, PAD_TINY);

  // Top Bar
  auto line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_TOP_BAR);

  new TextButton(line, rect_t{}, STR_SETUP_WIDGETS,
            [=]() -> uint8_t {
                window->getParent()->deleteLater();
                new SetupTopBarWidgetsPage();
                return 0;
            });

  line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_WIDGET_SIZE);
  line = window->newLine(grid);
  auto box = new Window(line, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_TINY);
  box->padAll(PAD_TINY);
  box->padLeft(PAD_MEDIUM);
  for (int i = 0; i < MAX_TOPBAR_ZONES; i += 1) {
    coord_t w = (g_model.topbarWidgetWidth[i] * (WWBTN_W + PAD_TINY)) - PAD_TINY;
    if (w < WWBTN_W) w = WWBTN_W;
    widths[i] = new Choice(box, {0, 0, w, 0}, 0, MAX_TOPBAR_ZONES,
              [=]() {
                return g_model.topbarWidgetWidth[i];
              },
              [=](int value) {
                g_model.topbarWidgetWidth[i] = value;
                coord_t w = (g_model.topbarWidgetWidth[i] * (WWBTN_W + PAD_TINY)) - PAD_TINY;
                widths[i]->setWidth(w);
                int remaining = MAX_TOPBAR_ZONES;
                for (int n = 0; n < MAX_TOPBAR_ZONES; n += 1) {
                  if (n > i) {
                    if (remaining > 0) {
                      if (g_model.topbarWidgetWidth[n] == 0) {
                        g_model.topbarWidgetWidth[n] = 1;
                        widths[n]->setWidth(WWBTN_W);
                        widths[n]->show();
                        widths[n]->update();
                      }
                    } else {
                      if (g_model.topbarWidgetWidth[n] != 0) {
                        g_model.topbarWidgetWidth[n] = 0;
                        widths[n]->setWidth(WWBTN_W);
                        widths[n]->hide();
                        ViewMain::instance()->getTopbar()->removeWidget(n);
                      }
                    }
                  }
                  ViewMain::instance()->getTopbar()->load();
                  remaining -= g_model.topbarWidgetWidth[n];
                }
                storageDirty(EE_MODEL);
              }, STR_WIDGET_SIZE);
    widths[i]->setAvailableHandler([=](int value) {
      int remaining = MAX_TOPBAR_ZONES;
      for (int n = 0; n < i; n += 1)
        remaining -= g_model.topbarWidgetWidth[n];
      return value > 0 && value <= remaining;
    });
    if (g_model.topbarWidgetWidth[i] == 0)
      widths[i]->hide();
  }
}
