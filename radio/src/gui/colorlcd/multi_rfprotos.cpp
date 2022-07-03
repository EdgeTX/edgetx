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

// TODO: translation
const char* RFSCAN_TITLE = "MPM: Scanning protocols...";

RfScanDialog::RfScanDialog(Window* parent, MultiRfProtocols* protos,
                           std::function<void()> onClose) :
  Dialog(parent, RFSCAN_TITLE, rect_t{}),
  protos(protos),
  progress(new Progress(&content->form, rect_t{})),
  onClose(std::move(onClose))
{
  progress->setHeight(LV_DPI_DEF / 4);

  content->setWidth(LCD_W * 0.8);
  content->updateSize();

  auto content_w = lv_obj_get_content_width(content->form.getLvObj());
  progress->setWidth(content_w);
  
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

