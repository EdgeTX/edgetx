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
#include "menu.h"
#include "audio.h"
#include "translations.h"
#include "model_select.h"
#include "menu_model.h"
#include "menu_radio.h"
#include "menu_screen.h"
#include "view_about.h"
#include "view_channels.h"
#include "view_statistics.h"
#include "select_fab_carousel.h"
#include "view_text.h"

ViewMainMenu::ViewMainMenu(Window* parent) :
    Window(parent->getFullScreenWindow(), rect_t{})
{
  // Save focus
  Layer::push(this);

  // Take over the screen
  setWidth(parent->width());
  setHeight(parent->height());

  auto carousel = new SelectFabCarousel(this);
  carousel->addButton(ICON_MODEL_SELECT, STR_MAIN_MENU_SELECT_MODEL, [=]() -> uint8_t {
    deleteLater();
    new ModelSelectMenu();
    return 0;
  });

  if (modelHasNotes()) {
    carousel->addButton(ICON_MODEL_NOTES, STR_MAIN_MENU_MODEL_NOTES, [=]() -> uint8_t {
      deleteLater();
      readModelNotes();
      return 0;
    });
  }

  carousel->addButton(ICON_MONITOR, STR_MAIN_MENU_CHANNEL_MONITOR, [=]() -> uint8_t {
    deleteLater();
    new ChannelsViewMenu();
    return 0;
  });

  carousel->addButton(ICON_MODEL, STR_MAIN_MENU_MODEL_SETTINGS, [=]() -> uint8_t {
    deleteLater();
    new ModelMenu();
    return 0;
  });

  carousel->addButton(ICON_RADIO, STR_MAIN_MENU_RADIO_SETTINGS, [=]() -> uint8_t {
    deleteLater();
    new RadioMenu();
    return 0;
  });

  carousel->addButton(ICON_THEME, STR_MAIN_MENU_SCREEN_SETTINGS, [=]() -> uint8_t {
    deleteLater();
    new ScreenMenu();
    return 0;
  });

  carousel->addButton(ICON_MODEL_TELEMETRY, STR_MAIN_MENU_RESET_TELEMETRY, [=]() -> uint8_t {
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

  carousel->addButton(ICON_OPENTX, STR_MAIN_MENU_ABOUT_EDGETX, [=]() -> uint8_t {
    deleteLater();
    new AboutUs();
    return 0;
  });

  auto carousel_obj = carousel->getLvObj();
  lv_obj_center(carousel_obj);

  lv_obj_update_layout(carousel_obj);
  carouselRect.x = lv_obj_get_x(carousel_obj);
  carouselRect.y = lv_obj_get_y(carousel_obj);
  carouselRect.w = lv_obj_get_width(carousel_obj);
  carouselRect.h = lv_obj_get_height(carousel_obj);

  // carousel->setCloseHandler([=]() { deleteLater(); });
  // carousel->setFocus();
}

uint16_t* lcdGetBackupBuffer();

void ViewMainMenu::paint(BitmapBuffer* dc)
{
  rect_t zone = carouselRect;
  zone.x -= 8; zone.y -= 8;
  zone.w += 16; zone.h += 16;

  dc->drawFilledRect(zone.x, zone.y, zone.w, zone.h, SOLID, BLACK, OPACITY(4));
}

void ViewMainMenu::deleteLater(bool detach, bool trash)
{
  Layer::pop(this);
  Window::deleteLater(detach, trash);
}

void ViewMainMenu::onCancel()
{
  deleteLater();
}
