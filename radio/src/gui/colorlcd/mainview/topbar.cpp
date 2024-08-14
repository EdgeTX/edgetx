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

#include "topbar.h"

#include "layer.h"
#include "menu_screen.h"
#include "edgetx.h"
#include "storage/storage.h"
#include "etx_lv_theme.h"
#include "topbar_impl.h"
#include "view_main.h"
#include "widgets_setup.h"

TopBar* TopbarFactory::create(Window* parent) { return new TopBar(parent); }

SetupTopBarWidgetsPage::SetupTopBarWidgetsPage() :
    Window(ViewMain::instance(), rect_t{})
{
  // remember focus
  Layer::push(this);

  auto viewMain = ViewMain::instance();

  // save current view & switch to 1st one
  viewMain->setCurrentMainView(0);

  // adopt the dimensions of the main view
  setRect(viewMain->getRect());

  auto topbar = viewMain->getTopbar();
  for (unsigned i = 0; i < topbar->getZonesCount(); i++) {
    auto rect = topbar->getZone(i);
    new SetupWidgetsPageSlot(this, rect, topbar, i);
  }

#if defined(HARDWARE_TOUCH)
  addBackButton();
#endif
}

void SetupTopBarWidgetsPage::onClicked()
{
  // block event forwarding (window is transparent)
}

void SetupTopBarWidgetsPage::onCancel() { deleteLater(); }

void SetupTopBarWidgetsPage::deleteLater(bool detach, bool trash)
{
  Layer::pop(this);

  // and continue async deletion...
  Window::deleteLater(detach, trash);

  // restore screen setting tab on top
  new ScreenMenu(0);

  storageDirty(EE_MODEL);
}

void SetupTopBarWidgetsPage::onEvent(event_t event)
{
#if defined(HARDWARE_KEYS)
  if (event == EVT_KEY_FIRST(KEY_PAGEUP) || event == EVT_KEY_FIRST(KEY_PAGEDN) ||
      event == EVT_KEY_FIRST(KEY_SYS) || event == EVT_KEY_FIRST(KEY_MODEL)) {
    killEvents(event);
  } else if (event == EVT_KEY_FIRST(KEY_TELE)) {
    onCancel();
  } else {
    Window::onEvent(event);
  }
#else
  Window::onEvent(event);
#endif
}
