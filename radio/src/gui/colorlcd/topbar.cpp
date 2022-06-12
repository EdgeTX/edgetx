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

#include "opentx.h"
#include "layer.h"
#include "view_main.h"
#include "storage/storage.h"
#include "topbar.h"
#include "topbar_impl.h"
#include "menu_screen.h"
#include "widgets_setup.h"


Topbar * TopbarFactory::create(Window * parent)
{
  return new TopbarImpl(parent);
}

SetupTopBarWidgetsPage::SetupTopBarWidgetsPage(ScreenMenu* menu):
  FormWindow(ViewMain::instance(), rect_t{}, FORM_FORWARD_FOCUS),
  menu(menu)
{
  // remember focus
  Layer::push(this);

  auto viewMain = ViewMain::instance();

  // save current view & switch to 1st one
  viewMain->setCurrentMainView(0);
  viewMain->bringToTop();

  // adopt the dimensions of the main view
  setRect(viewMain->getRect());

  auto topbar = dynamic_cast<TopbarImpl*>(viewMain->getTopbar());
  for (unsigned i = 0; i < topbar->getZonesCount(); i++) {
    auto rect = topbar->getZone(i);
    new SetupWidgetsPageSlot(this, rect, topbar, i);
  }

#if defined(HARDWARE_TOUCH)
  new Button(
      this, {0, 0, MENU_HEADER_BUTTON_WIDTH, MENU_HEADER_BUTTON_WIDTH},
      [this]() -> uint8_t {
        this->deleteLater();
        return 1;
      },
      NO_FOCUS | FORM_NO_BORDER, 0, window_create);
#endif
}

void SetupTopBarWidgetsPage::onClicked()
{
  // block event forwarding (window is transparent)
}

void SetupTopBarWidgetsPage::onCancel()
{
  deleteLater();  
}

void SetupTopBarWidgetsPage::deleteLater(bool detach, bool trash)
{
  // restore screen setting tab on top
  menu->bringToTop();
  Layer::pop(this);

  // and continue async deletion...
  FormWindow::deleteLater(detach, trash);

  storageDirty(EE_MODEL);
}
