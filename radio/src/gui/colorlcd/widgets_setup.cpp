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

#include "widgets_setup.h"
#include "menu_screen.h"
#include "view_main.h"
#include "widget_settings.h"
#include "widget.h"

#include "layer.h"
#include "menu.h"

#include "gui.h"
#include "translations.h"

SetupWidgetsPageSlot::SetupWidgetsPageSlot(FormGroup* parent,
                                           const rect_t& rect,
                                           WidgetsContainer* container,
                                           uint8_t slotIndex) :
    Button(parent, rect)
{
  setPressHandler([parent, container, slotIndex]() -> uint8_t {
    Menu* menu = new Menu(parent);
    menu->addLine(STR_SELECT_WIDGET, [=]() {
      Menu* menu = new Menu(parent);
      for (auto factory : getRegisteredWidgets()) {
        menu->addLine(factory->getDisplayName(), [=]() {
          container->createWidget(slotIndex, factory);
          auto widget = container->getWidget(slotIndex);
          if (widget->getOptions() && widget->getOptions()->name)
            new WidgetSettings(parent, widget);
        });
      }
    });

    if (container->getWidget(slotIndex)) {
      auto widget = container->getWidget(slotIndex);
      if (widget->getOptions() && widget->getOptions()->name)
        menu->addLine(STR_WIDGET_SETTINGS,
                      [=]() { new WidgetSettings(parent, widget); });
      menu->addLine(STR_REMOVE_WIDGET,
                    [=]() { container->removeWidget(slotIndex); });
    }

    return 0;
  });
}

void SetupWidgetsPageSlot::paint(BitmapBuffer* dc)
{
  if (hasFocus()) {
    dc->drawRect(0, 0, width() - 1, height() - 1, 2, STASHED,
                 COLOR_THEME_FOCUS);
  } else {
    dc->drawSolidRect(0, 0, width() - 1, height() - 1, 2, COLOR_THEME_PRIMARY3);
  }
}

SetupWidgetsPage::SetupWidgetsPage(ScreenMenu* menu, uint8_t customScreenIdx) :
    FormWindow(ViewMain::instance(), {0, 0, 0, 0}, FORM_FORWARD_FOCUS),
    menu(menu),
    customScreenIdx(customScreenIdx)
{
  Layer::push(this);

  // attach this custom screen here so we can display it
  auto screen = customScreens[customScreenIdx];
  if (screen) {
    setRect(screen->getRect());
    auto viewMain = ViewMain::instance();
    savedView = viewMain->getCurrentMainView();
    viewMain->setCurrentMainView(customScreenIdx);
    viewMain->bringToTop();
  }

  for (unsigned i = 0; i < screen->getZonesCount(); i++) {
    auto rect = screen->getZone(i);
    auto widget =
        new SetupWidgetsPageSlot(this, rect, customScreens[customScreenIdx], i);
    if (i == 0) widget->setFocus();
  }

#if defined(HARDWARE_TOUCH)
  new Button(
      this,
      {0, 0, MENU_HEADER_BACK_BUTTON_WIDTH, MENU_HEADER_BACK_BUTTON_HEIGHT},
      [this]() -> uint8_t {
        this->deleteLater();
        return 1;
      },
      NO_FOCUS | FORM_NO_BORDER);
#endif
}

void SetupWidgetsPage::deleteLater(bool detach, bool trash)
{
#if defined(HARDWARE_TOUCH)
  Keyboard::hide();
#endif
  auto screen = customScreens[customScreenIdx];
  if (screen) {
    auto viewMain = ViewMain::instance();
    viewMain->setCurrentMainView(savedView);
  }
  FormWindow::deleteLater(detach, trash);

  // restore screen setting tab on top
  menu->bringToTop();
  Layer::pop(this);
}

#if defined(HARDWARE_TOUCH)
bool SetupWidgetsPage::onTouchSlide(coord_t x, coord_t y, coord_t startX,
                                    coord_t startY, coord_t slideX,
                                    coord_t slideY)
{
  FormWindow::onTouchSlide(x, y, startX, startY, slideX, slideY);
  return true;
}

bool SetupWidgetsPage::onTouchEnd(coord_t x, coord_t y)
{
  FormWindow::onTouchEnd(x, y);
  return true;
}
#endif

#if defined(HARDWARE_KEYS)
void SetupWidgetsPage::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(),
                event);

  if (event == EVT_KEY_LONG(KEY_EXIT) || event == EVT_KEY_BREAK(KEY_EXIT)) {
    killEvents(event);
    deleteLater();
  }
}
#endif
