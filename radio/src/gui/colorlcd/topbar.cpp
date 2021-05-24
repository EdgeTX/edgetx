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

#include "opentx.h"
#include "layer.h"
#include "view_main.h"
#include "screen_setup.h"
#include "storage/storage.h"
#include "topbar.h"
#include "topbar_impl.h"


Topbar * TopbarFactory::create(Window * parent)
{
  return new TopbarImpl(parent);
}

SetupTopBarWidgetsPage::SetupTopBarWidgetsPage(ScreenMenu* menu):
  FormWindow(ViewMain::instance(), {0, 0, 0, 0}, FORM_FORWARD_FOCUS),
  menu(menu)
{
  // remember focus
  Layer::push(this);

  auto viewMain = ViewMain::instance();

  // save current view & switch to 1st one
  savedView = viewMain->getCurrentMainView();
  viewMain->setCurrentMainView(0);
  viewMain->bringToTop();

  //TODO: force the topbar to be visible?

  // adopt the dimensions of the main view
  setRect(viewMain->getRect());

  auto topbar = dynamic_cast<TopbarImpl*>(viewMain->getTopbar());
  for (unsigned i = 0; i < topbar->getZonesCount(); i++) {
    auto rect = topbar->getZone(i);
    auto widget = new SetupWidgetsPageSlot(this, rect, topbar, i);
    if (i == 0) widget->setFocus();
  }
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

#if defined(HARDWARE_KEYS)
void SetupTopBarWidgetsPage::onEvent(event_t event)
{
  switch (event) {
    case EVT_KEY_BREAK(KEY_EXIT):
      killEvents(event);
      deleteLater();
      break;
  }
}
#endif

#if defined(HARDWARE_TOUCH)
bool SetupTopBarWidgetsPage::onTouchSlide(coord_t x, coord_t y,
                                          coord_t startX, coord_t startY,
                                          coord_t slideX, coord_t slideY)
{
  return true;
}

bool SetupTopBarWidgetsPage::onTouchEnd(coord_t x, coord_t y)
{
  if (FormWindow::onTouchEnd(x, y))
    return true;

  return true;
}
#endif
