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

#include "gui.h"
#include "layer.h"
#include "menu.h"
#include "menu_screen.h"
#include "translations.h"
#include "view_main.h"
#include "widget.h"
#include "widget_settings.h"

SetupWidgetsPageSlot::SetupWidgetsPageSlot(FormWindow* parent,
                                           const rect_t& rect,
                                           WidgetsContainer* container,
                                           uint8_t slotIndex) :
    Button(parent, rect)
{
  setPressHandler([=]() -> uint8_t {
    if (container->getWidget(slotIndex)) {
      Menu* menu = new Menu(parent);
      menu->addLine(STR_SELECT_WIDGET,
                    [=]() { addNewWidget(container, slotIndex); });
      auto widget = container->getWidget(slotIndex);
      if (widget->getOptions() && widget->getOptions()->name)
        menu->addLine(STR_WIDGET_SETTINGS,
                      [=]() { new WidgetSettings(parent, widget); });
      menu->addLine(STR_REMOVE_WIDGET,
                    [=]() { container->removeWidget(slotIndex); });
    } else {
      addNewWidget(container, slotIndex);
    }

    return 0;
  });
}

void SetupWidgetsPageSlot::addNewWidget(WidgetsContainer* container,
                                        uint8_t slotIndex)
{
  Menu* menu = new Menu(parent);
  menu->setTitle(STR_SELECT_WIDGET);
  for (auto factory : getRegisteredWidgets()) {
    menu->addLine(factory->getDisplayName(), [=]() {
      container->createWidget(slotIndex, factory);
      auto widget = container->getWidget(slotIndex);
      if (widget->getOptions() && widget->getOptions()->name)
        new WidgetSettings(parent, widget);
    });
  }
}

void SetupWidgetsPageSlot::paint(BitmapBuffer* dc)
{
  dc->drawRect(0, 0, width(), height(), 2, hasFocus() ? STASHED : SOLID,
               COLOR_THEME_FOCUS);
}

SetupWidgetsPage::SetupWidgetsPage(uint8_t customScreenIdx) :
    FormWindow(ViewMain::instance(), rect_t{}), customScreenIdx(customScreenIdx)
{
  Layer::push(this);

  // attach this custom screen here so we can display it
  auto screen = customScreens[customScreenIdx];
  if (screen) {
    setRect(screen->getRect());
    auto viewMain = ViewMain::instance();
    savedView = viewMain->getCurrentMainView();
    viewMain->setCurrentMainView(customScreenIdx);
  }

  for (unsigned i = 0; i < screen->getZonesCount(); i++) {
    auto rect = screen->getZone(i);
    auto widget_container = customScreens[customScreenIdx];
    new SetupWidgetsPageSlot(this, rect, widget_container, i);
  }

#if defined(HARDWARE_TOUCH)
  new Button(
      this,
      {0, 0, MENU_HEADER_BACK_BUTTON_WIDTH, MENU_HEADER_BACK_BUTTON_HEIGHT},
      [this]() -> uint8_t {
        this->deleteLater();
        return 1;
      },
      NO_FOCUS, 0, window_create);
#endif
}

void SetupWidgetsPage::onClicked()
{
  // block event forwarding (window is transparent)
}

void SetupWidgetsPage::onCancel() { deleteLater(); }

void SetupWidgetsPage::deleteLater(bool detach, bool trash)
{
  // restore screen setting tab on top
  Layer::pop(this);

  // and continue async deletion...
  auto screen = customScreens[customScreenIdx];
  if (screen) {
    auto viewMain = ViewMain::instance();
    viewMain->setCurrentMainView(savedView);
  }
  FormWindow::deleteLater(detach, trash);
  new ScreenMenu(customScreenIdx + 1);

  storageDirty(EE_MODEL);
}

void SetupWidgetsPage::onEvent(event_t event)
{
#if defined(HARDWARE_KEYS)
  if (event == EVT_KEY_FIRST(KEY_PAGEUP) ||
      event == EVT_KEY_FIRST(KEY_PAGEDN) || event == EVT_KEY_FIRST(KEY_SYS) ||
      event == EVT_KEY_FIRST(KEY_MODEL)) {
    killEvents(event);
  } else if (event == EVT_KEY_FIRST(KEY_TELE)) {
    onCancel();
  } else {
    FormWindow::onEvent(event);
  }
#else
  FormWindow::onEvent(event);
#endif
}
