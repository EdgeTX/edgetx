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

#include "view_main_menu.h"
#include "menu.h"
#include "audio.h"
#include "translations.h"
#include "model_select.h"
#include "menu_model.h"
#include "menu_radio.h"
#include "menu_screen.h"
#include "view_channels.h"
#include "view_statistics.h"
#include "select_fab_carousel.h"

ViewMainMenu::ViewMainMenu(Window* parent) :
    Window(parent->getFullScreenWindow(), {})
{
  // Save focus
  Layer::push(this);

  // Take over the screen
  setWidth(parent->width());
  setHeight(parent->height());

  auto carousel = new SelectFabCarousel(this);
  carousel->setMaxButtons(4);

  // Disabled Title
  //
  // auto title = new StaticText(this,
  //                {0, pos, width(), getFontHeight(FONT(XL))},
  //                "Tasks", 0, FOCUS_COLOR | FONT(XL) | CENTERED);
  // pos += title->height() + PAGE_LINE_SPACING;

  carousel->addButton(ICON_RADIO_TOOLS, "Model\nSettings", [=]() -> uint8_t {
    deleteLater();
    new ModelMenu();
    return 0;
  });

  carousel->addButton(ICON_MODEL, "Select\nModel", [=]() -> uint8_t {
    deleteLater();
    new ModelSelectMenu();
    return 0;
  });

  carousel->addButton(ICON_RADIO, "Radio\nSettings", [=]() -> uint8_t {
    deleteLater();
    new RadioMenu();
    return 0;
  });

  carousel->addButton(ICON_THEME, "Screens\nSetup", [=]() -> uint8_t {
    deleteLater();
    new ScreenMenu();
    return 0;
  });

  carousel->addButton(ICON_MONITOR, "Channel\nMonitor", [=]() -> uint8_t {
    deleteLater();
    new ChannelsViewMenu();
    return 0;
  });

  carousel->addButton(ICON_MODEL_TELEMETRY, "Reset\nTelemetry", [=]() -> uint8_t {
    deleteLater();
    Menu* resetMenu = new Menu(parent);
    resetMenu->addLine(STR_RESET_FLIGHT, []() { flightReset(); });
    resetMenu->addLine(STR_RESET_TIMER1, []() { timerReset(0); });
    resetMenu->addLine(STR_RESET_TIMER2, []() { timerReset(1); });
    resetMenu->addLine(STR_RESET_TIMER3, []() { timerReset(2); });
    resetMenu->addLine(STR_RESET_TELEMETRY, []() { telemetryReset(); });
    return 0;
  });

  carousel->addButton(ICON_STATS, "Statistics", [=]() -> uint8_t {
    deleteLater();
    new StatisticsViewPageGroup();
    return 0;
  });

  carousel->addButton(ICON_OPENTX, "About\nOpenTx", [=]() -> uint8_t {
    deleteLater();
    //TODO: new AboutUs();
    return 0;
  });

  carousel->setWindowCentered();
  carouselRect = carousel->getRect();

  carousel->setCloseHandler([=]() { deleteLater(); });
  carousel->setFocus();
}

uint16_t* lcdGetBackupBuffer();

void ViewMainMenu::paint(BitmapBuffer* dc)
{
  rect_t zone = carouselRect;
  zone.x -= 8; zone.y -= 8;
  zone.w += 16; zone.h += 16;
  dc->drawFilledRect(zone.x, zone.y, zone.w, zone.h, SOLID, OVERLAY_COLOR, OPACITY(5));
}

void ViewMainMenu::deleteLater(bool detach, bool trash)
{
  Layer::pop(this);
  Window::deleteLater(detach, trash);
}
