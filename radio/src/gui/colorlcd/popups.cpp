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

#include "popups.h"
#include "libopenui.h"

static MessageDialog * createPopupInformation(const char * message)
{
  return new MessageDialog(MainWindow::instance(), "Message", message);
}

static MessageDialog * createPopupWarning(const char * message)
{
  return new MessageDialog(MainWindow::instance(), "Warning", message);
}

void POPUP_INFORMATION(const char * message)
{
  auto popup = createPopupInformation(message);
  while (popup->getParent()) {
    WDG_RESET();
    lv_refr_now(nullptr);
    LvglWrapper::instance()->pollInputs();
    RTOS_WAIT_MS(20);
  }
}

void POPUP_WARNING(const char * message, const char * info)
{
  auto popup = createPopupWarning(message);
  if (info) {
    popup->setInfoText(std::string(info));
  }
  while (popup->getParent()) {
    WDG_RESET();
    lv_refr_now(nullptr);
    LvglWrapper::instance()->pollInputs();
    RTOS_WAIT_MS(20);
  }
}
