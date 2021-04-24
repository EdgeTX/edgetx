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
//#include "radio_setup.h"
#include "view_channels.h"
#include "view_statistics.h"
#include "fab_button.h"

constexpr coord_t FAB_BUTTON_WIDTH = 68;
constexpr coord_t FAB_BUTTON_BORDER = 12;
constexpr coord_t MENU_BORDER = 16;

class SelectFabButton : public FabButton
{
  public:
  SelectFabButton(FormGroup* parent, coord_t x, coord_t y, uint8_t icon,
                  const char* title, std::function<uint8_t(void)> pressHandler,
                  WindowFlags windowFlags = 0) :
      FabButton(parent, x + FAB_BUTTON_WIDTH / 2, y + FAB_BUTTON_WIDTH / 2,
                icon, pressHandler, windowFlags),
      title(title)
  {
    setHeight(height() + 2 * PAGE_LINE_HEIGHT);
  }

  void paint(BitmapBuffer* dc) override
  {
    FabButton::paint(dc);
    auto pos = title.find('\n');
    dc->drawSizedText(width() / 2, FAB_BUTTON_WIDTH, title.c_str(), pos,
                      FOCUS_COLOR | CENTERED);
    dc->drawSizedText(width() / 2, FAB_BUTTON_WIDTH, title.substr(pos).c_str(), 255,
                      FOCUS_COLOR | CENTERED);
    if (hasFocus()) {
      dc->drawSolidRect(0, 0, width(), height(), 2, CHECKBOX_COLOR);
    }
  }

  void onEvent(event_t event) override;

  protected:
  std::string title;
};

void SelectFabButton::onEvent(event_t event)
{
#if defined(HARDWARE_KEYS)
  switch (event) {
    case EVT_KEY_BREAK(KEY_EXIT):
      killEvents(event);
      parent->deleteLater();
      return;
  }    
#endif
  FabButton::onEvent(event);
}

ViewMainMenu::ViewMainMenu(Window* parent) :
    FormGroup(parent->getFullScreenWindow(),
              { 0, 0, 350, 180 }, OPAQUE | FORM_FORWARD_FOCUS)
{
  Layer::push(this);

  coord_t y_pos = MENU_BORDER;
  coord_t x_pos = MENU_BORDER;

  // Disabled Title
  //
  // auto title = new StaticText(this,
  //                {0, pos, width(), getFontHeight(FONT(XL))},
  //                "Tasks", 0, FOCUS_COLOR | FONT(XL) | CENTERED);
  // pos += title->height() + PAGE_LINE_SPACING;

  new SelectFabButton(this, x_pos, y_pos, ICON_RADIO_TOOLS, "Model\nSettings",
                      [=]() -> uint8_t {
    deleteLater();
    new ModelMenu();
    return 0;
  });

  x_pos += FAB_BUTTON_WIDTH + FAB_BUTTON_BORDER;
  
  new SelectFabButton(this, x_pos, y_pos, ICON_MODEL, "Select\nModel",
                      [=]() -> uint8_t {
    deleteLater();
    new ModelSelectMenu();
    return 0;
  });

  x_pos += FAB_BUTTON_WIDTH + FAB_BUTTON_BORDER;

  new SelectFabButton(this, x_pos, y_pos, ICON_RADIO, "Radio\nSettings",
                     [=]() -> uint8_t {
    deleteLater();
    new RadioMenu();
    return 0;
  });

  x_pos += FAB_BUTTON_WIDTH + FAB_BUTTON_BORDER;

  auto button = new SelectFabButton(this, x_pos, y_pos, ICON_THEME, "Screens\nSetup",
                      [=]() -> uint8_t {
    deleteLater();
    new ScreenMenu();
    return 0;
  });

  x_pos += FAB_BUTTON_WIDTH + MENU_BORDER;
  y_pos += button->height() + MENU_BORDER;

  setWidth(x_pos);
  setInnerHeight(x_pos);

  setHeight(y_pos);
  setInnerHeight(y_pos);

  setWindowCentered();

  // new IconButton(
  //     this, {0, 0, MENU_HEADER_BUTTON_WIDTH, MENU_HEADER_BUTTON_WIDTH},
  //     ICON_BACK,
  //     [=]() -> uint8_t {
  //       // openMenu();
  //       return 0;
  //     });

  //TODO: adjust to whatever is in this window
  //setRect()
  
  
  // Menu* menu = new Menu(this);
  // menu->addLine(STR_MODEL_SELECT, []() { new ModelSelectMenu(); });
  // // if (modelHasNotes()) {
  // //   menu->addLine(STR_VIEW_NOTES, [=]() {
  // //     // TODO
  // //   });
  // // }
  // menu->addLine(STR_MONITOR_SCREENS, []() { new ChannelsViewMenu(); });
  // menu->addLine(STR_RESET_SUBMENU, [this]() {
  //   Menu* resetMenu = new Menu(this);
  //   resetMenu->addLine(STR_RESET_FLIGHT, []() { flightReset(); });
  //   resetMenu->addLine(STR_RESET_TIMER1, []() { timerReset(0); });
  //   resetMenu->addLine(STR_RESET_TIMER2, []() { timerReset(1); });
  //   resetMenu->addLine(STR_RESET_TIMER3, []() { timerReset(2); });
  //   resetMenu->addLine(STR_RESET_TELEMETRY, []() { telemetryReset(); });
  // });
  // menu->addLine(STR_STATISTICS, []() { new StatisticsViewPageGroup(); });
  // menu->addLine(STR_ABOUT_US, []() {
  //   // TODO
  // });

  // // Place at current position
  // menu->setLeft(menu->left() + getScrollPositionX());
  // menu->setTop(menu->top() + getScrollPositionY());

  setFocus();
}

void ViewMainMenu::paint(BitmapBuffer* dc)
{
  dc->drawFilledRect(0, 0, width(), height(), SOLID, OVERLAY_COLOR, OPACITY(5));
}
void ViewMainMenu::deleteLater(bool detach, bool trash)
{
  Layer::pop(this);
  FormGroup::deleteLater(detach, trash);
}
