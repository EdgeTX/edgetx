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

#include "io/multi_protolist.h"
#include "multi_rfprotos.h"
#include "strhelpers.h"

#include "progress.h"

// TODO: format for LCD_WIDTH > LCD_HEIGHT
constexpr rect_t RFSCAN_DIALOG_RECT = {
  50, 100, LCD_W - 100, LCD_H - 200
};

// TODO: translation
const char* RFSCAN_TITLE = "MPM: Scanning protocols...";

RfScanDialog::RfScanDialog(Window* parent, MultiRfProtocols* protos,
                           std::function<void()> onClose) :
  Dialog(parent, RFSCAN_TITLE, RFSCAN_DIALOG_RECT),
  protos(protos),
  progress(new Progress(&content->form,
                        {PAGE_PADDING, PAGE_PADDING,
                            content->form.width() - 2 * PAGE_PADDING,
                            content->form.height() - 2 * PAGE_PADDING})),
  onClose(std::move(onClose))
{
  // disable canceling dialog
  setCloseWhenClickOutside(false);
  // setFocus();
}

void RfScanDialog::showProgress()
{
  progress->setValue((int)(protos->getProgress() * 100.0));
}

  // disable keys
void RfScanDialog::onEvent(event_t) { return; }
  
void RfScanDialog::checkEvents()
{
  if (!protos->isScanning()) {
    deleteLater();
    onClose();
  } else if (RTOS_GET_MS() - lastUpdate >= 200) {
    showProgress();
    lastUpdate = RTOS_GET_MS();
  }
  
  Dialog::checkEvents();
}

