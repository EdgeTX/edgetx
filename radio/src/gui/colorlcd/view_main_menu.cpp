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

#include "view_main_menu.h"

#include "menu_model.h"
#include "menu_radio.h"
#include "menu_screen.h"
#include "model_select.h"
#include "select_fab_carousel.h"
#include "view_about.h"
#include "view_channels.h"
#include "view_statistics.h"
#include "view_text.h"

#if LCD_W > LCD_H
#define VM_W (FAB_BUTTON_SIZE * 4 + 16)
#define VM_H ((FAB_BUTTON_SIZE + 34) * 2 + 16)
#else
#define VM_W (FAB_BUTTON_SIZE * 3 + 16)
#define VM_H ((FAB_BUTTON_SIZE + 34) * 3 + 16)
#endif

ViewMainMenu::ViewMainMenu(Window* parent, std::function<void()> closeHandler) :
    Window(parent->getFullScreenWindow(), {0, 0, LCD_W, LCD_H}),
    closeHandler(std::move(closeHandler))
{
  // Save focus
  Layer::push(this);

  coord_t width = VM_W;
  bool hasNotes = modelHasNotes();

#if LCD_W > LCD_H
  if (hasNotes)
    width += FAB_BUTTON_SIZE;
#endif

  auto box =
      new Window(this, {(LCD_W - width) / 2, (LCD_H - VM_H) / 2, width, VM_H}, 0,
                 0, etx_modal_dialog_create);
  box->padAll(8);

  auto carousel = new SelectFabCarousel(box);
  carousel->addButton(ICON_MODEL_SELECT, STR_MAIN_MENU_MANAGE_MODELS,
                      [=]() -> uint8_t {
                        deleteLater();
                        new ModelLabelsWindow();
                        return 0;
                      });

  if (hasNotes) {
    carousel->addButton(ICON_MODEL_NOTES, STR_MAIN_MENU_MODEL_NOTES,
                        [=]() -> uint8_t {
                          deleteLater();
                          readModelNotes(true);
                          return 0;
                        });
  }

  carousel->addButton(ICON_MONITOR, STR_MAIN_MENU_CHANNEL_MONITOR,
                      [=]() -> uint8_t {
                        deleteLater();
                        new ChannelsViewMenu();
                        return 0;
                      });

  carousel->addButton(ICON_MODEL, STR_MAIN_MENU_MODEL_SETTINGS,
                      [=]() -> uint8_t {
                        deleteLater();
                        new ModelMenu();
                        return 0;
                      });

  carousel->addButton(ICON_RADIO, STR_MAIN_MENU_RADIO_SETTINGS,
                      [=]() -> uint8_t {
                        deleteLater();
                        new RadioMenu();
                        return 0;
                      });

  carousel->addButton(ICON_THEME, STR_MAIN_MENU_SCREEN_SETTINGS,
                      [=]() -> uint8_t {
                        deleteLater();
                        new ScreenMenu();
                        return 0;
                      });

  carousel->addButton(
      ICON_MODEL_TELEMETRY, STR_MAIN_MENU_RESET_TELEMETRY, [=]() -> uint8_t {
        deleteLater();
        Menu* resetMenu = new Menu(parent);
        resetMenu->addLine(STR_RESET_FLIGHT, []() { flightReset(); });
        resetMenu->addLine(STR_RESET_TIMER1, []() { timerReset(0); });
        resetMenu->addLine(STR_RESET_TIMER2, []() { timerReset(1); });
        resetMenu->addLine(STR_RESET_TIMER3, []() { timerReset(2); });
        resetMenu->addLine(STR_RESET_TELEMETRY, []() { telemetryReset(); });
        return 0;
      });

  carousel->addButton(ICON_STATS, STR_MAIN_MENU_STATISTICS, [=]() -> uint8_t {
    deleteLater();
    new StatisticsViewPageGroup();
    return 0;
  });

  carousel->addButton(ICON_EDGETX, STR_MAIN_MENU_ABOUT_EDGETX,
                      [=]() -> uint8_t {
                        deleteLater();
                        new AboutUs();
                        return 0;
                      });
}

void ViewMainMenu::deleteLater(bool detach, bool trash)
{
  if (closeHandler) closeHandler();
  Layer::pop(this);
  Window::deleteLater(detach, trash);
}

void ViewMainMenu::onClicked() { deleteLater(); }

void ViewMainMenu::onCancel() { deleteLater(); }
